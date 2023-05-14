#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>
#include"server.h"
void Server::start_server() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cout << "Error creating socket" << std::endl;
        return;
    }

    int PORT;
    std::cout << "Input port: ";
    std::cin >> PORT;
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_address,
             sizeof(server_address)) < 0) {
        std::cout << "Error binding socket"<<std::endl;            
        close(server_socket);
            return;
    }

    if (listen(server_socket, 5) < 0) {
        std::cout << "Error listening for connections" << std::endl;
        close(server_socket);
        return;
    }

    std::cout << "Server started. Listening for connections..." << std::endl;

    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);
        int client_socket =
            accept(server_socket, (struct sockaddr*)&client_address,
                   &client_address_size);
        if (client_socket < 0) {
            std::cout << "Error accepting connection" << std::endl;
            continue;
        }

        std::thread t(&Server::handle_new_connection, this, client_socket,
                      client_address);
        t.detach();
    }

    close(server_socket);
}
void Server::handle_new_connection(int Client)/// тут я туплю жестко


