#pragma once
#include <thread>
#include <array>
// socket, addres
void Worker() {

}


class ThreadPool {
    int ileWatkw;
    std::vector<std::thread> threads;
    public:
    ThreadPool(int ile ):ileWatkw(ile) {
        for (int i = 0; i < ileWatkw; i++) {
            threads.emplace_back()
        }
    };
};