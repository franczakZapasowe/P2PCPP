#include <cstring>
#include <iostream>
#include <ostream>
#include <vector>

#include "FileTransferHeader.h"
#include "ChunkHeader.h"
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

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    // Żądamy wersji 2.2 biblioteki Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif

    SOCKET serwerSocket = socket(AF_INET,SOCK_STREAM,0);
    if (!ISVALIDSOCKET(serwerSocket)) {
        std::cerr<<"socket() failed"<<std::endl;
        return 1;
    }

    constexpr int PORT = 4000;
    sockaddr_in serwerAddr{};
    serwerAddr.sin_family = AF_INET;
    serwerAddr.sin_port = htons(PORT);
    serwerAddr.sin_addr.s_addr = INADDR_ANY;

    auto serwerAddress = reinterpret_cast<sockaddr*>(&serwerAddr);
    if (bind(serwerSocket,serwerAddress,sizeof(serwerAddr))==-1) {
        std::cerr<<"bind() failed"<<std::endl;
        return 1;
    }

    if (listen(serwerSocket,4)==-1) {
        std::cerr<<"listen() failed"<<std::endl;
        return 1;
    }

    sockaddr_in clientAddr{};
    socklen_t clientAddrLen = sizeof(clientAddr);
    int klientSocket = 0;
    if (((klientSocket = accept(serwerSocket,reinterpret_cast<sockaddr*>(&clientAddr),&clientAddrLen))==-1)){
        std::cerr<<"accept() failed"<<std::endl;
        return 1;
    }
    std::cout<<"Polaczenie udane\n";

    FileTransferHeader file;
    int ifRecv = recv(klientSocket,&file,sizeof(file),0);
    if (ifRecv <= 0) {
        std::cerr<<"Can t recev a File Transfer Header\n";
        return 1;
    }
    std::cout<<"Otzymałem FileHeader\n";
    std::cout<<"Id: "<<file.id<<" name: "<<file.name<<" size: "<<file.size<<std::endl;

    FILE * plik = fopen("Nowy.wav","wb");
    if (plik==nullptr) {
        std::cerr<<"can not create new file "<<std::endl;
        return 1;
    }

    ChunkHeader chunk;
    size_t sumaRozmiaruWszystkichChankow = 0;
    std::vector<char> buffor(65536);
    int ileDoBufora = 0;
    int licznikChankow = 0;
    while (file.size > sumaRozmiaruWszystkichChankow) {
        licznikChankow = 0;
        int czyRecev = recv(klientSocket,&chunk,sizeof(chunk),0); // odbieramy chunk
        if (czyRecev <= 0) {
            std::cerr<<"Can t recev a File Transfer Header\n";
            return 1;
        }
        fseek(plik,chunk.offset,SEEK_SET); // przsuwamy glowice na offset
        while (chunk.chunk_size > licznikChankow ) {
            ileDoBufora = recv(klientSocket,buffor.data() + licznikChankow,chunk.chunk_size - licznikChankow,0); // odbieramy pakiet z danymi
            licznikChankow+=ileDoBufora;
            sumaRozmiaruWszystkichChankow += ileDoBufora;
        }
        fwrite(buffor.data(),1,licznikChankow,plik); // wpisujemy go do pliku
    }

    fclose(plik);
    CLOSESOCKET(serwerSocket);
#ifdef _WIN32
    WSACleanup(); // Zwolnienie biblioteki przed zamknięciem programu
#endif
    return 0;
}