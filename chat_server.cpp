#include "socket_helper.h"

#include <atomic>
#include <cstring>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

std::mutex clients_mutex;
std::set<int> clients;

void handle_client(int client_socket) {
    try {
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.insert(client_socket);
        }

        while (true) {
            char buffer[1024];
            ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                break;
            }
            buffer[bytes_received] = '\0';

            std::lock_guard<std::mutex> lock(clients_mutex);
            for (int other_client : clients) {
                if (other_client != client_socket) {
                    send(other_client, buffer, bytes_received, 0);
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(client_socket);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }

    SocketHelper::close_socket(client_socket);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: chat_server_no_asio <port>\n";
        return 1;
    }

    uint16_t port = std::stoi(argv[1]);
    int server_socket = SocketHelper::create_socket();

    sockaddr_in server_address = SocketHelper::create_address("0.0.0.0", port);
    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        SocketHelper::close_socket(server_socket);
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
        SocketHelper::close_socket(server_socket);
        return 1;
    }

    std::cout << "Chat server listening on port " << port << std::endl;

    while (true) {
        sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);
        int client_socket = accept(server_socket, (sockaddr*)&client_address, &client_address_length);

        if (client_socket < 0) {
            std::cerr << "Failed to accept client: " << strerror(errno) << std::endl;
            continue;
        }

        std::thread(handle_client, client_socket).detach();
    }

    SocketHelper::close_socket(server_socket);
    return 0;
}

