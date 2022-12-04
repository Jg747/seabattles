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

#include <server.hpp>
#include <client.hpp>
#include <gui.hpp>

#define BUF_LEN (1000+1)

Client::Client() {
    g = new Gui(this);
    s = NULL;
    t_server = NULL;
    stop = false;
}

Client::~Client() {
    delete g;
    if (s != NULL) {
        delete s;
    }
    close(client_socket);
}

void Client::reset_fd_set() {
    FD_ZERO(&fd_list);
    FD_SET(fileno(stdin), &fd_list);
    FD_SET(client_socket, &fd_list);
}

void Client::start() {
    if (!g->pregame()) {
        return;
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
}

bool Client::create_server() {
    if (s == NULL) {
        s = new Server(SERVER_PORT);
    } else {
        delete s;
        delete t_server;
        s = new Server(SERVER_PORT);
    }
    t_server = new std::thread(s->start());
}

bool Client::connect_to_server(std::string ip, int port) {
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        error = "[ERROR] Socket creation failed";
        return false;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(INADDR_ANY);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(client_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        error = "[ERROR] Binding failed";
        close(client_socket);
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
        return false;
    }
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

bool Client::do_from_socket() {
    // socket reading and merdate varie
}