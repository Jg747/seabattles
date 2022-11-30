#ifndef __client_hpp__
#define __client_hpp__

#include <string>
#include <thread>

#include <player.hpp>

class Client {
    private:
        int client_socket;
        bool stop;
        std::thread *recv_thread;

        Player *p;
    
    public:
        Client(std::string ip, int port);
        ~Client();
};

#endif