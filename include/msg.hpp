#ifndef __msg_h__
#define __msg_h__

#include <string>

extern const char *MSG_TYPE_STR[];
#define MSG_TYPE_STR_LEN 10

enum msg_type_e {
	// SCAMBIO DATI
	MSG_PLAYER_GET_OWN_ID,
	MSG_PLAYER_LIST_REQUEST,
	MSG_PLAYER_LIST,

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
enum msg_type_e get_msg_type(std::string str);

#endif