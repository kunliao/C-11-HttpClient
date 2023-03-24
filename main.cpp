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
    HttpClient::getInstance().enableCookies("./cookie.txt");
    //HttpClient::getInstance().enqueue(request);

    std::shared_ptr<HttpResponse> r=  HttpClient::getInstance().execute(request);

    std::shared_ptr<HttpCookie> cookie= HttpClient::getInstance().getCookie();
    cout<<"";
}

//GET HTTP
void testCase2(bool isImmediate = false) {
    RequestPtr request = make_shared<HttpRequest>();
    request->setRequestType(HttpRequest::Type::GET);
    request->setUrl("http://httpbin.org/ip");
    request->setCallback(onMessage);

    request->setTag("test case 2: GET");
    HttpClient::getInstance().enqueue(request);

}

//GET HTTPS
void testCase3(bool isImmediate = false) {
    RequestPtr request = make_shared<HttpRequest>();
    request->setRequestType(HttpRequest::Type::GET);
    request->setUrl("https://httpbin.org/get");
    request->setCallback(onMessage);

    request->setTag("test case 3: GET");
    HttpClient::getInstance().enqueue(request);

}

//POST
void testCase4(bool isImmediate = false) {
    RequestPtr request = make_shared<HttpRequest>();
    request->setRequestType(HttpRequest::Type::POST);
    request->setUrl("http://httpbin.org/post");
    request->setCallback(onMessage);
    request->setRequestData("visitor=zhangkuo&time=20171103");

    request->setTag("test case 4: POST");
    HttpClient::getInstance().enqueue(request);

}


//POST binary
void testCase6(bool isImmediate = false) {
    auto request = make_shared<HttpRequest>();
    request->setRequestType(HttpRequest::Type::POST);
    request->setUrl("http://httpbin.org/post");
    request->setCallback(onMessage);
    const char *p = "binary data: I love cpp!\0\0from zhangkuo";
    request->setRequestData(p, 39);

    request->setTag("test case 6: POST binary");
    HttpClient::getInstance().enqueue(request);

}

//PUT
//void testCase7(bool isImmediate = false) {
//    auto request = make_shared<HttpRequest>();
//    request->setRequestType(HttpRequest::Type::PUT);
//    request->setUrl("http://httpbin.org/put");
//    request->setCallback(onMessage);
//    request->setRequestData("visitor=zhangkuo&time=20171103");
//    request->setUploadFilePath("./curl/lib/libcurl.lib");
//    if (isImmediate) {
//        request->setTag("test case 7: PUT immediate");
//        HttpClient::getInstance()->sendImmediate(request);
//    } else {
//        request->setTag("test case 7: PUT");
//        HttpClient::getInstance()->send(request);
//    }
//}

//DELETE
void testCase8(bool isImmediate = false) {
    auto request = make_shared<HttpRequest>();
    request->setRequestType(HttpRequest::Type::DELETE);
    request->setUrl("http://httpbin.org/delete");
    request->setCallback(onMessage);

    request->setTag("test case 8: DELETE immediate");
    HttpClient::getInstance().enqueue(request);

}

void testAllCase() {
    testCase1();
//    testCase1(true);
//    testCase2();
//    testCase2(true);
//    testCase3();
//    testCase3(true);
//    testCase4();
//    testCase4(true);

//    testCase6();
//    testCase6(true);
//    testCase7();
//    testCase7(true);
//    testCase8();
//    testCase8(true);
}

int main() {
    testAllCase();
    getchar();
}