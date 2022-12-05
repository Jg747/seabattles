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

#include <server.hpp>
#include <common.hpp>
#include <msg.hpp>
#include <debug.hpp>

#ifdef _WIN32
// ...
#else
Server::Server(int port) {
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error.is_error = true;
		error.error += "- Server socket creation error\n";
	}

	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = port;
	local_addr.sin_port = htons(port);

	if (bind(server_socket, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
		error.is_error = true;
		error.error += "- Bind error\n";
	}

	if (listen(server_socket, 3) < 0) {
		error.is_error = true;
		error.error += "- Listen on socket error\n";
	}

	stop_serv = false;
}

Server::~Server() {
	for (auto c : *clients) {
		if (c->client_socket >= 0) {
			close(c->client_socket);
			delete c->p;
		}
	}
	shutdown(server_socket, SHUT_RDWR);
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
		return false;
	}

	clients = new std::vector<struct client_t*>();
	m = NULL;
	
	while (!stop_serv) {
		reset_fd_set();
		if (select(FD_SETSIZE, &sock_list, NULL, NULL, NULL) < 0) {
			this->error.is_error = true;
			this->error.error += "- Server::start()::select()\n";
			return false;
		}
		
		if (FD_ISSET(server_socket, &sock_list)) {
			add_new_client();
		}
		for (auto c : *clients) {
			if (FD_ISSET(c->client_socket, &sock_list)) {
				handle_client_request(c);
			}
		}
	}

	reset();

	return true;
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
}

bool Server::add_new_client() {
	socklen_t in_addr_size;
	struct sockaddr_in remote;

	if (clients->size() < MAX_CLIENTS) {
		in_addr_size = sizeof(remote);
		
		struct client_t *c = new struct client_t;
		c->client_socket = accept(server_socket, (struct sockaddr *)&remote, &in_addr_size);
		
		std::string msg = create_message(MSG_CONN_ACCEPTED);
		if (send(c->client_socket, msg.c_str(), msg.size(), 0) < 0) {
			// error
			error.is_error = true;
			error.error += "- Server::add_new_client()::send() | OK_ADD";
		}
		clients->push_back(c);
		return true;
	} else {
		// send error to client
		in_addr_size = sizeof(remote);
		int temp_client = accept(server_socket, (struct sockaddr *)&remote, &in_addr_size);
		if (temp_client < 0) {
			// error
			error.is_error = true;
			error.error += "- Server::add_new_client()::accept() | NO_ADD";
		} else {
			std::string msg = create_message(MSG_CONN_SERVER_FULL);
			if (send(temp_client, msg.c_str(), msg.size(), 0) < 0) {
				// error
				error.is_error = true;
				error.error += "- Server::add_new_client()::send() | NO_ADD";
			}
			close(temp_client);
		}
		return false;
	}
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

string Server::get_current_ip_address() {
	return "";
}

void Server::create_match() {
	m = new Match();
}

Match *Server::get_match() {
	return this->m;
}



void thread_server(Server *s) {
	s->start();
}
#endif