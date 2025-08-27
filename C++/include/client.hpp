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
        msg_parsing r_msg;

        Gui *g;
        Server *s;
        std::thread *t_server;

        void reset_fd_set();
        bool do_from_socket();

        void reset_player_list();

    public:
        Client();
        ~Client();

        void create_server();
        void stop_server();

        std::string get_error();
        bool start();
        bool connect_to_server(std::string ip, int port);
        void send_message(msg_creation *msg);
        void receive_message(msg_parsing *msg);
};

#endif