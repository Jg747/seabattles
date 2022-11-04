#ifndef __msg_h__
#define __msg_h__

#include <string>
#include <map>
#include <fstream>

extern const std::map<char, char> string_to_print;
extern const std::map<char, char> string_to_msg;

extern const char* MSG_TYPE_STR[];
#define MSG_TYPE_LEN 18

enum msg_type_e {
	// SCAMBIO DATI (Client)
	MSG_GET_OWN_ID_S, MSG_GET_OWN_ID_R,
	MSG_GET_PLAYER_LIST_S, MSG_GET_PLAYER_LIST_R,
	MSG_GET_PLAYER_INFO_S, MSG_GET_PLAYER_INFO_R,
	MSG_GET_PLAYER_BOARD_S, MSG_GET_PLAYER_BOARD_R,
	MSG_SHIPS_PLACEMENT,
	MSG_CAN_ATTACK_S, MSG_CAN_ATTACK_R,
	MSG_ATTACK_PLAYER_S, MSG_ATTACK_PLAYER_R,
	MSG_FORFEIT,

	// SCAMBIO DATI (Server)
	MSG_MATCH_START,
	MSG_MATCH_END_S, MSG_MATCH_END_R,
	MSG_PLAYER_WIN_S, MSG_PLAYER_WIN_R,
	MSG_PLAYER_LOSE_S, MSG_PLAYER_LOSE_R,

	// INFO
	MSG_INFO,
	MSG_CONN_ACCEPTED,

	// ERRORI
	MSG_CONN_ERR,
	MSG_CONN_SERVER_FULL,

	// RISPOSTE GENERICHE
	MSG_RSP_ACK,
	MSG_RSP_NAK,
};

std::string create_message(enum msg_type_e type);
std::string create_message(enum msg_type_e type_S, void *ptr);
void parse_message(std::string &str_S, void *ptr);
std::vector<std::string> tokenize_message(std::string &str);
std::string convert_msg_to_print(std::string msg);
void print_msg(std::string &msg_S, FILE *out);
void print_msg(std::string &msg_S, std::fstream &out);
void print_msg(std::string &msg_S, std::ofstream &out);

#endif