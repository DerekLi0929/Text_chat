#ifndef SOCKET_HELPER_H
#define SOCKET_HELPER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

class SocketHelper {
public:
    static int create_socket() {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
        }
        return sockfd;
    }

    static void close_socket(int sockfd) {
        if (close(sockfd) < 0) {
            std::cerr << "Failed to close socket: " << strerror(errno) << std::endl;
        }
    }

    static sockaddr_in create_address(const std::string& ip, uint16_t port) {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        throw std::runtime_error("Failed to convert IP address: " + std::string(strerror(errno)));
    }
    return addr;
}


};

#endif  // SOCKET_HELPER_H
