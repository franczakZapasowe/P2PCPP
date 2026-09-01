#pragma once
#include <thread>
#include <netinet/in.h>
#include <mutex>
#include "SafeQueue.h"
#include "TransferTask.h"
// socket, addres
extern std::atomic<bool> isRunning;
class ThreadPool {
    int m_ileWatkow;
    int m_socket;
    std::mutex m_mutex;
    SafeQueue<TransferTask> * m_transferQueue;
    std::vector<std::thread> threads;
    void worker();
    public:
    ThreadPool(int ileWatkow, int socket,SafeQueue<TransferTask> * transferQueue)
    :m_ileWatkow(ileWatkow), m_socket(socket),  m_transferQueue(transferQueue) {
        for (int i = 0; i < m_ileWatkow; i++) {
            threads.emplace_back([this](){worker();});
        }
    };

    ~ThreadPool() {
        for (auto &t : threads) {
            t.join();
        }
    }
};

void ThreadPool::worker() {
    TransferTask headerTemp{};
    while (isRunning) {
        if (m_transferQueue->pop(headerTemp)) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                send(m_socket, &headerTemp.header, sizeof(headerTemp.header),0); // wysylamy
                send(m_socket, headerTemp.data.data(), headerTemp.header.chunk_size,0); // wysylamy
            }
        }
    }
}