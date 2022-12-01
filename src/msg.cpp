#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <string.h>

#include <pugi/pugixml.hpp>
#include <msg.hpp>

#include <player.hpp>
#include <board.hpp>
#include <ship.hpp>

const char *MSG_TYPE_STR[] = {
	// Generici
	"GENERIC",
	"NAK",

	// Connessione
	"MSG_CONN_ACCEPTED",
	"MSG_CONN_ERR",
	"MSG_CONN_SERVER_FULL",
	"MSG_CONN_MATCH_STARTED",

	// Player
	"MSG_PLAYER_GET_OWN_ID",
	"MSG_PLAYER_SHIP_PLACEMENT",
	"MSG_PLAYER_GET_BOARD",
	"MSG_PLAYER_GET_BOARD_LOST",
	"MSG_PLAYER_ATTACK",
	"MSG_PLAYER_QUIT",

	// Host
	"MSG_HOST_START_MATCH",
	"MSG_HOST_PLAYER_KICK",

	// ACKs
	"MSG_PLAYER_GET_OWN_ID",
	"MSG_MATCH_ATTACK_STATUS",
	"MSG_GET_BOARD",
	"MSG_GET_BOARD_LOST",
	"MSG_MATCH_END",
	"INVALID_SHIP_PLACEMENT",
	"MSG_MATCH_ATTACK_ERR",
	"MSG_MATCH_NOT_HOST",
	"MSG_MATCH_NOT_DEAD",

	// Match
	"MSG_PLAYER_LIST",
	"MSG_MATCH_PLAYER_REMOVED",
	"MSG_MATCH_STARTED",
	"MSG_MATCH_ALL_PLACED",
	"MSG_MATCH_NEW_BOARD",
	"MSG_MATCH_WIN",
	"MSG_MATCH_LOSE",
	"MSG_MATCH_END",
	"MSG_MATCH_GOT_KICKED"
};

const char *ATTACK_STATUS_STR[] = {
	"FAILED_ATTACK",
	"MISSED",
	"HIT",
	"HIT_SUNK",

	"NOT_YOUR_TURN",
	"NOT_SAME_PLAYER",
	"DEAD_CANNOT_ATTACK",
	"INVALID_ATTACK"
};

template<typename T>
static bool is_a_string(T value) {
	return 
		std::is_same<T, std::string>::value ||
		std::is_same<T, char*>::value ||
		std::is_same<T, const std::string>::value ||
		std::is_same<T, const char*>::value;
}

template<typename T>
static void add_value(pugi::xml_node &elem, T value) {
	std::string temp;
	if (!is_a_string(value)) {
		std::stringstream stream;
		stream << std::boolalpha << value;
		stream >> temp;
	} else {
		temp = value;
	}
	elem.append_child(pugi::node_pcdata).set_value(temp.c_str());
}

static pugi::xml_document init_msg(enum msg_type_e type) {
	pugi::xml_document doc;
	pugi::xml_node msg = doc.append_child("message");
	pugi::xml_node msg_type = msg.append_child("type");
	msg_type.append_child(pugi::node_pcdata).set_value(type != ACK ? MSG_TYPE_STR[type] : "ack");
	return doc;
}

static std::string close_msg(pugi::xml_document &doc) {
	std::string str;
	std::ostringstream stream;
	doc.save(stream);
	return stream.str();
}

template<typename T>
static void add_node(pugi::xml_node &data, std::string elem, T value) {
	pugi::xml_node element = data.append_child(elem.c_str());
	add_value(element, value);
}

template<typename T>
static void add_node(pugi::xml_node &data, std::string elem, T values[], int len) {
	pugi::xml_node element = data.append_child(elem.c_str());
	for (int i = 0; i < len; i++) {
		pugi::xml_node col = element.append_child("element");
		add_value(col, values[i]);
	}
}

template<typename T>
static void add_node(pugi::xml_node &data, std::string elem, T **values, int rows, int cols) {
	pugi::xml_node element = data.append_child(elem.c_str());
	for (int i = 0; i < rows; i++) {
		pugi::xml_node row = element.append_child("row");
		for (int j = 0; j < cols; j++) {
			pugi::xml_node col = row.append_child("element");
			add_value(col, values[i][j]);
		}
	}
}

template<typename T>
static void add_node(pugi::xml_node &data, std::string elem, std::vector<T> values) {
	pugi::xml_node element = data.append_child(elem.c_str());
	for (auto v : values) {
		pugi::xml_node col = element.append_child("element");
		add_value(col, v);
	}
}

static pugi::xml_node &get_data_node(pugi::xml_document &doc) {
	pugi::xml_node msg = doc.child("message");
	pugi::xml_node data;
	if (!(data = msg.child("data"))) {
		data = msg.append_child("data");
	}
	return data;
}

// ----------------------------------------------------
//                     MSG CREATION                    
// ----------------------------------------------------

