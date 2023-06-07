#include "socket_helper.h"

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

std::atomic<bool> running{true};

void receive_messages(int sockfd) {
    while (running) {
        char buffer[1024];
        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) {
            running = false;
            break;
        }

        buffer[bytes_received] = '\0';
        std::cout << buffer;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: chat_client_no_asio <ip> <port>\n";
        return 1;
    }

    std::string ip = argv[1];
    uint16_t port = std::stoi(argv[2]);

    int sockfd = SocketHelper::create_socket();
    sockaddr_in server_address = SocketHelper::create_address(ip, port);

    if (connect(sockfd, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Failed to connect to server: " << strerror(errno) << std::endl;
        SocketHelper::close_socket(sockfd);
        return 1;
    }

    std::thread receive_thread(receive_messages, sockfd);
    std::string line;
    while (std::getline(std::cin, line) && running) {
        line += '\n';
        send(sockfd, line.data(), line.size(), 0);
    }

    running = false;
    receive_thread.join();
    SocketHelper::close_socket(sockfd);
    return 0;
}
