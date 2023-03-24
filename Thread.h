#pragma once

#include <functional>
#include <thread>

class Thread {
public:
    using ThreadFunc = std::function<void(int)>;

    Thread(ThreadFunc func)
            : func_(func), threadId_(generateId_++) {}


    ~Thread() = default;

    void start() {
        std::thread t(func_, threadId_);
        t.detach();
    }

    int getId() const {
        return threadId_;
    }

private:
    ThreadFunc func_;
    static int generateId_;
    int threadId_;
};

