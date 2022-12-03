#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <string.h>

#include "pugi/pugixml.hpp"
#include "msg.hpp"

#define AS_INT(value) strtol(value, NULL, 10);

const char *MSG_TYPE_STR[] = {
	// Generici
	"GENERIC",
	"nak",

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

static int get_enum(std::string str, const char *arr[], size_t len) {
	for (int i = 0; i < len; i++) {
		if (!strcmp(str.c_str(), arr[i])) {
			return i;
		}
	}
	return -1;
}

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
	msg_type.append_child(pugi::node_pcdata).set_value(type == ACK || (type >= 14 && type <= 22) || (type >= 2 && type <= 5) ? "ack" : MSG_TYPE_STR[type]);
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

static pugi::xml_node get_data_node(pugi::xml_document &doc) {
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

static void append_list(pugi::xml_node &data, struct msg_player_list *player_list) {
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

static void create_ack(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack.player_id);
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

static void create_player_get_own_id(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "username", msg->data.player_get_own_id.username);
}

static void create_player_ship_placement(pugi::xml_document &doc, msg_creation *msg) {
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

static void create_player_get_board(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.player_get_board.id);
}

static void create_player_get_board_lost(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.player_get_board_lost.id);
}

static void create_player_attack(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.player_attack.player.player_id);
	add_node(data, "x", msg->data.player_attack.x);
	add_node(data, "y", msg->data.player_attack.y);
}

static void create_host_player_kick(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.host_player_kick.player.player_id);
	add_node(data, "msg", msg->data.host_player_kick.message);
}

static void create_ack_player_get_own_id(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	create_ack_node(data, ACK_MSG_PLAYER_GET_OWN_ID);
	add_node(data, "id", msg->data.ack_player_get_own_id.player_id);
}

static void create_ack_match_attack_status(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_attack_status.player.player_id);
	create_ack_node(data, ACK_MSG_MATCH_ATTACK_STATUS);
	add_node(data, "status", ATTACK_STATUS_STR[msg->data.ack_match_attack_status.status]);
}

static void create_ack_get_board(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_get_board.client.player_id);
	create_ack_node(data, ACK_MSG_GET_BOARD);
	add_node(data, "player", msg->data.ack_get_board.player.player_id);
	append_board(data, &msg->data.ack_get_board.board);
}

static void create_ack_get_board_lost(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_get_board_lost.client.player_id);
	create_ack_node(data, ACK_MSG_GET_BOARD);
	add_node(data, "player", msg->data.ack_get_board_lost.player.player_id);
	append_board(data, &msg->data.ack_get_board_lost.board);
}

static void create_ack_match_end(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_end.player.player_id);
	create_ack_node(data, ACK_MSG_MATCH_END);
	append_info(data, &msg->data.ack_match_end.info);
}

static void create_ack_invalid_ship_placement(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_invalid_ship_placement.player_id);
	create_ack_node(data, ACK_INVALID_SHIP_PLACEMENT);
}

static void create_ack_match_attack_err(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_attack_err.player.player_id);
	create_ack_node(data, ACK_MSG_MATCH_ATTACK_ERR);
	add_node(data, "status", (int)msg->data.ack_match_attack_err.status);
}

static void create_ack_match_not_host(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_not_host.player_id);
	create_ack_node(data, ACK_MSG_MATCH_NOT_HOST);
}

static void create_ack_match_not_dead(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.ack_match_not_dead.player_id);
	create_ack_node(data, ACK_MSG_MATCH_NOT_DEAD);
}

static void create_player_list(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	append_list(data, &msg->data.msg_player_list);
}

static void create_match_player_removed(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "who", msg->data.match_player_removed.player.name);
	add_node(data, "reason", msg->data.match_player_removed.reason);
	append_list(data, &msg->data.match_player_removed.list);
}

static void create_match_all_placed(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "turn", msg->data.match_all_placed.turn);
}

static void create_match_new_board(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.match_new_board.player.player_id);
	add_node(data, "attacker", msg->data.match_new_board.attacker.name);
	append_board(data, &msg->data.match_new_board.board);
}

static void create_match_win(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.match_win.player.player_id);
	append_info(data, &msg->data.match_win.info);
}

static void create_match_lose(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.match_lose.player.player_id);
	append_info(data, &msg->data.match_lose.info);
}

static void create_match_end(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "id", msg->data.match_end.player.player_id);
	append_info(data, &msg->data.match_end.info);
}

static void create_match_got_kicked(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	add_node(data, "reason", msg->data.match_got_kicked.reason);
}

