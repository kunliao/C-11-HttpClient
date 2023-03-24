#pragma once

#include "HttpRequest.h"
#include <unordered_map>
#include <iostream>


class HttpClient;

class HttpResponse {
    friend class HttpClient;
public:
    HttpResponse() :
            succeed_(false),
            responseCode_(-1) {
    }

    ~HttpResponse() = default;

    bool isSucceed() const {
        return succeed_;
    }

    std::vector<char> *getResponseData() {
        return &responseData_;
    }

    const std::unordered_multimap<std::string, std::string> &getResponseHeader() const {
        return responseHeader_;
    }

    long getResponseCode() const {
        return responseCode_;
    }


    const std::string responseDataAsString() const {
        if (responseData_.empty()) return "receive data is empty!";
        return std::string(&*responseData_.begin(), responseData_.size());
    }

    const std::string &gerErrorBuffer() const {
        return errorBuffer_;
    }

    const std::string &getProtocol() const {
        return protocol_;
    }

    void readHeader(void *data,size_t length);

    void readBody(void *data,size_t length);

private:
    bool succeed_;
    std::vector<char> responseData_;
    std::string protocol_;
    std::unordered_multimap<std::string, std::string> responseHeader_;
    int responseCode_;
    std::string errorBuffer_;
};
