/*
This part is for multiplayer, this will start a new thread that
is going to be your server
*/

#include <string>
#include <string.h>

#ifdef _WIN32
// ...
#else
#include <curl/curl.h>
#include <netdb.h>
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
#include <gui.hpp>

Server::Server(int port) {
	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error.is_error = true;
		error.error = "Server socket creation error";
		server_socket = -1;
		return;
	}

	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(port);
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_socket, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
		error.is_error = true;
		error.error = "Bind error";
		close(server_socket);
		server_socket = -1;
		return;
	}

	if (listen(server_socket, 3) < 0) {
		error.is_error = true;
		error.error = "Listen on socket failed";
		close(server_socket);
		server_socket = -1;
		return;
	}

	stop_serv = false;
	clients = NULL;
	m = NULL;
	// ip_addr = Server::get_current_ip_address();
}

Server::~Server() {
	if (clients != NULL) {
		for (size_t i = 0; i < clients->size(); i++) {
			struct client_t *c = clients->at(i);
			if (c->client_socket >= 0) {
				close(c->client_socket);
				if (c->p != NULL) {
					delete c->p;
					c->p = NULL;
				}
			}
			if (c != NULL) {
				delete c;
				c = NULL;
			}
		}
		delete clients;
		clients = NULL;
	}

	if (m != NULL) {
		delete m;
		m = NULL;
	}

	if (server_socket >= 0) {
		close(server_socket);
	}
}

void Server::reset_fd_set() {
	FD_ZERO(&sock_list);
	FD_SET(server_socket, &sock_list);
	for (auto &c : *clients) {
		FD_SET(c->client_socket, &sock_list);
	}
}

bool Server::start() {
	if (error.is_error) {
		Logger::write("[server][ERROR] " + error.error);
		return false;
	}
	Logger::write("[server] Server started");

	clients = new std::vector<struct client_t*>();
	create_match();
	
	struct timeval t = { .tv_sec = 0, .tv_usec = 500000 };
	while (!stop_serv) {
		reset_fd_set();
		if (select(FD_SETSIZE, &sock_list, NULL, NULL, &t) > 0) {
			if (FD_ISSET(server_socket, &sock_list)) {
				add_new_client();
			}
			for (auto &c : *clients) {
				if (FD_ISSET(c->client_socket, &sock_list)) {
					handle_client_request(c);
				}
			}	
		}
	}

	Logger::write("[server] Server stopped");

	return true;
}

void Server::send_message(int client_socket, msg_creation *msg) {
    std::string buffer = create_message(msg->msg_type, msg);
    Logger::write("[server][" + std::to_string(client_socket) + "] Sending " + string(MSG_TYPE_STR[msg->msg_type]));
    
    send(client_socket, buffer.c_str(), buffer.length(), 0);
}

void Server::receive_message(int client_socket, msg_parsing *msg) {
    char buffer[RECV_BUF_LEN];

    recv(client_socket, buffer, RECV_BUF_LEN, 0);
    std::string buf_string = std::string(buffer);
    parse_message(buf_string, msg);
    Logger::write("[server][" + std::to_string(client_socket) + "] Received " + string(MSG_TYPE_STR[msg->msg_type]));
}

void Server::receive_message(struct client_t *c) {
    char buffer[RECV_BUF_LEN];
    std::string buf_string = "";
    size_t pos;
    std::string temp;
    msg_parsing msg;

    while (recv(c->client_socket, buffer, RECV_BUF_LEN, 0) == RECV_BUF_LEN) {
        buf_string += std::string(buffer);
    }
    buf_string += std::string(buffer);

    while ((pos = buf_string.find("<?xml", 1)) != std::string::npos) {
        temp = buf_string.substr(0, pos);
        buf_string = buf_string.substr(pos);
        parse_message(temp, &msg);
        Logger::write("[server][" + std::to_string(c->client_socket) + "] Received " + string(MSG_TYPE_STR[msg.msg_type]));
        c->msgs.push_back(msg);
    }
    parse_message(buf_string, &msg);
    Logger::write("[server][" + std::to_string(c->client_socket) + "] Received " + string(MSG_TYPE_STR[msg.msg_type]));
    c->msgs.push_back(msg);
}