static void append_board(pugi::xml_node &data, struct board_t *board) {
	pugi::xml_node board_node = data.append_child("board");
	for (int i = 0; i < BOARD_SIZE; i++) {
		pugi::xml_node row = board_node.append_child("row");
		for (int j = 0; j < BOARD_SIZE; j++) {
			add_node(row, "color", board->matrix[i][j]);
		}
	}
}

static void append_list(pugi::xml_node &data, struct c_msg_player_list *player_list) {
	pugi::xml_node list = data.append_child("playerlist");
	for (auto p : player_list->array) {
		pugi::xml_node player = list.append_child("player");
		add_node(player, "id", p.player_id);
		add_node(player, "username", p.name);
	}
}

static void append_info(pugi::xml_node &data, struct stats_t *info) {
	add_node(data, "grade", (int)info->grade);
	add_node(data, "hits", info->hits);
	add_node(data, "misses", info->missed);
	pugi::xml_node ships = data.append_child("ships");
	add_node(ships, "sunk", info->sunk_ships);
	add_node(ships, "remaining", info->remaining_ships);
}

static void create_ack_node(pugi::xml_node &data, enum msg_type_e type) {
	add_node(data, "acktype", MSG_TYPE_STR[type]);
}

static void create_ack(pugi::xml_document &doc) {
	pugi::xml_node data = get_data_node(doc);
	create_ack_node(data, ACK);
}

static void create_conn_accepted(pugi::xml_document &doc) {
	pugi::xml_node data = get_data_node(doc);
	create_ack_node(data, MSG_CONN_ACCEPTED);
}

static void create_conn_err(pugi::xml_document &doc) {
	pugi::xml_node data = get_data_node(doc);
	create_ack_node(data, MSG_CONN_ERR);
}

static void create_conn_server_full(pugi::xml_document &doc) {
	pugi::xml_node data = get_data_node(doc);
	create_ack_node(data, MSG_CONN_SERVER_FULL);
}

static void create_conn_match_started(pugi::xml_document &doc) {
	pugi::xml_node data = get_data_node(doc);
	create_ack_node(data, MSG_CONN_MATCH_STARTED);
}

static void create_player_get_own_id(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "username", msg->data.player_get_own_id.username);
}

static void create_player_ship_placement(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);

	for (int i = 0; i < SHIPS_COUNT; i++) {
		pugi::xml_node node = data.append_child("ship");
		struct ship_t *temp = &msg->data.player_ship_placement.array[i];
		add_node(node, "type", (int)temp->type);
		add_node(node, "x", temp->x);
		add_node(node, "y", temp->y);
		add_node(node, "orientation", (int)temp->rotation);
	}
}

static void create_player_get_board(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.player_get_board.id);
}

static void create_player_get_board_lost(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.player_get_board_lost.id);
}

static void create_player_attack(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.player_attack.player.player_id);
	add_node(data, "x", msg->data.player_attack.x);
	add_node(data, "y", msg->data.player_attack.y);
}

static void create_host_player_kick(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.host_player_kick.player.player_id);
	add_node(data, "msg", msg->data.host_player_kick.message);
}

static void create_ack_player_get_own_id(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	create_ack_node(data, ACK_MSG_PLAYER_GET_OWN_ID);
	add_node(data, "id", msg->data.ack_player_get_own_id.player_id);
}

static void create_ack_match_attack_status(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_attack_status.player.player_id);
	create_ack_node(data, ACK_MSG_MATCH_ATTACK_STATUS);
	add_node(data, "status", ATTACK_STATUS_STR[msg->data.ack_match_attack_status.status]);
}

static void create_ack_get_board(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_get_board.client.player_id);
	create_ack_node(data, ACK_MSG_GET_BOARD);
	add_node(data, "player", msg->data.ack_get_board.player.player_id);
	append_board(data, &msg->data.ack_get_board.board);
}

static void create_ack_get_board_lost(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_get_board_lost.client.player_id);
	create_ack_node(data, ACK_MSG_GET_BOARD);
	add_node(data, "player", msg->data.ack_get_board_lost.player.player_id);
	append_board(data, &msg->data.ack_get_board_lost.board);
}

static void create_ack_match_end(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_end.player.player_id);
	create_ack_node(data, ACK_MSG_MATCH_END);
	append_info(data, &msg->data.ack_match_end.info);
}

static void create_ack_invalid_ship_placement(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_invalid_ship_placement.player_id);
	create_ack_node(data, ACK_INVALID_SHIP_PLACEMENT);
}

static void create_ack_match_attack_err(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_attack_err.player.player_id);
	create_ack_node(data, ACK_MSG_MATCH_ATTACK_ERR);
	add_node(data, "status", (int)msg->data.ack_match_attack_err.status);
}

