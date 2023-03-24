#pragma once

#include <mutex>
#include <string>
#include <memory>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpCookie.h"
#include "ThreadPool.hpp"
#include <functional>


class HttpClient {
public:
    using SSLBack = std::function<CURLcode(CURL *curl, void *ssl_ctx)>;
public:
    static HttpClient &getInstance();

    void enableCookies(const std::string &cookieFile = std::string());

    const std::string &getCookieFilename() const;

    void setSSLVerification(const std::string &caFile);

    void setSSLVerification(std::string &&caFile);

    const std::string &getSSLVerification() const;

    void setTimeoutForConnect(int value);

    int getTimeoutForConnect() const;

    void setTimeoutForRead(int value);

    int getTimeoutForRead() const;

    void enqueue(const std::shared_ptr<HttpRequest> &request);

    std::shared_ptr<HttpResponse> execute(const std::shared_ptr<HttpRequest> &request);

    std::shared_ptr<HttpCookie> getCookie() const {
        if (cookieFilename_.empty())
            return std::shared_ptr<HttpCookie>();
        return std::make_shared<HttpCookie>(cookieFilename_);
    }

    SSLBack &getSSLCallback() {
        return ssl_cb;
    }

private:
    HttpClient(int connectTimeOut = 10, int readTimeOut = 10, int maxThreadNum = 10);

    ~HttpClient();

    HttpClient(const HttpClient &) = delete;

    HttpClient &operator=(const HttpClient &) = delete;

    void processResponse(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response);

    void doGet(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response);

    void doPost(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response);

    void doPut(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response);

    void doDelete(const std::shared_ptr<HttpRequest> &request, std::shared_ptr<HttpResponse> &response);

    void setSSLCallback(const SSLBack ssl_cb) {
        this->ssl_cb = std::move(ssl_cb);
    }

    CURLcode sslCallback(CURL *curl, void *ssl_ctx);

private:
    static const int kErrorBufSize = 256;

    mutable std::mutex cookieFileMutex_;

    std::string cookieFilename_;

    mutable std::mutex sslCaFileMutex_;

    std::string sslCaFilename_;

    int timeoutForConnect_;

    int timeoutForRead_;

    ThreadPool threadPool_;

    SSLBack ssl_cb;
};

