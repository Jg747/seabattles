#ifndef __multi_h__
#define __multi_h__

#include <thread>
#include <vector>

#ifdef _WIN32

#else
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <player.hpp>

#define MAX_CLIENTS 8

struct client {
	Player *p;
	int client_socket;
};

struct server_error {
	bool is_error;
	std::string error;
};

class Server {
	private:
		int server_socket;

		std::vector<struct client*> clients;
		
		#ifdef _WIN32
		// WINSOCK
		#else
		struct sockaddr_in server_addr;
		struct sockaddr_in in_addr;
		socklen_t in_addr_size;
		struct fd_set sock_list;
		#endif

		bool add_new_client();
		bool handle_client_request(struct client *c);
		void reset_fd_set();

	public:	
		struct server_error error;

		Server();
		~Server();

		bool start();
		void stop();
};

#endif