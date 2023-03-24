```
void onMessage(const RequestPtr &request, const ResponsePtr &response) {
}


std::shared_ptr<HttpResponse> request = make_shared<HttpRequest>();
request->setRequestType(HttpRequest::Type::GET);
request->setUrl("https://www.baidu.com");
request->setCallback(onMessage);

request->setTag("test case 1: GET");
//HttpClient::getInstance().enableCookies("./cookie.txt");//enable cookies

//asynchronous request
HttpClient::getInstance().enqueue(request);


//synchronous request
//std::shared_ptr<HttpResponse> response =  HttpClient::getInstance().execute(request);



//std::shared_ptr<HttpCookie> cookie= HttpClient::getInstance().getCookie();
```