#include "HttpResponse.h"

void HttpResponse::readHeader(void *data, size_t length) {

    std::string responseline(static_cast<char *>(data));
    if (responseline.find("HTTP") == 0)
        this->protocol_ = std::move(responseline);
    else {
        std::string::size_type index = responseline.find(":");
        if (index != std::string::npos) {
            std::string key = responseline.substr(0, index);
            std::string value = responseline.substr(index + 1);
            this->responseHeader_.insert({key, value});
        }
    }
}

void HttpResponse::readBody(void *data, size_t length) {
    std::vector<char> *recvBuf = this->getResponseData();

    recvBuf->insert(recvBuf->end(), (char *) data, (char *) data + length);
}