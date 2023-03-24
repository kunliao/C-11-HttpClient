#include "HttpClient.h"
#include <memory>
#include <curl/curl.h>
#include <iostream>
#include <map>


using namespace std::placeholders;

static size_t __writeCallback(void *data, size_t size, size_t nmemb, void *userp) {
    HttpResponse *response = static_cast<HttpResponse *>(userp);

    response->readBody(data, size * nmemb);

    return size * nmemb;
}


static size_t __headerCallback(void *data, size_t size, size_t nmemb, void *userp) {
    HttpResponse *response = static_cast<HttpResponse *>(userp);
    response->readHeader(data, size * nmemb);
    return size * nmemb;
}


static CURLcode __ssl_callback(CURL *curl, void *ssl_ctx, void *clientp) {
    HttpClient *client = static_cast<HttpClient *>(clientp);
    HttpClient::SSLBack &ssl_cb = client->getSSLCallback();
    if (ssl_cb)
        return ssl_cb(curl, ssl_ctx);
    return CURLE_OK;
}

static size_t __readCallback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t nRead = fread(data, size, nmemb, (FILE *) userp);
    return nRead;
}

class Curl {
public:
    Curl() :
            curl_(curl_easy_init()),
            headers_(nullptr) {
    }

    ~Curl() {
        if (curl_) curl_easy_cleanup(curl_);

        if (headers_) curl_slist_free_all(headers_);
    }

    bool init(const HttpClient &client,
              const std::shared_ptr<HttpRequest> &request,
              const std::shared_ptr<HttpResponse> &response,
              char *errorBuf) {
        if (curl_ == nullptr) return false;

        if (!setOption(CURLOPT_NOSIGNAL, 1L)) return false;


        if (!setOption(CURLOPT_ACCEPT_ENCODING, "")) return false;


        std::string cookieFilename = client.getCookieFilename();
        if (!cookieFilename.empty()) {
            if (!setOption(CURLOPT_COOKIEFILE, cookieFilename.data())) {
                return false;
            }
            if (!setOption(CURLOPT_COOKIEJAR, cookieFilename.data())) {
                return false;
            }
        }


        if (!setOption(CURLOPT_TIMEOUT, client.getTimeoutForRead())) {
            return false;
        }

        if (!setOption(CURLOPT_CONNECTTIMEOUT, client.getTimeoutForConnect())) {
            return false;
        }

        std::string sslCaFilename = client.getSSLVerification();
        if (!sslCaFilename.empty()) {
            if (!setOption(CURLOPT_CAINFO, sslCaFilename.c_str()))
                return false;
            if (!setOption(CURLOPT_SSL_VERIFYPEER, 1)) return false;
            if (!setOption(CURLOPT_SSL_VERIFYHOST, 2)) return false;
        } else {
            if (!setOption(CURLOPT_SSL_VERIFYPEER, 0)) return false;
            if (!setOption(CURLOPT_SSL_VERIFYHOST, 0)) return false;
        }

        std::unordered_multimap<std::string, std::string> headers = request->getHeaders();
        if (!headers.empty()) {
            for (auto &header : headers) {
                char buf[512] = {0};
                snprintf(buf, sizeof(buf), "%s:%s", header.first.c_str(), header.second.c_str());
                headers_ = curl_slist_append(headers_, buf);
            }

            if (!setOption(CURLOPT_HTTPHEADER, headers_)) {
                return false;
            }
        }

        return setOption(CURLOPT_URL, request->getUrl().c_str()) &&
               setOption(CURLOPT_WRITEFUNCTION, __writeCallback) &&
               setOption(CURLOPT_WRITEDATA, response.get()) &&
               setOption(CURLOPT_HEADERFUNCTION, __headerCallback) &&
               setOption(CURLOPT_HEADERDATA, response.get()) &&
               setOption(CURLOPT_ERRORBUFFER, errorBuf) &&
               setOption(CURLOPT_SSL_CTX_FUNCTION, __ssl_callback) &&
               setOption(CURLOPT_SSL_CTX_DATA, &client);
    }

    template<typename T>
    bool setOption(CURLoption option, T data) {
        return CURLE_OK == curl_easy_setopt(curl_, option, data);
    }

    bool perform(long *responseCode) {
        if (CURLE_OK != curl_easy_perform(curl_)) return false;

        CURLcode code = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, responseCode);
        if (code != CURLE_OK || !(*responseCode >= 200 && *responseCode < 300)) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(code));
            return false;
        }

        return true;
    }

private:
    CURL *curl_;
    curl_slist *headers_;
};


HttpClient &HttpClient::getInstance() {
    static HttpClient client;
    return client;
}

HttpClient::HttpClient(int connectTimeOut, int readTimeOut, int maxThreadNum) :
        timeoutForConnect_(connectTimeOut),
        timeoutForRead_(readTimeOut) {
    threadPool_.start();
    curl_global_init(CURL_GLOBAL_WIN32);
    this->setSSLCallback(std::bind(&HttpClient::sslCallback, this, _1, _2));
}

HttpClient::~HttpClient() {
    curl_global_cleanup();
}

void HttpClient::enableCookies(const std::string &cookieFile) {
    std::lock_guard<std::mutex> lock(cookieFileMutex_);
    if (!cookieFile.empty()) {
        cookieFilename_ = cookieFile;
    }
}

const std::string &HttpClient::getCookieFilename() const {
    std::lock_guard<std::mutex> lock(cookieFileMutex_);
    return cookieFilename_;
}

