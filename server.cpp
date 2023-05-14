#include <arpa/inet.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

// Структура для хранения информации о клиенте
struct ClientInfo {
    int socket;
    string user_name;
    struct sockaddr_in address;
};

bool is_Loginned_user(string name) {
    ifstream file("userpwd.txt");
    string nameFromFile;
    // int line = -1;
    while (!file.eof()) {
        file >> nameFromFile;
        //  ++line;
        if (nameFromFile == name) {
            file.close();
            return true;
            break;
        }
    }
    file.close();
    return false;
}
bool check_Pwd(int client_socket, string name, string password, int attempt) {
    if (attempt < 2) {
        string line = name + " : " + password;
        ifstream file("userpwd.txt");
        string checkForm;
        while (!file.eof()) {
            getline(file, checkForm);
            if (line == checkForm) {
                string msg = "logging done";
                send(client_socket, msg.c_str(), msg.length(), 0);
                return true;
            }
        }

        string msg = "incorrect password try again: ";
        send(client_socket, msg.c_str(), msg.length(), 0);

        string againPassword;
        char buffer[1024] = {0};
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        againPassword = string(buffer, bytes_received);
        check_Pwd(client_socket, name, againPassword, ++attempt);

    } else {
        string msg =
            "your logginig has been detatched by reason of excess attempts";
        send(client_socket, msg.c_str(), msg.length(), 0);
        cout << client_socket
             << "logginig has been detatched by reason of excess attempts"
             << endl;
        return false;
        close(client_socket);
    }
    return false;
}
string addNewUser(int client_socket) {
    ofstream file("userpwd.txt", ios::app);
    string name;
    string msg = "input your nick: ";
    send(client_socket, msg.c_str(), msg.length(), 0);
    char buffer[1024] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    name = string(buffer, bytes_received);
    string password;
    msg = "input your password: ";
    send(client_socket, msg.c_str(), msg.length(), 0);
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    password = string(buffer, bytes_received);
    file << '\n' + name + " : " + password;
    file.close();
    msg = "you have been registrated";
    send(client_socket, msg.c_str(), msg.length(), 0);
    return name;
}
string LogIn(int client_socket) {
    string msg = "input your login: ";
    send(client_socket, msg.c_str(), msg.length(), 0);

    // принимаем имя юзера
    char buffer[1024] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    string user = string(buffer, bytes_received);
    cout << "logging" << endl;
    buffer[1024] = {0};
    if (!is_Loginned_user(user)) {
        msg = "you are not logginned ";
        send(client_socket, msg.c_str(), msg.length(),
             0);  // говорим, что не залогинен
        msg = "you want to sign up [y/n]";
        send(client_socket, msg.c_str(), msg.length(),
             0);  // спрашиваем хочет ли
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        string ans = string(buffer, bytes_received);
        if (ans == "y") {
            return addNewUser(client_socket);
        } else {
            close(client_socket);
        }
    } else {
        // просим ввести пароль
        string msg = "input Password: ";
        send(client_socket, msg.c_str(), msg.length(), 0);
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        string password = string(buffer, bytes_received);
        cout << "pwd in login" << endl;
        int attempt = 0;
        // чекаем
        if (check_Pwd(client_socket, user, password, attempt)) {
            return user;
        }
    }
}
void history(string msg) {
    ofstream historyFile("history.txt", ios::app);
    historyFile << msg + '\n';
}
void getHistory(int client_socket) {
    ifstream historyFile("history.txt");
    string msg;
    while (!historyFile.eof()) {
        getline(historyFile, msg);
        msg += '\n';
        send(client_socket, msg.c_str(), msg.length(), 0);
    }
}

vector<ClientInfo> clients;

// Функция для обработки подключения нового клиента
void handle_new_connection(int client_socket,
                           struct sockaddr_in client_address) {
    // Добавляем информацию о клиенте в список
    string registrated_name = LogIn(client_socket);
    clients.push_back({client_socket, registrated_name, client_address});

    // Бесконечный цикл для приема сообщений от клиента
    while (true) {
        char buffer[1024] = {0};
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            // Клиент отключился
            cout << "Client disconnected" << endl;
            break;
        }

        string msg = string(buffer, bytes_received);
        if (msg == "~h") {
            getHistory(client_socket);
        } else {
            // добавляем адресанта
            for (auto& client : clients) {
                if (client.socket == client_socket) {
                    msg = client.user_name + ": " + msg;
                }
            }
            history(msg);
        }
        // Выводим сообщение на экран
        cout << "Received message from client: " << msg << endl;
        // Отправляем сообщение всем остальным клиентам
        for (auto& client : clients) {
            if (client.socket != client_socket) {
                send(client.socket, msg.c_str(), msg.length(), 0);
            }
        }
    }

    // Удаляем информацию о клиенте из списка
    auto it = find_if(clients.begin(), clients.end(), [&](const ClientInfo& c) {
        return c.socket == client_socket;
    });
    if (it != clients.end()) {
        clients.erase(it);
    }

    // Закрываем сокет клиента
    close(client_socket);
}

int main() {
    // Создаем сокет для прослушивания входящих подключений
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        cout << "Error creating socket" << endl;
        return 1;
    }

    // Настраиваем адрес сервера
    int PORT;
    cout << "input port: ";
    cin >> PORT;
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Связываем сокет с адресом сервера
    if (bind(server_socket, (struct sockaddr*)&server_address,
             sizeof(server_address)) < 0) {
        cout << "Error binding socket" << endl;
        close(server_socket);
        return 1;
    }

    // Начинаем прослушивать входящие подключения
    if (listen(server_socket, 5) < 0) {
        cout << "Error listening for connections" << endl;
        close(server_socket);
        return 1;
    }

    cout << "Server started. Listening for connections..." << endl;

    while (true) {
        // Принимаем входящее
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);
        int client_socket =
            accept(server_socket, (struct sockaddr*)&client_address,
                   &client_address_size);
        if (client_socket < 0) {
            cout << "Error accepting connection" << endl;
            continue;
        }

        // Запускаем новый поток для обработки подключения нового клиента
        thread t(handle_new_connection, client_socket, client_address);
        t.detach();
    }

    // Закрываем сокет сервера
    close(server_socket);

    return 0;
}