void Server::stop() {
	this->stop_serv = true;
}

bool Server::add_new_client() {
	socklen_t remote_len;
	struct sockaddr_in remote;

	int temp_client = 0;
	msg_creation c_msg;
	msg_parsing r_msg;
	std::string buf;
	
	bool result;

	remote_len = sizeof(remote);
	temp_client = accept(server_socket, (struct sockaddr *)&remote, &remote_len);
	Logger::write("[server][" + std::to_string(temp_client) + "] Connected");

	if (clients != NULL && clients->size() < MAX_CLIENTS) {
		c_msg.msg_type = MSG_CONN_ACCEPTED;
		send_message(temp_client, &c_msg);

		struct client_t *c = new struct client_t;
		c->client_socket = temp_client;
		if (clients->empty()) {
			c->p = new Player(true);
		} else {
			c->p = new Player(false);
		}
		clients->push_back(c);
		m->add_player(c->p);

		result = true;
		receive_message(temp_client, &r_msg);
	} else if (clients != NULL && clients->size() >= MAX_CLIENTS) {
		c_msg.msg_type = MSG_CONN_SERVER_FULL;
		send_message(temp_client, &c_msg);

		result = false;
		receive_message(temp_client, &r_msg);
	} else if (m != NULL && m->get_status() == RUNNING) {
		c_msg.msg_type = MSG_CONN_MATCH_STARTED;
		send_message(temp_client, &c_msg);

		result = false;
		receive_message(temp_client, &r_msg);
	} else {
		result = false;
	}

	if (!result) {
		close(temp_client);
	}

	return result;
}

bool Server::is_running() {
	return this->stop_serv == false;
}

bool Server::handle_client_request(struct client_t *c) {
	receive_message(c);

	for (size_t i = 0; i < c->msgs.size(); i++) {
		msg_parsing msg = c->msgs[i];
		switch (msg.msg_type) {
			case MSG_PLAYER_GET_OWN_ID:
				handle_player_get_own_id(c, &msg);
				break;
			case MSG_PLAYER_SHIP_PLACEMENT:
				handle_player_ship_placement(c, &msg);
				break;
			case MSG_PLAYER_GET_BOARD:
				handle_player_get_board(c, &msg);
				break;
			case MSG_PLAYER_GET_BOARD_LOST:
				handle_player_get_board_lost(c, &msg);
				break;
			case MSG_PLAYER_ATTACK:
				handle_player_attack(c, &msg);
				break;
			case MSG_PLAYER_QUIT:
				handle_player_quit(c);
				break;
			case MSG_HOST_INIT_MATCH:
				handle_host_init_match(c, &msg);
				break;
			case MSG_HOST_START_MATCH:
				handle_host_start_match(c);
				break;
			case MSG_HOST_PLAYER_KICK:
				handle_host_player_kick(c, &msg);
				break;
			default:
				break;
		}
		c->msgs.erase(c->msgs.begin() + i);
		i--;
	}

	return true;
}

void Server::create_match() {
	m = new Match();
}

std::string Server::get_ip() {
	return this->ip_addr;
}

static size_t ip_to_string(void *contents, size_t size, size_t nmemb, void *userp) { 
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string Server::get_current_ip_address() {
	CURL *curl;
	std::string ip_buf;
	
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ip_to_string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip_buf);
		curl_easy_perform(curl);
    	curl_easy_cleanup(curl);
		return ip_buf;
	} else {
		return "";
	}
}

void Server::handle_player_get_own_id(struct client_t *c, msg_parsing *msg) {
	if (m == NULL) {
		send_server_error(c);
		return;
	}

	c->p->set_name(msg->data.player_get_own_id.username);
	msg_creation c_msg;
	c_msg.msg_type = ACK_MSG_PLAYER_GET_OWN_ID;
	c_msg.data.ack_player_get_own_id.player_id = c->p->get_id();
	send_message(c->client_socket, &c_msg);

	msg_creation player_list;
	msg_parsing r_msg;
	player_list.msg_type = MSG_PLAYER_LIST;
	std::vector<Player*> *players = m->get_players();
	for (auto &p : *players) {
		if (!p->is_dead()) {
			struct player_info info;
			info.name = p->get_name();
			info.player_id = p->get_id();
			player_list.data.msg_player_list.array.push_back(info);
		}
	}

	for (auto &p : *clients) {
		send_message(p->client_socket, &player_list);
		//receive_message(p->client_socket, &r_msg);
	}
}

