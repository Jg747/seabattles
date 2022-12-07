/*
This part is for multiplayer, this will start a new thread that
is going to be your server
*/

#include <string>
#include <string.h>

#ifdef _WIN32
#else
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <debug.hpp>
#include <server.hpp>
#include <common.hpp>
#include <msg.hpp>

#ifdef _WIN32
// ...
#else
Server::Server(int port) {
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error.is_error = true;
		error.error += "- Server socket creation error\n";
		server_socket = -1;
		return;
	}

	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(port);
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_socket, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
		error.is_error = true;
		error.error += "- Bind error\n";
		close(server_socket);
		server_socket = -1;
		return;
	}

	if (listen(server_socket, 3) < 0) {
		error.is_error = true;
		error.error += "- Listen on socket error\n";
		close(server_socket);
		server_socket = -1;
		return;
	}

	stop_serv = false;
	clients = NULL;
	m = NULL;
}

Server::~Server() {
	if (clients != NULL) {
		for (auto c : *clients) {
			if (c->client_socket >= 0) {
				close(c->client_socket);
				delete c->p;
			}
			delete c;
		}
	}

	if (server_socket >= 0) {
		close(server_socket);
	}
}

void Server::reset_fd_set() {
	FD_ZERO(&sock_list);
	FD_SET(server_socket, &sock_list);
	for (auto c : *clients) {
		FD_SET(c->client_socket, &sock_list);
	}
}

bool Server::start() {
	if (error.is_error) {
		Logger::write("Server error: " + error.error);
		return false;
	}
	Logger::write("Server started");

	clients = new std::vector<struct client_t*>();
	
	while (!stop_serv) {
		reset_fd_set();
		if (select(FD_SETSIZE, &sock_list, NULL, NULL, NULL) >= 0) {
			if (FD_ISSET(server_socket, &sock_list)) {
				add_new_client();
			}
			for (auto c : *clients) {
				if (FD_ISSET(c->client_socket, &sock_list)) {
					handle_client_request(c);
				}
			}	
		}
	}

	Logger::write("Server stopped");
	reset();

	return true;
}

void Server::send_message(int client_socket, msg_creation *msg) {
    std::string buffer = create_message(msg->msg_type, msg);
    Logger::write("[server] Sending " + string(MSG_TYPE_STR[msg->msg_type]));
    
    send(client_socket, buffer.c_str(), buffer.length(), 0);
}

void Server::receive_message(int client_socket, msg_parsing *msg) {
    char buffer[RECV_BUF_LEN];

    recv(client_socket, buffer, RECV_BUF_LEN, 0);
    std::string buf_string = std::string(buffer);
    parse_message(buf_string, msg);
    Logger::write("[server] Received " + string(MSG_TYPE_STR[msg->msg_type]));
}

void Server::stop() {
	this->stop_serv = true;
}

void Server::reset() {
	if (m != NULL) {
		delete m;
	}

	for (auto c : *clients) {
		if (c->client_socket >= 0) {
			close(c->client_socket);
			delete c->p;
		}
	}
	delete clients;

	Logger::write("Server reset");
}

bool Server::add_new_client() {
	socklen_t remote_len;
	struct sockaddr_in remote;

	int temp_client = 0;
	msg_creation c_msg;
	msg_parsing r_msg;
	std::string buf;
	
	bool result;

	if (clients->size() < MAX_CLIENTS) {
		remote_len = sizeof(remote);
		temp_client = accept(server_socket, (struct sockaddr *)&remote, &remote_len);

		c_msg.msg_type = MSG_CONN_ACCEPTED;
		send_message(temp_client, &c_msg);

		struct client_t *c = new struct client_t;
		c->client_socket = temp_client;
		clients->push_back(c);

		result = true;
	} else if (clients->size() >= MAX_CLIENTS) {
		remote_len = sizeof(remote);
		temp_client = accept(server_socket, (struct sockaddr *)&remote, &remote_len);
		
		c_msg.msg_type = MSG_CONN_SERVER_FULL;
		send_message(temp_client, &c_msg);

		result = false;
	} else if (m != NULL && m->get_status() == RUNNING) {
		remote_len = sizeof(remote);
		temp_client = accept(server_socket, (struct sockaddr *)&remote, &remote_len);
		
		c_msg.msg_type = MSG_CONN_MATCH_STARTED;
		send_message(temp_client, &c_msg);

		result = false;
	} else {
		result = false;
	}

	receive_message(temp_client, &r_msg);
	if (!result) {
		close(temp_client);
	}

	return result;
}

bool Server::is_running() {
	return this->stop_serv == false;
}

bool Server::handle_client_request(struct client_t *c) {
	size_t len;
	char buffer[1000+1];
	string received;

	if ((len = recv(c->client_socket, buffer, 1000+1, 0)) < 0) {
		this->error.is_error = true;
		this->error.error += "- Server::start()::handle_client_request()::recv()\n";
		return false;
	}

	return true;
}

void Server::create_match() {
	m = new Match();
}

string Server::get_current_ip_address() {
	return "";
}


// Server thread, non toccare con quelle tue luride manine
void thread_server(Server *s) {
	Logger::write("Starting server thread...");
	s->start();
}
#endif