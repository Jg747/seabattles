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
#include <string.h>
#include <thread>

#include <debug.hpp>
#include <client.hpp>
#include <server.hpp>
#include <gui.hpp>
#include <msg.hpp>
#include <common.hpp>
#include <thread_manager.hpp>

Client::Client(struct thread_manager_t *mng) {
    g = new Gui(this, mng);
    s = NULL;
    t_server = NULL;
    receiver = NULL;
    stop = false;
    client_socket = -1;
    this->mng = mng;
    reset_waiting_msg(mng);
}

Client::~Client() {
    delete g;
    stop_server();
    stop_receiver();
    if (client_socket >= 0) {
        close(client_socket);
    }
}

void Client::stop_receiver() {
    if (receiver != NULL) {
        this->stop = true;
        if (receiver->joinable()) {
            receiver->join();
        }
        delete receiver;
    }
    receiver = NULL;
}

void Client::stop_server() {
    if (s != NULL) {
        s->stop();
        delete s;
    }
    s = NULL;
    
    if (t_server != NULL) {
        if (t_server->joinable()) {
            t_server->join();
        }
        delete t_server;
    }
    t_server = NULL;
    
    stop_receiver();
}

bool Client::start() {
    int temp = g->pregame();
    
    if (temp == 0) {
        stop_server();
        Logger::write("[client] Program terminated");
        return false;
    } else if (temp == 2) {
        stop_server();
        Logger::write("[client] Retrying mode choice");
        return true;
    }
    Logger::write("[client] Pregame successfully passed");

    g->start_game();

    stop_server(); // PROBLEMA QUI

    Logger::write("cose");

    Logger::write("[client] Main menu");
    return true;
}

bool Client::do_from_socket() {
    wait_socket();
    receive_message();
    
    for (size_t i = 0; i < this->msgs.size(); i++) {
        msg_parsing r_msg = msgs[i];
        // Messaggi che sono in attesa di essere ricevuti
        if (r_msg.msg_type >= mng->waiting_message_low && r_msg.msg_type <= mng->waiting_message_high) {
            unlock(mng);
        } else {
            // Messaggi asincroni del server
            switch (r_msg.msg_type) {
                case MSG_PLAYER_LIST:
                case MSG_MATCH_PLAYER_REMOVED:
                    reset_player_list();
                    i--;
                    break;
            
                case MSG_MATCH_STARTED:
                    handle_match_started();
                    i--;
                    break;
                case MSG_MATCH_TURN:
                    g->turn(r_msg.data.match_turn.turn);
                    msgs.erase(msgs.begin() + i);
                    i--;
                    break;
                case MSG_MATCH_NEW_BOARD:
                    g->set_new_board(&r_msg);
                    msgs.erase(msgs.begin() + i);
                    i--;
                    break;
                
                case MSG_MATCH_LOSE:
                    g->turn(false);
                    g->end_game_win(&r_msg);
                    msgs.erase(msgs.begin() + i);
                    i--;
                    break;
                
                case MSG_MATCH_WIN:
                case MSG_MATCH_END:
                    g->end_game_win(&r_msg);
                    stop = true;
                    msgs.erase(msgs.begin() + i);
                    i--;
                    break;

                case MSG_MATCH_GOT_KICKED:
                    g->got_kicked(&r_msg);
                    stop = true;
                    msgs.erase(msgs.begin() + i);
                    i--;
                    break;
                
                default:
                    msgs.erase(msgs.begin() + i);
                    i--;
                    break;
            }
        }
    }

    return true;
}

void Client::wait_socket() {
    fd_set set;
    FD_ZERO(&set);
    FD_SET(client_socket, &set);
    select(FD_SETSIZE, &set, NULL, NULL, NULL);
}

void Client::create_server() {
    s = new Server(SERVER_PORT);
    Logger::write("[client] Created server (port: " + std::to_string(SERVER_PORT) + ")");
    t_server = new std::thread(thread_server, s);
}

void Client::create_receiver() {
    this->stop = false;
    receiver = new std::thread(thread_receiver, this);
    Logger::write("[client] New receiver created");
}

