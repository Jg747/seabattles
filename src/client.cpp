#ifdef _WIN32
// WINSOCK
#else
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <string>
#include <thread>

#include <client.hpp>
#include <server.hpp>
#include <gui.hpp>

#define BUF_LEN (1000+1)

Client::Client() {
    g = new Gui(this);
    s = NULL;
    t_server = NULL;
    stop = true;
    client_socket = -1;
}

Client::~Client() {
    delete g;
    if (s != NULL) {
        s->stop();
        delete s;
        s = NULL;
        if (t_server != NULL) {
            t_server->join();
            delete t_server;
        }
        t_server = NULL;
    }
    if (client_socket >= 0) {
        close(client_socket);
    }
}

void Client::reset_fd_set() {
    FD_ZERO(&fd_list);
    FD_SET(fileno(stdin), &fd_list);
    FD_SET(client_socket, &fd_list);
}

bool Client::start() {
    if (!g->pregame()) {
        if (s->is_running()) {
            s->stop();
        }
        return false;
    }

    do {
        reset_fd_set();
        if (select(FD_SETSIZE, &fd_list, NULL, NULL, NULL) >= 0) {
            if (FD_ISSET(fileno(stdin), &fd_list)) {
                if (!g->do_from_input()) {
                    stop = true;
                }
            }
            if (FD_ISSET(client_socket, &fd_list)) {
                if (!this->do_from_socket()) {
                    stop = true;
                }
            }
        }
    } while (!stop);

    return true;
}

void Client::create_server() {
    if (s == NULL) {
        s = new Server(SERVER_PORT);
    } else {
        delete s;
        delete t_server;
        s = new Server(SERVER_PORT);
    }
    t_server = new std::thread(thread_server, s);
}

bool Client::connect_to_server(std::string ip, int port) {
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        error = "[ERROR] Socket creation failed";
        client_socket = -1;
        return false;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(INADDR_ANY);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(client_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        error = "[ERROR] Binding failed";
        close(client_socket);
        client_socket = -1;
        return false;
    }

    struct hostent *h;
    h = gethostbyname(ip.c_str());
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = *((int*) h->h_addr_list[0]);

    if (connect(client_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        error  = "[ERROR] Connect failed";
        close(client_socket);
        client_socket = -1;
        return false;
    }

    return true;
}

void Client::send_message(msg_creation *msg) {
    std::string buffer = create_message(msg->msg_type, msg);
    
    send(client_socket, buffer.c_str(), buffer.length(), 0);
}

void Client::receive_message(msg_parsing *msg) {
    char buffer[BUF_LEN];

    recv(client_socket, buffer, BUF_LEN, 0);
    std::string buf_string = std::string(buffer);
    parse_message(buf_string, msg);
}

void Client::reset_player_list() {
    std::map<int, Player*> *list = g->get_player_list();
    std::map<int, std::string> names;
    std::map<int, Player*> new_list;
    
    for (auto p : r_msg.data.msg_player_list.array) {
        names.insert({p.player_id, p.name});
        new_list.insert({p.player_id, NULL});
    }
    
    for (auto p : *list) {
        if (new_list.contains(p.first)) {
            new_list[p.first] = p.second;
        } else {
            delete p.second;
        }
    }

    for (auto p : new_list) {
        if (p.second == NULL) {
            p.second = new Player(false);
            p.second->set_id(p.first);
            p.second->set_name(names[p.first]);
        }
    }
    
    g->set_player_list(new_list);
}

bool Client::do_from_socket() {
    this->receive_message(&this->r_msg);

    switch (r_msg.msg_type) {
        case MSG_CONN_ERR:
        case MSG_CONN_SERVER_FULL:
        case MSG_CONN_MATCH_STARTED:
            g->conn_err(&this->r_msg);
            stop = true;
            break;
        
        case MSG_PLAYER_LIST:
        case MSG_MATCH_PLAYER_REMOVED:
            reset_player_list();
            break;
    
        case MSG_MATCH_STARTED:
            g->game_starting();
            break;
        case MSG_MATCH_TURN:
            g->turn(this->r_msg.data.match_turn.turn);
            break;
        case MSG_MATCH_NEW_BOARD:
            g->set_new_board(&this->r_msg);
            break;
        
        case MSG_MATCH_WIN:
        case MSG_MATCH_LOSE:
        case MSG_MATCH_END:
            g->end_game_win(&this->r_msg);
            stop = true;
            break;

        case MSG_MATCH_GOT_KICKED:
            g->got_kicked(&this->r_msg);
            stop = true;
            break;
        
        default:
            break;
    }

    return true;
}