msg_parsing Server::get_msg(struct client_t *c, enum msg_type_e type) {
    for (size_t i = 0; i < c->msgs.size(); i++) {
        if (c->msgs[i].msg_type == type) {
            msg_parsing msg = c->msgs[i];
            c->msgs.erase(c->msgs.begin() + i);
            return msg;
        }
    }
    return { .msg_type = UNKNOWN };
}

void Server::handle_player_ship_placement(struct client_t *c, msg_parsing *msg) {
	if (m == NULL) {
		send_server_error(c);
		return;
	}

	msg_creation c_msg;
	Board *b = c->p->get_board();
	Ship **ships = b->get_ships();
	bool error = false;
	int j;
	for (int i = 0; i < SHIPS_COUNT; i++) {
		Ship *s = NULL;
		for (j = 0; j < SHIPS_COUNT; j++) {
			if (ships[j]->get_type() == msg->data.player_ship_placement.array[i].type) {
				s = ships[j];
				break;
			}
		}
		if (s != NULL) {
			s->set_x(msg->data.player_ship_placement.array[i].x);
			s->set_y(msg->data.player_ship_placement.array[i].y);
			s->set_rotation(msg->data.player_ship_placement.array[i].rotation);
			if (!b->insert_ship(j, PLACE)) {
				error = true;
				break;
			}
		}
	}

	if (error) {
		c_msg.msg_type = ACK_INVALID_SHIP_PLACEMENT;
		c_msg.data.ack_invalid_ship_placement.player_id = c->p->get_id();
		send_message(c->client_socket, &c_msg);
	} else {
		c->p->set_placed_ships(true);

		c_msg.msg_type = ACK;
		c_msg.data.ack.player_id = c->p->get_id();
		send_message(c->client_socket, &c_msg);

		if (m->can_start()) {
			m->start_match();
			msg_creation turn;
			msg_parsing recv;
			for (auto &p : *clients) {
				turn.msg_type = MSG_MATCH_TURN;
				turn.data.match_turn.turn = p->p->his_turn();
				send_message(c->client_socket, &turn);
				//receive_message(c->client_socket, &recv);
			}
		}
	}
}

void Server::handle_player_get_board(struct client_t *c, msg_parsing *msg) {
	if (m == NULL) {
		send_server_error(c);
		return;
	}

	Player *p = m->get_player_by_id(msg->data.player_get_board.id);
	if (p == NULL) {
		send_unknown_player(c);
		return;
	}

	msg_creation c_msg;
	c_msg.msg_type = ACK_MSG_GET_BOARD;
	c_msg.data.ack_get_board.client.player_id = c->p->get_id();
	c_msg.data.ack_get_board.player.player_id = p->get_id();

	Ship **ships = p->get_board()->get_ships();
	int **matrix = p->get_board()->get_board();
	bool blue = true;
	int k;

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (matrix[i][j] > 0) {
				if (matrix[i][j] == DAMAGE) {
					c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_NOT_HIT;
				} else if (matrix[i][j] > DAMAGE) {
					for (k = 0; k < SHIPS_COUNT; k++) {
						if (ships[k]->point_intersect(j, i)) {
							break;
						}
					}
					if (ships[k]->is_sunk()) {
						c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_SUNK;
					} else {
						c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_HIT;
					}
				} else {
					if (blue) {
						c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_BLUE_TILE;
					} else {
						c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_AQUA_TILE;
					}
				}
			} else {
				if (blue) {
					c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_BLUE_TILE;
				} else {
					c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_AQUA_TILE;
				}
			}
			blue = !blue;
		}
		if (BOARD_SIZE % 2 == 0) {
			blue = !blue;
		}
	}
}

