#pragma warning(disable:4996)

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include "HttpClient.h"
#include <curl/curl.h>
#include <time.h>
#include <chrono>
#include <atomic>

#include <codecvt>
#include <locale>

using namespace std;


using RequestPtr = shared_ptr<HttpRequest>;
using ResponsePtr = shared_ptr<HttpResponse>;

std::mutex mutex_;

void onMessage(const RequestPtr &request, const ResponsePtr &response) {
    std::lock_guard<std::mutex> lock(mutex_);

    cout << "\n" << request->getTag() << endl;
    for (auto it = response->getResponseHeader().begin(); it != response->getResponseHeader().end(); it++) {
        cout << it->first << ":" << it->second << endl;
    }

    if (response->isSucceed()) {
        cout << "HTTP request succeed!" << endl;

        auto utf8Str = response->responseDataAsString();


        cout << utf8Str << endl;
    } else {
        cout << "HTTP request failed!" << endl;
        cout << "status code: " << response->getResponseCode() << endl;
        cout << "reason: " << response->gerErrorBuffer() << endl;
    }

}

//GET HTTP failed
void testCase1(bool isImmediate = false) {
    RequestPtr request = make_shared<HttpRequest>();
    request->setRequestType(HttpRequest::Type::GET);
//    request->setUrl("http://175.178.71.203:8000");
    request->setUrl("https://www.baidu.com");
    request->setCallback(onMessage);

    request->setTag("test case 1: GET");
    //HttpClient::getInstance().enableCookies("cookie.txt");
    HttpClient::getInstance().enqueue(request);
}



int httptest() {
    curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
    printf("openssl ssl_version %s \n",data->ssl_version);
    testCase1();
    return 0;
}
