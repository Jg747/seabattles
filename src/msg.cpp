#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <string.h>

#include <msg.hpp>
#include <server.hpp>
#include <player.hpp>
#include <board.hpp>

using std::string;
using std::to_string;
using std::map;
using std::vector;

#define MSG_RESPONSE 0
#define MSG_REQUEST 1

const map<char, char> string_to_print {
	{'\x01', '\n'}, // LINE_START
	{'\x02', '('},  // OPEN_LEN
	{'\x03', ')'},  // CLOSE_LEN
	{'\x04', '{'},  // VALUE_START
	{'\x05', '}'},  // VALUE_STOP
	{'\x06', '['},  // VALUE_OPEN
	{'\x07', ']'},  // VALUE_CLOSE
	{'\x08', ','},  // VALUE_SEPARATOR
	{'\x09', '='},  // MSG TYPE
};

const map<char, char> string_to_msg {
	{'\n', '\x01'}, // LINE_START
	{'(', '\x02'},  // OPEN_LEN
	{')', '\x03'},  // CLOSE_LEN
	{'{', '\x04'},  // VALUE_START
	{'}', '\x05'},  // VALUE_STOP
	{'[', '\x06'},  // VALUE_OPEN
	{']', '\x07'},  // VALUE_CLOSE
	{',', '\x08'},  // VALUE_SEPARATOR
	{'=', '\x09'}   // MSG TYPE
};

const char* MSG_TYPE_STR[] = {
	"MSG_GET_OWN_ID", "MSG_GET_OWN_ID",
	"MSG_GET_PLAYER_LIST", "MSG_GET_PLAYER_LIST",
	"MSG_GET_PLAYER_INFO", "MSG_GET_PLAYER_INFO",
	"MSG_GET_PLAYER_BOARD", "MSG_GET_PLAYER_BOARD",
	"MSG_SHIPS_PLACEMENT", "MSG_SHIPS_PLACEMENT",
	"MSG_CAN_ATTACK", "MSG_CAN_ATTACK",
	"MSG_ATTACK_PLAYER", "MSG_ATTACK_PLAYER",
	"MSG_FORFEIT", "MSG_FORFEIT",
	"MSG_MATCH_START", "MSG_MATCH_START",
	"MSG_MATCH_END", "MSG_MATCH_END",
	"MSG_PLAYER_WIN", "MSG_PLAYER_WIN",
	"MSG_PLAYER_LOSE", "MSG_PLAYER_LOSE",
	"MSG_INFO", "MSG_INFO",
	"MSG_CONN_ACCEPTED", "MSG_CONN_ACCEPTED",
	"MSG_CONN_ERR", "MSG_CONN_ERR",
	"MSG_CONN_SERVER_FULL", "MSG_CONN_SERVER_FULL",
	"MSG_RSP_ACK", "MSG_RSP_ACK",
	"MSG_RSP_NAK", "MSG_RSP_NAK"
};

static int get_enum(char *str, char* arr[], int arr_len) {
	for (int i = 0; i < arr_len; i++) {
		if (strcmp(arr[i], str) == 0) {
			return i;
		}
	}
	return -1;
}

static void init_msg(string &str, enum msg_type_e type, int msg_type) {
	str = "{";
	str += string_to_msg.at('=');
	if (msg_type == MSG_REQUEST) {
		str += "s";
	} else {
		str += "r";
	}
	str += string(MSG_TYPE_STR[type]);
}

static void add_element(string &str, string elem, int len) {
	str += string_to_msg.at('\n') + elem + string_to_msg.at('(') + 
	std::to_string(len) + string_to_msg.at(')') + string_to_msg.at('{');
}

