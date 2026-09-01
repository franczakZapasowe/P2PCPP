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
        cv.notify_one();
    }
    bool pop(T &item) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock,[this]{return !queue.empty() || flag ;});
        if (queue.empty()) {
            return false;
        }
        item = queue.front(); // zgarniamy elemnt zanim go usuneimy
        queue.pop();
        return true;
    }

    bool isEmpty() { // kazda metoda ktora korzysta z kolejki musi być chroniona
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

    void wakeup() {
        std::lock_guard<std::mutex> lock(mutex);
        flag = true;
        cv.notify_all();
    }
};