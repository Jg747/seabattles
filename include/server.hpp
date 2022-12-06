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
#include <msg.hpp>
#include <common.hpp>

struct client_t {
	Player *p;
	int client_socket;
};

struct server_error {
	bool is_error;
	std::string error;

	server_error() {
		is_error = false;
		error = "";
	}
};

class Server {
	private:
		int server_socket;

		std::vector<struct client_t*> *clients;
		Match *m;
		bool stop_serv;
		
		#ifdef _WIN32
		// WINSOCK
		#else
		fd_set sock_list;
		#endif

		bool add_new_client();
		bool handle_client_request(struct client_t *c);
		void reset_fd_set();

		void create_match();

		void send_message(int client_socket, msg_creation *msg);
        void receive_message(int client_socket, msg_parsing *msg);

	public:	
		struct server_error error;

		Server(int port);
		~Server();

		bool start();
		void stop();
		void reset();

		bool is_running();

		static string get_current_ip_address();
};

void thread_server(Server *s);

#endif