static void add_element(string &str, string elem, int rows, int cols) {
	str += string_to_msg.at('\n') + elem + string_to_msg.at('(') + 
	std::to_string(rows) + string_to_msg.at(',') + std::to_string(cols) 
	+ string_to_msg.at(')') + string_to_msg.at('{');
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
static void add_val(string &str, T value) {
	string temp;
	if (!is_a_string(value)) {
		std::stringstream stream;
		stream << value;
		stream >> temp;
		if (stream.fail()) {
			return;
		}
	} else {
		temp = value;
	}
	str += string_to_msg.at('[');
	str += temp;
	str += string_to_msg.at(']');
}

template<typename T>
static void add_item(string &str, string elem, T value) {
	add_element(str, elem, 1);
	add_val(str, value);
	str += string_to_msg.at('}');
}

template<typename T>
static void add_item(string &str, string elem, T values[], int len) {
	add_element(str, elem, len);
	for (int i = 0; i < len; i++) {
		add_val(str, values[i]);
		str += string_to_msg.at(',');
	}
	str.pop_back();
	str += string_to_msg.at('}');
}

template<typename T>
static void add_item(string &str, string elem, T **values, int rows, int cols) {
	add_element(str, elem, rows, cols);
	for (int i = 0; i < rows; i++) {
		str += string_to_msg.at('[');
		for (int j = 0; j < cols; j++) {
			add_val(str, values[i][j]);
			str += string_to_msg.at(',');
		}
		str.pop_back();
		str += string_to_msg.at(']');
		str += string_to_msg.at(',');
	}
	str.pop_back();
	str += string_to_msg.at('}');
}

template<typename T>
static void add_item(string &str, string elem, vector<T> values) {
	add_element(str, elem, values.size());
	for (int i = 0; i < values.size(); i++) {
		add_val(str, values[i]);
		str += string_to_msg.at(',');
	}
	str.pop_back();
	str += string_to_msg.at('}');
}

static void close_msg(string &str) {
	str += "}";
}

vector<string> tokenize_message(string &str) {
	std::stringstream stream(str);
	std::istream_iterator<string> begin(stream);
	std::istream_iterator<string> end;
	vector<string> tokens(begin, end);
	string temp = "";
	temp += string_to_msg.at('\n');
	std::copy(tokens.begin(), tokens.end(), std::ostream_iterator<string>(std::cout, temp.c_str()));
	return tokens;
}

string convert_msg_to_print(string msg) {
	size_t pos;
	string temp = msg;

	while ((pos = temp.find(string_to_msg.at('\n'))) != string::npos) {
		temp[pos] = string_to_print.at(string_to_msg.at('\n'));
	}
	while ((pos = temp.find(string_to_msg.at('('))) != string::npos) {
		temp[pos] = string_to_print.at(string_to_msg.at('('));
	}
	while ((pos = temp.find(string_to_msg.at(')'))) != string::npos) {
		temp[pos] = string_to_print.at(string_to_msg.at(')'));
	}
	while ((pos = temp.find(string_to_msg.at('{'))) != string::npos) {
		temp[pos] = string_to_print.at(string_to_msg.at('{'));
	}
	while ((pos = temp.find(string_to_msg.at('}'))) != string::npos) {
		temp[pos] = string_to_print.at(string_to_msg.at('}'));
	}
	while ((pos = temp.find(string_to_msg.at('['))) != string::npos) {
		temp[pos] = string_to_print.at(string_to_msg.at('['));
	}
	while ((pos = temp.find(string_to_msg.at(']'))) != string::npos) {
		temp[pos] = string_to_print.at(string_to_msg.at(']'));
	}
	while ((pos = temp.find(string_to_msg.at(','))) != string::npos) {
		temp[pos] = string_to_print.at(string_to_msg.at(','));
	}
	while ((pos = temp.find(string_to_msg.at('='))) != string::npos) {
		temp[pos] = string_to_print.at(string_to_msg.at('='));
	}

	return temp;
}

void print_msg(string &msg, FILE *out) {
	string print = convert_msg_to_print(msg);
	if (out == NULL) {
		fprintf(stdout, "%s", print.c_str());
	} else {
		fprintf(out, "%s", print.c_str());
	}
}

void print_msg(string &msg, std::fstream &out) {
	string print = convert_msg_to_print(msg);
	out << msg << "\n";
}

void print_msg(string &msg, std::ofstream &out) {
	string print = convert_msg_to_print(msg);
	out << msg << "\n";
}

// ----------------------------------------------------
//                     MSG CREATION                    
// ----------------------------------------------------

static void create_get_own_id_req(string &str) {
	init_msg(str, MSG_GET_OWN_ID_S, MSG_REQUEST);
	close_msg(str);
}

static void create_get_own_id_rsp(string &str, Player *p) {
	init_msg(str, MSG_GET_OWN_ID_R, MSG_RESPONSE);
	add_item(str, "id", p->get_id());
	close_msg(str);
}

static void create_get_player_list_req(string &str) {
	init_msg(str, MSG_GET_PLAYER_LIST_S, MSG_REQUEST);
	close_msg(str);
}

static void create_get_player_list_rsp(string &str, std::vector<struct client*> *vec) {
	init_msg(str, MSG_GET_PLAYER_LIST_R, MSG_RESPONSE);
	
	string **arr;
	arr = new string*[vec->size()];
	for (int i = 0; i < vec->size(); i++) {
		arr[i] = new string[2];
		arr[i][0] = vec->at(i)->p->get_name();
		arr[i][1] = to_string(vec->at(i)->p->get_id());
	}
	add_item(str, "players", arr, vec->size(), 2);
	for (int i = 0; i < vec->size(); i++) {
		delete [] arr[i];
	}
	delete [] arr;

	close_msg(str);
}

static void create_get_player_info_req(string &str, int *id) {
	init_msg(str, MSG_GET_PLAYER_INFO_S, MSG_REQUEST);
	add_item(str, "id", *id);
	close_msg(str);
}

static void create_get_player_info_rsp(string &str, Player *p) {
	init_msg(str, MSG_GET_PLAYER_INFO_R, MSG_RESPONSE);
	add_item(str, "id", p->get_id());
	add_item(str, "name", p->get_name());
	add_item(str, "is_ai", p->is_ai());
	add_item(str, "winner", p->is_winner());
	add_item(str, "loser", p->is_loser());
	add_item(str, "grade", (char)p->get_grade());
	add_item(str, "hits", p->get_hits());
	add_item(str, "misses", p->get_misses());
	add_item(str, "sunk ships", p->get_sunk_ships());
	add_item(str, "remaining ships", p->remaining_ships());
	close_msg(str);
}

static void create_get_player_board_req(string &str, int *id) {
	init_msg(str, MSG_GET_PLAYER_BOARD_S, MSG_REQUEST);
	add_item(str, "id", *id);
	close_msg(str);
}

static void create_get_player_board_rsp(string &str, Player *p) {
	init_msg(str, MSG_GET_PLAYER_BOARD_R, MSG_RESPONSE);
	add_item(str, "id", p->get_id());
	add_item(str, "board", p->get_board()->get_board(), BOARD_SIZE, BOARD_SIZE);
	close_msg(str);
}

static void create_can_attack_req(string &str, int *id) {
	init_msg(str, MSG_CAN_ATTACK_S, MSG_REQUEST);
	add_item(str, "id", *id);
	close_msg(str);
}

static void create_can_attack_rsp(string &str, Player *p) {
	init_msg(str, MSG_CAN_ATTACK_R, MSG_RESPONSE);
	add_item(str, "id", p->get_id());
	add_item(str, "can_attack", p->his_turn());
	close_msg(str);
}

string create_message(enum msg_type_e type, void *ptr) {
	string str;
	switch (type) {
		case MSG_GET_OWN_ID_S:
			create_get_own_id_req(str);
			break;
		case MSG_GET_OWN_ID_R:
			create_get_own_id_rsp(str, (Player*)ptr);
			break;

		case MSG_GET_PLAYER_LIST_S:
			create_get_player_list_req(str);
			break;
		case MSG_GET_PLAYER_LIST_R:
			create_get_player_list_rsp(str, (std::vector<struct client*>*)ptr);
			break;

		case MSG_GET_PLAYER_INFO_S:
			create_get_player_info_req(str, (int*)ptr);
			break;
		case MSG_GET_PLAYER_INFO_R:
			create_get_player_info_rsp(str, (Player*)ptr);
			break;

		case MSG_GET_PLAYER_BOARD_S:
			create_get_player_board_req(str, (int*)ptr);
			break;
		case MSG_GET_PLAYER_BOARD_R:
			create_get_player_board_rsp(str, (Player*)ptr);
			break;

		case MSG_CAN_ATTACK_S:
			create_can_attack_req(str, (int*)ptr);
			break;
		case MSG_CAN_ATTACK_R:
			create_can_attack_rsp(str, (Player*)ptr);
			break;

		case MSG_ATTACK_PLAYER_S:
			break;
		case MSG_ATTACK_PLAYER_R:
			break;

		case MSG_MATCH_END_S:
			break;
		case MSG_MATCH_END_R:
			break;

		case MSG_PLAYER_WIN_S:
			break;
		case MSG_PLAYER_WIN_R:
			break;

		case MSG_PLAYER_LOSE_S:
			break;
		case MSG_PLAYER_LOSE_R:
			break;

		case MSG_SHIPS_PLACEMENT:
			break;
		case MSG_FORFEIT:
			break;
		case MSG_MATCH_START:
			break;
		case MSG_INFO:
			break;
		case MSG_CONN_ACCEPTED:
			break;
		case MSG_CONN_ERR:
			break;
		case MSG_CONN_SERVER_FULL:
			break;
		case MSG_RSP_ACK:
			break;
		case MSG_RSP_NAK:
			break;
	}
	return str;
}

string create_message(enum msg_type_e type) {
	return create_message(type, NULL);
}

// ----------------------------------------------------
//                      MSG PARSING                     
// ----------------------------------------------------

void parse_message(string &str, void *ptr) {
	// ...
}