void Server::handle_player_get_board_lost(struct client_t *c, msg_parsing *msg) {
	if (m == NULL) {
		send_server_error(c);
		return;
	}

	msg_creation c_msg;

	if (c->p->is_dead()) {
		Player *p = m->get_player_by_id(msg->data.player_get_board.id);
		if (p == NULL) {
			send_unknown_player(c);
			return;
		}

		c_msg.msg_type = ACK_MSG_GET_BOARD_LOST;
		c_msg.data.ack_get_board.client.player_id = c->p->get_id();
		c_msg.data.ack_get_board.player.player_id = p->get_id();

		Ship **ships = p->get_board()->get_ships();
		int **matrix = p->get_board()->get_board();
		bool blue = true;
		int k;

		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++) {
				if (matrix[i][j] > 0) {
					if (matrix[i][j] == DAMAGE) {
						c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_NOT_HIT;
					} else if (matrix[i][j] > DAMAGE) {
						for (k = 0; k < SHIPS_COUNT; k++) {
							if (ships[k]->point_intersect(j, i)) {
								break;
							}
						}
						if (ships[k]->is_sunk()) {
							c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_SUNK;
						} else {
							c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_HIT;
						}
					} else {
						c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_SHIP;
					}
				} else {
					if (blue) {
						c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_BLUE_TILE;
					} else {
						c_msg.data.ack_get_board.board.matrix[i][j] = COLOR_AQUA_TILE;
					}
				}
				blue = !blue;
			}
			if (BOARD_SIZE % 2 == 0) {
				blue = !blue;
			}
		}
	} else {
		c_msg.msg_type = ACK_MSG_MATCH_NOT_DEAD;
		c_msg.data.ack_match_not_dead.player_id = c->p->get_id();
	}

	send_message(c->client_socket, &c_msg);
}

