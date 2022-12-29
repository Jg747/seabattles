#ifndef __multi_h__
#define __multi_h__

#include <thread>
#include <vector>
#include <string>

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
	std::vector<msg_parsing> msgs;
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

		std::string ip_addr;
		std::vector<struct client_t*> *clients;
		Match *m;
		bool stop_serv;
		
		#ifdef _WIN32
		// WINSOCK
		#else
		fd_set sock_list;
		#endif

		bool add_new_client();
		struct client_t *get_client(int id);
		void remove_client(int client_socket);
		msg_parsing get_msg(struct client_t *c, enum msg_type_e type);

		bool handle_client_request(struct client_t *c);
		void reset_fd_set();

		void create_match();

		void send_message(int client_socket, msg_creation *msg);
        void receive_message(struct client_t *c);
		void receive_message(int client_socket, msg_parsing *msg);

		void handle_player_get_own_id(struct client_t *c, msg_parsing *msg);
		void handle_player_ship_placement(struct client_t *c, msg_parsing *msg);
		void handle_player_get_board(struct client_t *c, msg_parsing *msg);
		void handle_player_get_board_lost(struct client_t *c, msg_parsing *msg);
		void handle_player_attack(struct client_t *c, msg_parsing *msg);
		void handle_player_quit(struct client_t *c);
		void handle_host_init_match(struct client_t *c, msg_parsing *msg);
		void handle_host_start_match(struct client_t *c);
		void handle_host_player_kick(struct client_t *c, msg_parsing *msg);

		void send_server_error(struct client_t *c);
		void send_unknown_player(struct client_t *c);

		void append_info(Player *p, struct stats_t *info);

		void print_players();

	public:	
		struct server_error error;

		Server(int port);
		~Server();

		bool start();
		void stop();
		void reset();

		bool is_running();
		std::string get_ip();

		static std::string get_current_ip_address();
};

void thread_server(Server *s);

#endif