#include <iostream>
#include <string>
#include <vector>
#include <string.h>

#include "msg.hpp"

using namespace std;

static int get_enum(std::string str, const char *arr[], size_t len) {
	for (int i = 0; i < len; i++) {
		if (!strcmp(str.c_str(), arr[i])) {
			return i;
		}
	}
	return -1;
}

int main() {
	msg_creation c;
	msg_parsing p;

	c.data.player_get_own_id.username = "Jg_747";

	string str = create_message(MSG_PLAYER_GET_OWN_ID, &c);
	cout << "msg:\n" << str << "\n\n\n";
	parse_message(str, &p);

	cout << p.msg_type;
	cout << MSG_TYPE_STR[p.msg_type] << " | DATA:\n";
	cout << p.data.player_get_own_id.username;
	return 0;
}
