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
#include <jni.h>
#include <android/log.h>

using namespace std;
#define TAG "http_client"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__);
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__);
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__);
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__);

using RequestPtr = shared_ptr<HttpRequest>;
using ResponsePtr = shared_ptr<HttpResponse>;

std::mutex mutex_;

void onMessage(const RequestPtr &request, const ResponsePtr &response) {
    std::lock_guard<std::mutex> lock(mutex_);
    LOGI("response code %d", response->getResponseCode());
    LOGI("response tag %s", request->getTag().c_str());
    cout << "\n" << request->getTag() << endl;
    for (auto it = response->getResponseHeader().begin();
         it != response->getResponseHeader().end(); it++) {
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

    request->setUrl("https://www.baidu.com");
    request->setCallback(onMessage);
    request->setTag("test case 1: GET");

    HttpClient::getInstance().enqueue(request);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_curl_MainActivity_request(JNIEnv *env, jobject thiz) {


    LOGI("MainActivity_request");
    testCase1();
}