void Server::handle_player_attack(struct client_t *c, msg_parsing *msg) {
	if (m == NULL) {
		send_server_error(c);
		return;
	}

	struct client_t *defender = get_client(msg->data.player_attack.player.player_id);
	if (defender == NULL) {
		send_unknown_player(c);
		return;
	}

	if (!c->p->his_turn()) {
		msg_creation c_msg;
		c_msg.msg_type = ACK_MSG_MATCH_ATTACK_ERR;
		c_msg.data.ack_match_attack_err.player.player_id = c->p->get_id();
		c_msg.data.ack_match_attack_err.status = NOT_YOUR_TURN;
		send_message(c->client_socket, &c_msg);
		return;
	}

	if (c->p->is_dead()) {
		msg_creation c_msg;
		c_msg.msg_type = ACK_MSG_MATCH_ATTACK_ERR;
		c_msg.data.ack_match_attack_err.player.player_id = c->p->get_id();
		c_msg.data.ack_match_attack_err.status = DEAD_CANNOT_ATTACK;
		send_message(c->client_socket, &c_msg);
		return;
	}

	if (msg->data.player_attack.x < 0 || 
		msg->data.player_attack.x >= BOARD_SIZE || 
		msg->data.player_attack.y < 0 || 
		msg->data.player_attack.y >= BOARD_SIZE) {
		msg_creation c_msg;
		c_msg.msg_type = ACK_MSG_MATCH_ATTACK_ERR;
		c_msg.data.ack_match_attack_err.player.player_id = c->p->get_id();
		c_msg.data.ack_match_attack_err.status = INVALID_ATTACK;
		send_message(c->client_socket, &c_msg);
		return;
	}

	struct attacked_player *atk = c->p->get_attack(defender->p);
	if (atk->attacked) {
		msg_creation c_msg;
		c_msg.msg_type = ACK_MSG_MATCH_ATTACK_ERR;
		c_msg.data.ack_match_attack_err.player.player_id = c->p->get_id();
		c_msg.data.ack_match_attack_err.status = NOT_SAME_PLAYER;
		send_message(c->client_socket, &c_msg);
		return;
	}

	int **matrix = defender->p->get_board()->get_board();
	if (matrix[msg->data.player_attack.y][msg->data.player_attack.x] >= DAMAGE) {
		msg_creation c_msg;
		c_msg.msg_type = ACK_MSG_MATCH_ATTACK_STATUS;
		c_msg.data.ack_match_attack_status.player.player_id = c->p->get_id();
		c_msg.data.ack_match_attack_status.status = FAILED_ATTACK;
		send_message(c->client_socket, &c_msg);
		return;
	}

	atk->attacked = true;

	msg_creation c_msg;
	c_msg.msg_type = ACK_MSG_MATCH_ATTACK_STATUS;
	if (matrix[msg->data.player_attack.y][msg->data.player_attack.x] == 0) {
		matrix[msg->data.player_attack.y][msg->data.player_attack.x] += DAMAGE;

		c_msg.data.ack_match_attack_status.status = MISSED;
		c->p->inc_misses();
	} else if (matrix[msg->data.player_attack.y][msg->data.player_attack.x] > 0) {
		matrix[msg->data.player_attack.y][msg->data.player_attack.x] += DAMAGE;

		int k;
		Ship **ships = defender->p->get_board()->get_ships();
		for (k = 0; k < SHIPS_COUNT; k++) {
			if (ships[k]->point_intersect(msg->data.player_attack.x, msg->data.player_attack.y)) {
				break;
			}
		}
		ships[k]->add_hit();
		if (ships[k]->is_sunk()) {
			c_msg.data.ack_match_attack_status.status = HIT_SUNK;
			c->p->inc_sunk_ships();
			defender->p->dec_remaining_ships();
		} else {
			c_msg.data.ack_match_attack_status.status = HIT;
			c->p->inc_hits();
		}
	}
	send_message(c->client_socket, &c_msg);

	msg_parsing recv;
	msg_creation new_board;
	new_board.msg_type = MSG_MATCH_NEW_BOARD;
	new_board.data.match_new_board.player.player_id = defender->p->get_id();
	new_board.data.match_new_board.attacker.name = c->p->get_name();

	Ship **ships = defender->p->get_board()->get_ships();
	matrix = defender->p->get_board()->get_board();
	bool blue = true;
	int k;

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (matrix[i][j] > 0) {
				if (matrix[i][j] == DAMAGE) {
					new_board.data.match_new_board.board.matrix[i][j] = COLOR_NOT_HIT;
				} else if (matrix[i][j] > DAMAGE) {
					for (k = 0; k < SHIPS_COUNT; k++) {
						if (ships[k]->point_intersect(j, i)) {
							break;
						}
					}
					if (ships[k]->is_sunk()) {
						new_board.data.match_new_board.board.matrix[i][j] = COLOR_SUNK;
					} else {
						new_board.data.match_new_board.board.matrix[i][j] = COLOR_HIT;
					}
				} else {
					if (blue) {
						new_board.data.match_new_board.board.matrix[i][j] = COLOR_BLUE_TILE;
					} else {
						new_board.data.match_new_board.board.matrix[i][j] = COLOR_AQUA_TILE;
					}
				}
			} else {
				new_board.data.match_new_board.board.matrix[i][j] = COLOR_SHIP;
			}
			blue = !blue;
		}
		if (BOARD_SIZE % 2 == 0) {
			blue = !blue;
		}
	}

	send_message(defender->client_socket, &new_board);
	//receive_message(defender->client_socket, &recv);

	if (m->eliminated(defender->p)) {
		defender->p->set_loser(true);

		msg_creation lose;
		msg_parsing recv;
		
		lose.msg_type = MSG_MATCH_LOSE;
		lose.data.match_lose.player.player_id = defender->p->get_id();
		append_info(defender->p, &lose.data.match_lose.info);
		send_message(defender->client_socket, &lose);
		//receive_message(defender->client_socket, &recv);

		msg_creation p_removed;
		p_removed.msg_type = MSG_MATCH_PLAYER_REMOVED;
		p_removed.data.match_player_removed.player.name = defender->p->get_name();
		p_removed.data.match_player_removed.reason = "LOST";

		std::vector<Player*> *players = m->get_players();
		for (auto &p : *players) {
			if (!p->is_dead()) {
				struct player_info info;
				info.name = p->get_name();
				info.player_id = p->get_id();
				p_removed.data.match_player_removed.list.array.push_back(info);
			}
		}

		for (auto &p : *clients) {
			send_message(p->client_socket, &p_removed);
			//receive_message(p->client_socket, &recv);
		}
	}

	if (m->is_winner(c->p)) {
		c->p->set_winner(true);

		msg_creation win;
		msg_parsing recv;
		
		win.msg_type = MSG_MATCH_WIN;
		win.data.match_win.player.player_id = c->p->get_id();
		append_info(c->p, &win.data.match_win.info);
		send_message(c->client_socket, &win);
		//receive_message(c->client_socket, &recv);

		msg_creation end;
		end.msg_type = MSG_MATCH_END;

		for (auto &p : *clients) {
			if (!p->p->is_winner()) {
				end.data.match_end.player.player_id = p->p->get_id();
				append_info(p->p, &end.data.match_end.info);
				
				send_message(p->client_socket, &end);
				//receive_message(p->client_socket, &recv);
			}
		}

		this->stop_serv = true;
		return;
	}

	if (m->all_attacked(c->p)) {
		msg_creation turn;
		turn.msg_type = MSG_MATCH_TURN;
		turn.data.match_turn.turn = false;
		msg_parsing recv;
		c->p->set_turn(false);
		send_message(c->client_socket, &turn);
		//receive_message(c->client_socket, &recv);

		size_t i;
		for (i = 0; i < clients->size(); i++) {
			if ((*clients)[i]->p->get_id() == c->p->get_id()) {
				if (i == clients->size() - 1) {
					i = 0;
				} else {
					i++;
				}
				break;
			}
		}
		(*clients)[i]->p->set_turn(true);
		turn.data.match_turn.turn = true;
		send_message((*clients)[i]->client_socket, &turn);
		//receive_message((*clients)[i]->client_socket, &recv);
	}
}

