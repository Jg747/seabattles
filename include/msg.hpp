#ifndef __msg_h__
#define __msg_h__

#include <string>
#include <vector>

#include <common.hpp>

extern const char *MSG_TYPE_STR[];
#define MSG_TYPE_STR_LEN 32

extern const char *ATTACK_STATUS_STR[];
#define ATTACK_STATUS_STR_LEN 8

enum msg_type_e {
	// Generico
	ACK,
	NAK,
	
	// Connessione
	MSG_CONN_ACCEPTED,
	MSG_CONN_ERR,
	MSG_CONN_SERVER_FULL,
	MSG_CONN_MATCH_STARTED,

	// Messaggi dei client
	MSG_PLAYER_GET_OWN_ID,
	MSG_PLAYER_SHIP_PLACEMENT,
	MSG_PLAYER_GET_BOARD,
	MSG_PLAYER_GET_BOARD_LOST,
	MSG_PLAYER_ATTACK,
	MSG_PLAYER_QUIT,

	// Messaggi dell'HOST
	MSG_HOST_START_MATCH,
	MSG_HOST_PLAYER_KICK,
	
	// Risposte del server
	ACK_MSG_PLAYER_GET_OWN_ID,
	ACK_MSG_MATCH_ATTACK_STATUS,
	ACK_MSG_GET_BOARD,
	ACK_MSG_GET_BOARD_LOST,
	ACK_MSG_MATCH_END,
	ACK_INVALID_SHIP_PLACEMENT,
	ACK_MSG_MATCH_ATTACK_ERR,
	ACK_MSG_MATCH_NOT_HOST,
	ACK_MSG_MATCH_NOT_DEAD,

	// Messaggi del server
	MSG_PLAYER_LIST,
	MSG_MATCH_PLAYER_REMOVED,
	MSG_MATCH_STARTED,
	MSG_MATCH_ALL_PLACED,
	MSG_MATCH_NEW_BOARD,
	MSG_MATCH_WIN,
	MSG_MATCH_LOSE,
	MSG_MATCH_END,
	MSG_MATCH_GOT_KICKED
};

struct ship_t {
	enum ship_e type;
	int x;
	int y;
	enum rotation_e rotation;
};

struct board_t {
	int matrix[BOARD_SIZE][BOARD_SIZE];
};

struct stats_t {
	enum grade_e grade;
	int hits;
	int missed;
	int sunk_ships;
	int remaining_ships;
};

struct match_stats_t {
	struct player_info player;
	struct stats_t info;
};

struct c_player_get_own_id {
	std::string username;
};

struct c_player_ship_placement {
	struct ship_t array[SHIPS_COUNT];
};

struct c_player_get_board {
	int id;
};

struct c_player_attack {
	struct player_info player;
	int x;
	int y;
};

struct c_host_player_kick {
	struct player_info player;
	std::string message;
};

typedef struct player_info c_ack_player_get_own_id;

struct c_ack_match_attack_status {
	struct player_info player;
	enum attack_status_e status;
};

struct c_ack_get_board {
	struct player_info client;
	struct player_info player;
	struct board_t board;
};

typedef struct match_stats_t c_ack_match_end;

typedef struct player_info c_ack_invalid_ship_placement;

struct c_ack_match_attack_err {
	struct player_info player;
	enum attack_status_e status;
};

typedef struct player_info c_ack_match_not_host;

typedef struct player_info c_ack_match_not_dead;

struct c_msg_player_list {
	std::vector<struct player_info> array;
};

struct c_match_player_removed {
	struct player_info player;
	std::string reason;
	struct c_msg_player_list list;
};

struct c_match_all_placed {
	bool turn;
};

struct c_match_new_board {
	struct player_info player;
	struct player_info attacker;
	struct board_t board;
};

typedef struct match_stats_t c_match_end;

typedef struct match_stats_t c_match_win;

typedef struct match_stats_t c_match_lose;

struct c_match_got_kicked {
	std::string reason;
};

struct msg_creation {
	union {
		struct c_player_get_own_id player_get_own_id;
		struct c_player_ship_placement player_ship_placement;
		struct c_player_get_board player_get_board;
		struct c_player_get_board player_get_board_lost;
		struct c_player_attack player_attack;
		
		struct c_host_player_kick host_player_kick;
		
		c_ack_player_get_own_id ack_player_get_own_id;
		struct c_ack_match_attack_status ack_match_attack_status;
		struct c_ack_get_board ack_get_board;
		struct c_ack_get_board ack_get_board_lost;
		c_ack_match_end ack_match_end;
		c_ack_invalid_ship_placement ack_invalid_ship_placement;
		struct c_ack_match_attack_err ack_match_attack_err;
		c_ack_match_not_host ack_match_not_host;
		c_ack_match_not_dead ack_match_not_dead;
	
		struct c_msg_player_list msg_player_list;
		struct c_match_player_removed match_player_removed;
		struct c_match_all_placed match_all_placed;
		struct c_match_new_board match_new_board;
		c_match_win match_win;
		c_match_lose match_lose;
		c_match_end match_end;
		struct c_match_got_kicked match_got_kicked;
	} data;
};

struct msg_parsing {
	union {
		
	} data;
};

std::string create_message(enum msg_type_e type, int ack_id);
std::string create_message(enum msg_type_e type, struct msg_creation *msg, int ack_id);
void parse_message(pugi::xml_document &doc, enum msg_type_e type, struct msg_parsing *msg);
enum msg_type_e get_msg_type(std::string &str);

#endif