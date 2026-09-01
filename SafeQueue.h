#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
template <typename T>
class SafeQueue {
    std::mutex mutex;
    bool flag {false};
    std::queue<T> queue;
    std::condition_variable cv;
    public:
    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push(item); // item w naszym wypadku to bedzie FileTransferHeader
        std::cout<<"Dodalem element do kolejki\n";
        flag = true;
        cv.notify_one();
    }
    void pop(T &item) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock,[this]{return flag;});
        item = queue.front(); // zgarniamy elemnt zanim go usuneimy
        queue.pop();
        flag = false;
    }
};