#include "client.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

Client::Client(int client_socket, const std::string& name,
               const struct sockaddr_in& client_address)
    : socket(client_socket), user_name(name), address(client_address) {}

int Client::getSocket() const { return socket; }

const std::string& Client::getUserName() const { return user_name; }

const struct sockaddr_in& Client::getAddress() const { return address; }

bool Client::is_Loginned_user(const std::string& name) const {
    std::ifstream file("userpwd.txt");
    std::string nameFromFile;

    while (file >> nameFromFile) {
        if (nameFromFile == name) {
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

bool Client::check_Pwd(int attempt, const std::string& name,
                       const std::string& password) {
    if (attempt < 2) {
        std::string line = name + " : " + password;
        std::ifstream file("userpwd.txt");
        std::string checkForm;

        while (std::getline(file, checkForm)) {
            if (line == checkForm) {
                std::string msg = "logging done";
                send(socket, msg.c_str(), msg.length(), 0);
                file.close();
                return true;
            }
        }

        std::string msg = "incorrect password try again: ";
        send(socket, msg.c_str(), msg.length(), 0);

        std::string againPassword;
        char buffer[1024] = {0};
        int bytes_received = recv(socket, buffer, sizeof(buffer), 0);
        againPassword = std::string(buffer, bytes_received);

        return check_Pwd(attempt + 1, name, againPassword);
    } else {
        std::string msg =
            "your logging has been detached due to excessive attempts";
        send(socket, msg.c_str(), msg.length(), 0);
        std::cout << socket
                  << " logging has been detached due to excessive attempts"
                  << std::endl;
        close(socket);
        return false;
    }
}

std::string Client::addNewUser() {
    std::ofstream file("userpwd.txt", std::ios::app);
    std::string name;
    std::string msg = "input your nick: ";
    send(socket, msg.c_str(), msg.length(), 0);
    char buffer[1024] = {0};
    int bytes_received = recv(socket, buffer, sizeof(buffer), 0);
    name = std::string(buffer, bytes_received);
    if (!is_Loginned_user(name)) {
        std::string msg = "input your password: ";
        send(socket, msg.c_str(), msg.length(), 0);
        std::string password;
        buffer[1024] = {0};
        bytes_received = recv(socket, buffer, sizeof(buffer), 0);
        password = std::string(buffer, bytes_received);

        file << '\n' + name + " : " + password;
        file.close();

        std::string successMsg = "you have been registered";
        send(socket, successMsg.c_str(), successMsg.length(), 0);

        return name;
    } else {
        std::string errorMsg = "user already exists";
        send(socket, errorMsg.c_str(), errorMsg.length(), 0);
        close(socket);
        throw std::runtime_error("registration failed");
    }
}

std::string Client::LogIn() {
    std::string msg = "input your login: ";
    send(socket, msg.c_str(), msg.length(), 0);

    char buffer[1024] = {0};
    int bytes_received = recv(socket, buffer, sizeof(buffer), 0);
    std::string user = std::string(buffer, bytes_received);
    std::cout << "logging" << std::endl;
    buffer[1024] = {0};

    if (!is_Loginned_user(user)) {
        std::string msg = "you are not logged in";
        send(socket, msg.c_str(), msg.length(), 0);

        std::string askMsg = "do you want to sign up? [y/n]";
        send(socket, askMsg.c_str(), askMsg.length(), 0);

        bytes_received = recv(socket, buffer, sizeof(buffer), 0);
        std::string ans = std::string(buffer, bytes_received);

        if (ans == "y") {
            return addNewUser();
        } else {
            close(socket);
        }
    } else {
        std::string msg = "input Password: ";
        send(socket, msg.c_str(), msg.length(), 0);

        bytes_received = recv(socket, buffer, sizeof(buffer), 0);
        std::string password = std::string(buffer, bytes_received);
        std::cout << "pwd in login" << std::endl;
        int attempt = 0;

        if (check_Pwd(attempt, user, password)) {
            return user;
        }
    }

    std::cout << "login failed!" << std::endl;
    throw std::runtime_error("login ended unexpectedly");
}