std::string create_message(enum msg_type_e type, msg_creation *msg) {
	if (msg != NULL) {
		msg->msg_type = type;
	}
	pugi::xml_document doc = init_msg(type);
	
	switch (type) {
		case ACK:
			create_ack(doc, msg);
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

std::string create_message(enum msg_type_e type) {
	return create_message(type, NULL);
}

// ----------------------------------------------------
//                      MSG PARSING                     
// ----------------------------------------------------

static enum msg_type_e get_msg_type(pugi::xml_document &doc) {
	pugi::xml_node type = doc.child("message").child("type");
	std::string msg_type = type.value();
	if (msg_type != "ack") {
		return (enum msg_type_e)get_enum(msg_type, MSG_TYPE_STR, MSG_TYPE_STR_LEN);
	} else {
		pugi::xml_node ack_type = get_data_node(doc).child("acktype");
		msg_type = ack_type.value();
		return (enum msg_type_e)get_enum(msg_type, MSG_TYPE_STR, MSG_TYPE_STR_LEN);
	}
}

static void get_player_list(pugi::xml_node &data, struct msg_player_list *list) {
	pugi::xml_node player_list = data.child("playerlist");
	struct player_info info;
	for (pugi::xml_node player = player_list.first_child(); player; player = player_list.next_sibling()) {
		info.player_id = AS_INT(player.child("id").value());
		info.name = player.child("username").value();
		list->array.push_back(info);
	}
}

static void get_board(pugi::xml_node &data, struct board_t *board) {
	size_t i = 0, j;
	for (pugi::xml_node row = data.child("board").first_child(); row; row = data.child("board").next_sibling(), i++) {
		j = 0;
		for (pugi::xml_node value = row.first_child(); value; value = row.next_sibling(), j++) {
			board->matrix[i][j] = AS_INT(value.value());
		}
	}
}

static void get_info(pugi::xml_node &data, struct stats_t *info) {
	info->grade = (enum grade_e)AS_INT(data.child("grade").value());
	info->hits = AS_INT(data.child("hits").value());
	info->missed = AS_INT(data.child("misses").value());
	pugi::xml_node ships = data.child("ships");
	info->sunk_ships = AS_INT(ships.child("sunk").value());
	info->remaining_ships = AS_INT(ships.child("remaining").value());
}

static void parse_ack_generic(pugi::xml_document &doc, msg_creation *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack.player_id = AS_INT(data.child("id").value());
}

static void parse_player_get_own_id(pugi::xml_document &doc, msg_parsing *msg) {
	msg->data.player_get_own_id.username = get_data_node(doc).child("username").value();
}

static void parse_player_ship_placement(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	size_t i = 0;
	for (pugi::xml_node ship_node = data.first_child(); ship_node; ship_node = data.next_sibling(), i++) {
		struct ship_t *ship = &msg->data.player_ship_placement.array[i];
		ship->type = (enum ship_e)AS_INT(ship_node.child("type").value());
		ship->x = AS_INT(ship_node.child("x").value());
		ship->y = AS_INT(ship_node.child("y").value())
		ship->rotation = (enum rotation_e)AS_INT(ship_node.child("orientation").value());
	}
}

static void parse_player_get_board(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.player_get_board.id = AS_INT(data.child("id").value());
}

static void parse_player_get_board_lost(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.player_get_board_lost.id = AS_INT(data.child("id").value());
}

static void parse_player_attack(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.player_attack.player.player_id = AS_INT(data.child("id").value());
	msg->data.player_attack.x = AS_INT(data.child("x").value());
	msg->data.player_attack.y = AS_INT(data.child("y").value());
}

static void parse_host_player_kick(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.host_player_kick.player.player_id = AS_INT(data.child("id").value());
	msg->data.host_player_kick.message = data.child("msg").value();
}

static void parse_ack_player_get_own_id(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack_player_get_own_id.player_id = AS_INT(data.child("id").value());
}

static void parse_ack_match_attack_status(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack_match_attack_status.player.player_id = AS_INT(data.child("id").value());
	msg->data.ack_match_attack_status.status = (enum attack_status_e)AS_INT(doc.child("status").value());
}

static void parse_ack_get_board(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack_get_board.client.player_id = AS_INT(data.child("id").value());
	msg->data.ack_get_board.player.player_id = AS_INT(data.child("player").value());
	get_board(data, &msg->data.ack_get_board.board);
}

static void parse_ack_get_board_lost(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack_get_board_lost.client.player_id = AS_INT(data.child("id").value());
	msg->data.ack_get_board_lost.player.player_id = AS_INT(data.child("player").value());
	get_board(data, &msg->data.ack_get_board_lost.board);
}

static void parse_ack_match_end(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack_match_end.player.player_id = AS_INT(data.child("id").value());
	get_info(data, &msg->data.ack_match_end.info);
}

static void parse_ack_invalid_ship_placement(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack_invalid_ship_placement.player_id = AS_INT(data.child("id").value());
}

static void parse_ack_match_attack_err(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack_match_attack_err.player.player_id = AS_INT(data.child("id").value());
	msg->data.ack_match_attack_err.status = (enum attack_status_e)AS_INT(data.child("error").value());
}

static void parse_ack_match_not_host(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack_match_not_host.player_id = AS_INT(data.child("id").value());
}

static void parse_ack_match_not_dead(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.ack_match_not_dead.player_id = AS_INT(data.child("id").value());
}

static void parse_player_list(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	get_player_list(data, &msg->data.msg_player_list);
}

static void parse_match_player_removed(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.match_player_removed.player.name = data.child("who").value();
	msg->data.match_player_removed.reason = data.child("reason").value();
	get_player_list(data, &msg->data.match_player_removed.list);
}

static void parse_match_all_placed(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.match_all_placed.turn = (data.child("turn").value() == "true" ? true : false);
}

static void parse_match_new_board(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.match_new_board.player.player_id = AS_INT(data.child("id").value());
	msg->data.match_new_board.attacker.name = data.child("attacker").value();
	get_board(data, &msg->data.match_new_board.board);
}

static void parse_match_win(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.match_win.player.player_id = AS_INT(data.child("id").value());
	get_info(data, &msg->data.match_win.info);
}

static void parse_match_lose(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.match_lose.player.player_id = AS_INT(data.child("id").value());
	get_info(data, &msg->data.match_lose.info);
}

static void parse_match_end(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.match_end.player.player_id = AS_INT(data.child("id").value());
	get_info(data, &msg->data.match_end.info);
}

static void parse_match_got_kicked(pugi::xml_document &doc, msg_parsing *msg) {
	pugi::xml_node data = get_data_node(doc);
	msg->data.match_got_kicked.reason = data.child("reason").value();
}

void parse_message(std::string &xml_message, msg_parsing *msg) {
	pugi::xml_document doc;
	doc.load_string(xml_message.c_str());
	msg->msg_type = get_msg_type(doc);

	switch (msg->msg_type) {	
		case ACK:
			parse_ack_generic(doc, msg);
			break;
			
		case MSG_PLAYER_GET_OWN_ID:
			parse_player_get_own_id(doc, msg);
			break;
		case MSG_PLAYER_SHIP_PLACEMENT:
			parse_player_ship_placement(doc, msg);
			break;
		case MSG_PLAYER_GET_BOARD:
			parse_player_get_board(doc, msg);
			break;
		case MSG_PLAYER_GET_BOARD_LOST:
			parse_player_get_board_lost(doc, msg);
			break;
		case MSG_PLAYER_ATTACK:
			parse_player_attack(doc, msg);
			break;
		
		case MSG_HOST_PLAYER_KICK:
			parse_host_player_kick(doc, msg);
			break;

		case ACK_MSG_PLAYER_GET_OWN_ID:
			parse_ack_player_get_own_id(doc, msg);
			break;
		case ACK_MSG_MATCH_ATTACK_STATUS:
			parse_ack_match_attack_status(doc, msg);
			break;
		case ACK_MSG_GET_BOARD:
			parse_ack_get_board(doc, msg);
			break;
		case ACK_MSG_GET_BOARD_LOST:
			parse_ack_get_board_lost(doc, msg);
			break;
		case ACK_MSG_MATCH_END:
			parse_ack_match_end(doc, msg);
			break;
		case ACK_INVALID_SHIP_PLACEMENT:
			parse_ack_invalid_ship_placement(doc, msg);
			break;
		case ACK_MSG_MATCH_ATTACK_ERR:
			parse_ack_match_attack_err(doc, msg);
			break;
		case ACK_MSG_MATCH_NOT_HOST:
			parse_ack_match_not_host(doc, msg);
			break;
		case ACK_MSG_MATCH_NOT_DEAD:
			parse_ack_match_not_dead(doc, msg);
			break;
		
		case MSG_PLAYER_LIST:
			parse_player_list(doc, msg);
			break;
		case MSG_MATCH_PLAYER_REMOVED:
			parse_match_player_removed(doc, msg);
			break;
		case MSG_MATCH_ALL_PLACED:
			parse_match_all_placed(doc, msg);
			break;
		case MSG_MATCH_NEW_BOARD:
			parse_match_new_board(doc, msg);
			break;
		case MSG_MATCH_WIN:
			parse_match_win(doc, msg);
			break;
		case MSG_MATCH_LOSE:
			parse_match_lose(doc, msg);
			break;
		case MSG_MATCH_END:
			parse_match_end(doc, msg);
			break;
		case MSG_MATCH_GOT_KICKED:
			parse_match_got_kicked(doc, msg);
			break;
		
		default:
			break;
	}
}
