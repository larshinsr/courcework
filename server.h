#ifndef SERVER_H
#define SERVER_H
#include <arpa/inet.h>

#include <string>
#include <vector>

#include "client.h"
using std::string;
class Server {
   private:
    std::vector<Client> clients;

   public:
    void handle_new_connection(int client_socket,
                               struct sockaddr_in client_address);
    void start_server();
};
#endif
