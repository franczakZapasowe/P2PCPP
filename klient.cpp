#include <cstring>
#include <iostream>
#include <ostream>

#include "FileTransferHeader.h"
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
    FileTransferHeader file;
    file.id = 0;
    file.size = 200;
    strncpy(file.name,"test dzialania",25);
    send(clientSocket, &file, sizeof(file), 0);
    char buffer[1024];
    int lastChar = recv(clientSocket, buffer, 1024, 0);
    buffer[lastChar] = '\0';
    std::cout<<buffer<<std::endl;
    CLOSESOCKET(clientSocket);
    return 0;
}