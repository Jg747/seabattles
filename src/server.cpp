/*
This part is for multiplayer, this will start a new thread that
is going to be your server
*/

#include <server.hpp>
#include <common.hpp>
#include <msg.hpp>

#include <string>

#ifdef _WIN32
#else
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#ifdef _WIN32
Server::Server() {
	
}

Server::~Server() {
	
}

void Server::reset_fd_set() {
	
}

bool Server::start() {
	return true;
}

bool Server::add_new_client() {
	return true;
}

bool Server::handle_client_request(struct client *c) {
	return true;
}
#else
Server::Server() {
	memset(&error, 0, sizeof(error));

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error.is_error = true;
		error.error += "- Server socket creation error\n";
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVER_PORT);

	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		error.is_error = true;
		error.error += "- Bind error\n";
	}

	if (listen(server_socket, 3) < 0) {
		error.is_error = true;
		error.error += "- Listen on socket error\n";
	}
}

Server::~Server() {
	for (auto c : clients) {
		if (c->client_socket >= 0) {
			close(c->client_socket);
		}
	}
	shutdown(server_socket, SHUT_RDWR);
}

void Server::reset_fd_set() {
	FD_ZERO(&sock_list);
	for (auto c : clients) {
		FD_SET(c->client_socket, &sock_list);
	}
}

bool Server::start() {
	if (error.is_error) {
		return false;
	}
	
	int error;
	while (true) {
		error = select(FD_SETSIZE, &sock_list, NULL, NULL, NULL);
		if (error < 0) {
			this->error.is_error = true;
			this->error.error += "- Server::start()::select()\n";
			return false;
		}
		
		if (FD_ISSET(server_socket, &sock_list)) {
			add_new_client();
		}
		for (auto c : clients) {
			if (FD_ISSET(c->client_socket, &sock_list)) {
				handle_client_request(c);
			}
		}
	}

	return true;
}

bool Server::add_new_client() {
	if (clients.size() < MAX_CLIENTS) {
		in_addr_size = sizeof(in_addr);
		
		struct client *c = new struct client;
		c->client_socket = accept(server_socket, (struct sockaddr *)&in_addr, (socklen_t*)&in_addr_size);
		
		std::string msg = create_message(MSG_CONN_ACCEPTED);
		if (send(c->client_socket, msg.c_str(), msg.size(), 0) < 0) {
			// error
			error.is_error = true;
			error.error += "- Server::add_new_client()::send() | OK_ADD";
		}
		clients.push_back(c);
		return true;
	} else {
		// send error to client
		in_addr_size = sizeof(in_addr);
		int temp_client = accept(server_socket, (struct sockaddr *)&in_addr, (socklen_t*)&in_addr_size);
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

bool Server::handle_client_request(struct client *c) {
	
}
#endif