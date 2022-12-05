#ifndef __client_h__
#define __client_h__

#include <thread>

#include <server.hpp>
#include <msg.hpp>
#include <gui.hpp>

class Gui;

class Client {
    private:
        int client_socket;
        bool stop;
        fd_set fd_list;
        std::string error;

        Gui *g;
        Server *s;
        std::thread *t_server;

        void reset_fd_set();
        bool connect_to_server(std::string ip, int port);
        bool do_from_socket();

    public:
        Client();
        ~Client();

        void create_server();

        void start();
        void send_message(msg_creation *msg);
        void receive_message(msg_parsing *msg);
};

#endif