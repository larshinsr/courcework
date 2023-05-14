#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

#include <string>

class Client {
   private:
    int socket;
    std::string user_name;
    struct sockaddr_in address;

   public:
    Client(int client_socket, const std::string& name,
           const struct sockaddr_in& client_address);

    int getSocket() const;
    const std::string& getUserName() const;
    const struct sockaddr_in& getAddress() const;

    bool is_Loginned_user(const std::string& name) const;
    bool check_Pwd(int attempt, const std::string& name,
                   const std::string& password);
    std::string addNewUser();
    std::string LogIn();
};

#endif  // CLIENT_H