static void create_ack_match_not_host(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_not_host.player_id);
	create_ack_node(data, ACK_MSG_MATCH_NOT_HOST);
}

static void create_ack_match_not_dead(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_not_dead.player_id);
	create_ack_node(data, ACK_MSG_MATCH_NOT_DEAD);
}

static void create_player_list(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	append_list(data, &msg->data.msg_player_list);
}

static void create_match_player_removed(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "who", msg->data.match_player_removed.player.name);
	add_node(data, "reason", msg->data.match_player_removed.reason);
	append_list(data, &msg->data.match_player_removed.list);
}

static void create_match_all_placed(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "turn", msg->data.match_all_placed.turn);
}

static void create_match_new_board(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.match_new_board.player.player_id);
	add_node(data, "attacker", msg->data.match_new_board.attacker.name);
	append_board(data, &msg->data.match_new_board.board);
}

static void create_match_win(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.match_win.player.player_id);
	append_info(data, &msg->data.match_win.info);
}

static void create_match_lose(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.match_lose.player.player_id);
	append_info(data, &msg->data.match_lose.info);
}

static void create_match_end(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.match_end.player.player_id);
	append_info(data, &msg->data.match_end.info);
}

static void create_match_got_kicked(pugi::xml_document &doc, struct msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "reason", msg->data.match_got_kicked.reason);
}

std::string create_message(enum msg_type_e type, struct msg_creation *msg, int ack_id) {
	pugi::xml_document doc = init_msg(type);

	switch (type) {
		case ACK:
			create_ack(doc);
			break;
		
		case MSG_CONN_ACCEPTED:
			create_conn_accepted(doc);
			break;
		case MSG_CONN_ERR:
			create_conn_err(doc);
			break;
		case MSG_CONN_SERVER_FULL:
			create_conn_server_full(doc);
			break;
		case MSG_CONN_MATCH_STARTED:
			create_conn_match_started(doc);
			break;
		
		case MSG_PLAYER_GET_OWN_ID:
			create_player_get_own_id(doc, msg);
			break;
		case MSG_PLAYER_SHIP_PLACEMENT:
			create_player_ship_placement(doc, msg);
			break;
		case MSG_PLAYER_GET_BOARD:
			create_player_get_board(doc, msg);
			break;
		case MSG_PLAYER_GET_BOARD_LOST:
			create_player_get_board_lost(doc, msg);
			break;
		case MSG_PLAYER_ATTACK:
			create_player_attack(doc, msg);
			break;
		
		case MSG_HOST_PLAYER_KICK:
			create_host_player_kick(doc, msg);
			break;

		case ACK_MSG_PLAYER_GET_OWN_ID:
			create_ack_player_get_own_id(doc, msg);
			break;
		case ACK_MSG_MATCH_ATTACK_STATUS:
			create_ack_match_attack_status(doc, msg);
			break;
		case ACK_MSG_GET_BOARD:
			create_ack_get_board(doc, msg);
			break;
		case ACK_MSG_GET_BOARD_LOST:
			create_ack_get_board_lost(doc, msg);
			break;
		case ACK_MSG_MATCH_END:
			create_ack_match_end(doc, msg);
			break;
		case ACK_INVALID_SHIP_PLACEMENT:
			create_ack_invalid_ship_placement(doc, msg);
			break;
		case ACK_MSG_MATCH_ATTACK_ERR:
			create_ack_match_attack_err(doc, msg);
			break;
		case ACK_MSG_MATCH_NOT_HOST:
			create_ack_match_not_host(doc, msg);
			break;
		case ACK_MSG_MATCH_NOT_DEAD:
			create_ack_match_not_dead(doc, msg);
			break;
		
		case MSG_PLAYER_LIST:
			create_player_list(doc, msg);
			break;
		case MSG_MATCH_PLAYER_REMOVED:
			create_match_player_removed(doc, msg);
			break;
		case MSG_MATCH_ALL_PLACED:
			create_match_all_placed(doc, msg);
			break;
		case MSG_MATCH_NEW_BOARD:
			create_match_new_board(doc, msg);
			break;
		case MSG_MATCH_WIN:
			create_match_win(doc, msg);
			break;
		case MSG_MATCH_LOSE:
			create_match_lose(doc, msg);
			break;
		case MSG_MATCH_END:
			create_match_end(doc, msg);
			break;
		case MSG_MATCH_GOT_KICKED:
			create_match_got_kicked(doc, msg);
			break;
		
		default:
			break;
	}

	return close_msg(doc);
}

std::string create_message(enum msg_type_e type, int ack_id) {
	return create_message(type, NULL, ack_id);
}

// ----------------------------------------------------
//                      MSG PARSING                     
// ----------------------------------------------------

void parse_message(pugi::xml_document &doc, enum msg_type_e type, struct msg_parsing *msg) {
	// ...
}

enum msg_type_e get_msg_type(std::string str) {
	// ...
}
