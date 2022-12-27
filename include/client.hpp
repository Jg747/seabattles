#ifndef __client_h__
#define __client_h__

#include <thread>
#include <condition_variable>
#include <mutex>
#include <list>

#include <server.hpp>
#include <msg.hpp>
#include <gui.hpp>
#include <thread_manager.hpp>

class Gui;

class Client {
    private:
        int client_socket;
        bool stop;
        fd_set fd_list;
        std::string error;
        std::list<msg_parsing> msgs;

        Gui *g;
        Server *s;
        std::thread *t_server;
        std::thread *receiver;

        struct thread_manager_t *mng;

        void reset_player_list();

    public:
        Client(struct thread_manager_t *mng);
        ~Client();

        bool is_stop();
        bool do_from_socket();

        void create_server();
        void stop_server();

        std::string get_error();
        bool start();
        bool connect_to_server(std::string ip, int port);
        void send_message(msg_creation *msg);
        void receive_message();

        msg_parsing get_msg(enum msg_type_e type);
};

#endif