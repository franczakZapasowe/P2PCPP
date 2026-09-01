#include <cstring>
#include <iostream>
#include <ostream>
#include <fstream>
#include "FileTransferHeader.h"
#include <vector>
#include "ThreadPool.h"
#include "FileTransferHeader.h"
#include "ChunkHeader.h"
#include "TransferTask.h"
#include "SafeQueue.h"
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // Automatycznie linkuje bibliotekę w MSVC
    #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
    #define CLOSESOCKET(s) closesocket(s)
    #define GETSOCKETERRNO() (WSAGetLastError())
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>

    #define SOCKET int
    #define ISVALIDSOCKET(s) ((s) >= 0)
    #define CLOSESOCKET(s) close(s)
    #define GETSOCKETERRNO() (errno)
#endif

inline std::atomic<bool> isRunning{true};

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    // Żądamy wersji 2.2 biblioteki Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (!ISVALIDSOCKET(clientSocket)) {
        std::cerr<<"socket() failed"<<std::endl;
        return 1;
    }
    constexpr int PORT = 4000;
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr<<"connect() failed"<<std::endl;
        return 1;
    }
    std::cout<<"Polaczenie udane\n";

    FILE * plik = fopen("Zamieniony.wav","rb");
    if (plik==nullptr) {
        std::cerr<<"error zamieniony.wav"<<std::endl;
        return 1;
    }
    FileTransferHeader fileHeader{};
    fileHeader.id = 0;
    fseek(plik,0,SEEK_END); // ustawiamy glowice na koniec pliku
    fileHeader.size = ftell(plik); // ROZMIAR PLIKU W BYTE
    strncpy(fileHeader.name,"Zamienony.wav" ,25); //  - docelowa damy  zczytanie nazwy z biblioteki filesystem
    send(clientSocket, &fileHeader, sizeof(fileHeader), 0); // w tymm momencie serwer wie jaki będzie rozmiar pliku id i nazwa
    fseek(plik,0,SEEK_SET);

    size_t ileByte = 0;
    std::vector<char>bufor(65536); // bufor na nasze dane 64 KB
    ChunkHeader chunk{}; // CHUNK
    chunk.file_id = fileHeader.id;

    int tempOffest = ftell(plik); // 0;
    TransferTask transfer_task; // sttuktura dla wątkow - header + dane
    SafeQueue<TransferTask> transferQueue;
    ThreadPool threadPool(4, clientSocket, &transferQueue);

    while ( (ileByte = fread(bufor.data(),1,65536,plik))> 0) {
        // teraz pracuje nasz producent on nie ma dostepu do send tylko tworzy nowe struktury i dodaje je do kolejki
        chunk.offset = tempOffest;
        chunk.chunk_size = ileByte;
        tempOffest = ftell(plik); // bedziemy zczytawac sobie do przodu w nastepnej iteracji dopierio bedzie wpisane to co jest tu zczytane
        transfer_task.header = chunk;
        transfer_task.data = bufor;
        transferQueue.push(transfer_task);
        // send(clientSocket, &chunk, sizeof(chunk), 0); // wysylamy etykiete
        // send(clientSocket,bufor.data(),ileByte, 0); // wysylamy dane
    }
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } while ( !transferQueue.isEmpty());

    isRunning.store(false);

    transferQueue.wakeup();

    fclose(plik);

    char buffer[1024];
    int lastChar = recv(clientSocket, buffer, 1024, 0);
    buffer[lastChar] = '\0';
    std::cout<<buffer<<std::endl;
    CLOSESOCKET(clientSocket);
#ifdef _WIN32
    WSACleanup(); // Zwolnienie biblioteki przed zamknięciem programu
#endif
    return 0;
}