bool Client::connect_to_server(std::string ip, int port) {
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        error = "Socket creation failed";
        client_socket = -1;
        return false;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(INADDR_ANY);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(client_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        error = "Binding failed";
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
        error  = "Connect failed";
        close(client_socket);
        client_socket = -1;
        return false;
    }

    create_receiver();

    wait_for(mng, MSG_CONN_ACCEPTED, MSG_CONN_SERVER_FULL);

    msg_parsing msg;
    msg = get_msg(MSG_CONN_ACCEPTED);
    if (msg.msg_type == UNKNOWN) {
        msg = get_msg(MSG_CONN_ERR);
        if (msg.msg_type == UNKNOWN) {
            msg = get_msg(MSG_CONN_MATCH_STARTED);
            if (msg.msg_type == UNKNOWN) {
                msg = get_msg(MSG_CONN_SERVER_FULL);
            }
        }
    }

    msg_creation c_msg;
    c_msg.msg_type = ACK;
    send_message(&c_msg);

    if (msg.msg_type == MSG_CONN_ACCEPTED) {
        return true;
    } else {
        error = string(MSG_TYPE_STR[msg.msg_type]);

        close(client_socket);
        client_socket = -1;
        g->conn_err(&msg);
        stop = true;

        return false;
    }
}

void Client::send_message(msg_creation *msg) {
    std::string buffer = create_message(msg->msg_type, msg);
    Logger::write("[client] Sending " + string(MSG_TYPE_STR[msg->msg_type]));
    
    send(client_socket, buffer.c_str(), buffer.length(), 0);
}

void Client::receive_message() {
    char buffer[RECV_BUF_LEN];
    std::string buf_string = "";
    size_t pos;
    std::string temp;
    msg_parsing msg;

    while (recv(client_socket, buffer, RECV_BUF_LEN, 0) == RECV_BUF_LEN) {
        buf_string += std::string(buffer);
    }
    buf_string += std::string(buffer);

    while ((pos = buf_string.find("<?xml", 1)) != std::string::npos) {
        temp = buf_string.substr(0, pos);
        buf_string = buf_string.substr(pos);
        parse_message(temp, &msg);
        Logger::write("[client] Received " + string(MSG_TYPE_STR[msg.msg_type]));
        msgs.push_back(msg);
    }
    parse_message(buf_string, &msg);
    Logger::write("[client] Received " + string(MSG_TYPE_STR[msg.msg_type]));
    msgs.push_back(msg);
}

void Client::reset_player_list() {
    msg_parsing msg;
    msg = get_msg(MSG_PLAYER_LIST);
    if (msg.msg_type == UNKNOWN) {
        msg = get_msg(MSG_MATCH_PLAYER_REMOVED);
    }

    std::map<int, Player*> *list = g->get_player_list();
    std::map<int, std::string> names;
    std::map<int, Player*> new_list;

    for (auto &p : msg.data.msg_player_list.array) {
        names.insert({p.player_id, p.name});
        new_list.insert({p.player_id, NULL});
    }
    
    for (auto &p : *list) {
        if (new_list.contains(p.first)) {
            new_list[p.first] = p.second;
        } else {
            delete p.second;
        }
    }

    for (auto &p : new_list) {
        if (p.second == NULL) {
            p.second = new Player(false);
            p.second->set_id(p.first);
            p.second->set_name(names[p.first]);
        }
    }
    
    g->set_player_list(new_list);

    msg_creation c_msg;
    c_msg.msg_type = ACK;
    send_message(&c_msg);
}

msg_parsing Client::get_msg(enum msg_type_e type) {
    for (size_t i = 0; i < this->msgs.size(); i++) {
        if (msgs[i].msg_type == type) {
            msg_parsing msg = msgs[i];
            msgs.erase(msgs.begin() + i);
            return msg;
        }
    }
    msg_parsing m;
    m.msg_type = UNKNOWN;
    return m;
}

void Client::handle_match_started() {
    msg_parsing r_msg = get_msg(MSG_MATCH_STARTED);
    msg_creation msg;
    msg.msg_type = ACK;
    send_message(&msg);
    
    g->game_starting();
}

bool Client::is_stop() {
    return this->stop;
}

std::string Client::get_error() {
    return this->error;
}