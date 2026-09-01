cmake_minimum_required(VERSION 4.2)
project(P2P)

set(CMAKE_CXX_STANDARD 20)

add_executable(P2P
        serwer.cpp
        serwer.cpp
        klient.cpp
        FileTransferHeader.h
        ChunkHeader.h
        TransferTask.h
        SafeQueue.h
        ThreadPool.h)
