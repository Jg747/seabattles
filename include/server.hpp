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
#include <match.hpp>

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

		std::vector<struct client*> *clients;
		Match *m;
		bool stop_serv;
		
		#ifdef _WIN32
		// WINSOCK
		#else
		fd_set sock_list;
		#endif

		bool add_new_client();
		bool handle_client_request(struct client *c);
		void reset_fd_set();
		string get_current_ip_address();

		void create_match();
		Match *get_match();

	public:	
		struct server_error error;

		Server(int port);
		~Server();

		bool start();
		void stop();
		void reset();
};

void thread_server(int port);

#endif