void Server::append_info(Player *p, struct stats_t *info) {
	info->duration = Match::get_duration(m->get_start_time(), p->get_end_time());
	info->grade = p->get_grade();
	info->hits = p->get_hits();
	info->missed = p->get_misses();
	info->remaining_ships = p->remaining_ships();
	info->sunk_ships = p->get_sunk_ships();
}

void Server::handle_player_quit(struct client_t *c) {
	if (m == NULL) {
		send_server_error(c);
		return;
	}

	msg_creation msg;
	msg.msg_type = ACK_MSG_MATCH_END;
	msg.data.ack_match_end.player.player_id = c->p->get_id();
	append_info(c->p, &msg.data.ack_match_end.info);
	send_message(c->client_socket, &msg);
}

void Server::handle_host_init_match(struct client_t *c, msg_parsing *msg) {
	if (m == NULL) {
		send_server_error(c);
		return;
	}

	msg_creation c_msg;
	if (c->p->is_host()) {
		m->set_difficulty((enum game_difficulty_e)msg->data.host_init_match.difficulty);
		for (int i = 0; i < msg->data.host_init_match.ais; i++) {
			Player *p = new Player(false);
			p->set_ai(true);
			p->set_diff(m->get_difficulty());
			p->set_name("ai_" + std::to_string(i+1));
			m->add_player(p);
		}

		c_msg.msg_type = ACK_MSG_MATCH_INIT_MATCH;
		c_msg.data.ack_match_init_match.player.player_id = c->p->get_id();
		c_msg.data.ack_match_init_match.status = GS_OK;
	} else {
		c_msg.msg_type = ACK_MSG_MATCH_NOT_HOST;
		c_msg.data.ack_match_not_host.player_id = c->p->get_id();
	}
	send_message(c->client_socket, &c_msg);
}