void HttpClient::setSSLVerification(const std::string &caFile) {
    std::lock_guard<std::mutex> lock(sslCaFileMutex_);
    sslCaFilename_ = caFile;
}

void HttpClient::setSSLVerification(std::string &&caFile) {
    std::lock_guard<std::mutex> lock(sslCaFileMutex_);
    sslCaFilename_ = std::move(caFile);
}

const std::string &HttpClient::getSSLVerification() const {
    std::lock_guard<std::mutex> lock(sslCaFileMutex_);
    return sslCaFilename_;
}

void HttpClient::setTimeoutForConnect(int value) {
    timeoutForConnect_ = value;
}

int HttpClient::getTimeoutForConnect() const {
    return timeoutForConnect_;
}

void HttpClient::setTimeoutForRead(int value) {
    timeoutForRead_ = value;
}

int HttpClient::getTimeoutForRead() const {
    return timeoutForRead_;
}

void HttpClient::enqueue(const std::shared_ptr<HttpRequest> &request) {
    std::shared_ptr<HttpResponse> response = std::make_shared<HttpResponse>();
    threadPool_.submitTask(std::bind(&HttpClient::processResponse, this, request, response));
}

std::shared_ptr<HttpResponse> HttpClient::execute(const std::shared_ptr<HttpRequest> &request) {
    std::shared_ptr<HttpResponse> response = std::make_shared<HttpResponse>();
    this->processResponse(request, response);
    return response;
}

void HttpClient::processResponse(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response) {
    switch (request->getRequestType()) {
        case HttpRequest::Type::GET:
            doGet(request, response);
            break;
        case HttpRequest::Type::POST:
            doPost(request, response);
            break;
        case HttpRequest::Type::PUT:
            doPut(request, response);
            break;
        case HttpRequest::Type::DELETE:
            doPut(request, response);
            break;
        default:
            break;
    }

    auto callback = request->getResponseCallback();
    if (callback)
        callback(request, response);
}

void HttpClient::doGet(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response) {
    char errorBuf[kErrorBufSize] = {0};

    auto responseHeader = response->getResponseHeader();
    long responseCode = -1;

    Curl curl;
    bool ok = curl.init(*this, request, response, errorBuf) &&
              curl.setOption(CURLOPT_FOLLOWLOCATION, true) &&
              curl.perform(&responseCode);

    response->responseCode_ = responseCode;

    if (ok) {
        response->succeed_ = true;
    } else {
        response->succeed_ = false;
        response->errorBuffer_ = errorBuf;
    }
}

void HttpClient::doPost(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response) {
    char errorBuf[kErrorBufSize] = {0};

    auto responseHeader = response->getResponseHeader();
    auto postData = request->getRequestData();
    auto postDataSize = request->getRequestDataSize();
    long responseCode = -1;

    Curl curl;
    bool ok = curl.init(*this, request, response, errorBuf) &&
              curl.setOption(CURLOPT_POST, 1) &&
              curl.setOption(CURLOPT_POSTFIELDS, postData) &&
              curl.setOption(CURLOPT_POSTFIELDSIZE, postDataSize) &&
              curl.perform(&responseCode);

    response->responseCode_ = responseCode;
    if (ok) {
        response->succeed_ = true;
    } else {
        response->succeed_ = false;
        response->errorBuffer_ = errorBuf;
    }
}

void HttpClient::doPut(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response) {

    auto responseHeader = response->getResponseHeader();
    char errorBuf[kErrorBufSize] = {0};
    auto requestData = request->getRequestData();
    auto requestDataSize = request->getRequestDataSize();
    long responseCode = -1;

    FILE *fp = nullptr;
    auto path = request->getUploadFilePath();
    size_t size = 0;
    if (!path.empty()) {
        fp = fopen(path.data(), "rb");
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
    }

    Curl curl;
    curl.init(*this, request, response, errorBuf);

    curl.setOption(CURLOPT_CUSTOMREQUEST, "PUT");
    curl.setOption(CURLOPT_VERBOSE, true);
    curl.setOption(CURLOPT_POSTFIELDS, requestData);
    curl.setOption(CURLOPT_POSTFIELDSIZE, requestDataSize);

    if (fp) {
        curl.setOption(CURLOPT_UPLOAD, 1L);
        curl.setOption(CURLOPT_READFUNCTION, __readCallback);
        curl.setOption(CURLOPT_READDATA, fp);
        curl.setOption(CURLOPT_INFILESIZE_LARGE, (curl_off_t) size);
    }

    bool ok = curl.perform(&responseCode);

    response->responseCode_ = responseCode;
    if (ok) {
        response->succeed_ = true;
    } else {
        response->succeed_ = false;
        response->errorBuffer_ = errorBuf;
    }
    if (fp) fclose(fp);
}

void HttpClient::doDelete(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response) {
    char errorBuf[kErrorBufSize] = {0};
    auto responseHeader = response->getResponseHeader();
    long responseCode = -1;

    Curl curl;
    bool ok = curl.init(*this, request, response, errorBuf) &&
              curl.setOption(CURLOPT_CUSTOMREQUEST, "DELETE") &&
              curl.setOption(CURLOPT_FOLLOWLOCATION, true) &&
              curl.perform(&responseCode);

    response->responseCode_ = responseCode;
    if (ok) {
        response->succeed_ = true;
    } else {
        response->succeed_ = false;
        response->errorBuffer_ = errorBuf;
    }
}


CURLcode HttpClient::sslCallback(CURL *curl, void *ssl_ctx) {
    return CURLE_OK;
}