void Server::handle_host_start_match(struct client_t *c) {
	if (m == NULL) {
		send_server_error(c);
		return;
	}

	msg_creation c_msg;
	if (c->p->is_host()) {
		c_msg.msg_type = ACK;
		c_msg.data.ack.player_id = c->p->get_id();
		send_message(c->client_socket, &c_msg);

		msg_creation start;
		msg_parsing recv;
		start.msg_type = MSG_MATCH_STARTED;
		for (auto &c : *clients) {
			send_message(c->client_socket, &start);
			//receive_message(c->client_socket, &recv);
		}

		m->set_status(RUNNING);
		Logger::write("[server] Starting match");
		print_players();
	} else {
		c_msg.msg_type = ACK_MSG_MATCH_NOT_HOST;
		c_msg.data.ack_match_not_host.player_id = c->p->get_id();
		send_message(c->client_socket, &c_msg);
	}
}

void Server::handle_host_player_kick(struct client_t *c, msg_parsing *msg) {
	if (m == NULL) {
		send_server_error(c);
		return;
	}

	msg_creation c_msg;
	if (c->p->is_host()) {
		if (m->get_player_by_id(msg->data.host_player_kick.player.player_id) == NULL) {
			send_unknown_player(c);
			return;
		}

		c_msg.msg_type = ACK;
		c_msg.data.ack.player_id = c->p->get_id();
		send_message(c->client_socket, &c_msg);

		msg_creation kick;
		msg_parsing recv;
		kick.msg_type = MSG_MATCH_GOT_KICKED;
		kick.data.match_got_kicked.reason = msg->data.host_player_kick.message;
		struct client_t *send_to = get_client(msg->data.host_player_kick.player.player_id);
		send_message(send_to->client_socket, &kick);
		//receive_message(send_to->client_socket, &recv);

		msg_creation removed;
		msg_parsing recv2;
		removed.msg_type = MSG_MATCH_PLAYER_REMOVED;
		removed.data.match_player_removed.player.name = send_to->p->get_name();
		removed.data.match_player_removed.reason = "Kicked: " + msg->data.host_player_kick.message;

		m->remove_player(msg->data.host_player_kick.player.player_id);
		remove_client(send_to->client_socket);

		for (auto &p : *clients) {
			if (!p->p->is_dead()) {
				struct player_info info;
				info.name = p->p->get_name();
				info.player_id = p->p->get_id();
				removed.data.match_player_removed.list.array.push_back(info);
			}
		}

		for (auto &p : *clients) {
			send_message(p->client_socket, &removed);
			//receive_message(p->client_socket, &recv2);
		}
	} else {
		c_msg.msg_type = ACK_MSG_MATCH_NOT_HOST;
		c_msg.data.ack.player_id = c->p->get_id();
		send_message(c->client_socket, &c_msg);
	}
}

void Server::send_server_error(struct client_t *c) {
	msg_creation msg;
	msg.msg_type = ACK_MSG_SERVER_ERROR;
	msg.data.ack_server_error.player_id = c->p->get_id();
	send_message(c->client_socket, &msg);
}

void Server::send_unknown_player(struct client_t *c) {
	msg_creation msg;
	msg.msg_type = ACK_MSG_SERVER_UNKNOWN_ID;
	msg.data.ack_server_error.player_id = c->p->get_id();
	send_message(c->client_socket, &msg);
}

struct client_t *Server::get_client(int id) {
	for (auto &c : *clients) {
		if (c->p->get_id() == id) {
			return c;
		}
	}
	return NULL;
}

void Server::remove_client(int client_socket) {
	for (size_t i = 0; i < clients->size(); i++) {
		if ((*clients)[i]->client_socket == client_socket) {
			close((*clients)[i]->client_socket);
			if (m != NULL) {
				m->remove_player((*clients)[i]->p->get_id());
			}
			delete (*clients)[i]->p;
			delete (*clients)[i];
			clients->erase(clients->begin() + i);
			return;
		}
	}
}

void Server::print_players() {
	std::vector<Player*> *players = m->get_players();
	Logger::write("\n[server] Current player list: " + std::to_string(players->size()));
	for (auto &p : *players) {
		Logger::write(
			"[" + std::to_string(p->get_id()) + "]" + 
			(p->is_host() ? "[host]" : "") + 
			(p->is_ai() ? "[ai]" : "") + 
			p->get_name() + 
			": " + 
			(p->is_dead() ? "spectator" : "playing")
		);
	}
	Logger::write("\n");
}