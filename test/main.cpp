#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <time.h>
#include <stdlib.h>

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
	srand(time(NULL));

	msg_creation c;
	msg_parsing p;

	struct data_union *data = &c.data;
	struct data_union *pdata = &p.data;

	data->ack.player_id = -1;

	string str = create_message(NAK, &c);
	cout << "msg:\n" << str << "\n\n\n";
	parse_message(str, &p);

	cout << MSG_TYPE_STR[p.msg_type] << " | DATA:\n";

	cout << pdata->ack.player_id;

	cout << "\n";
	return 0;
}
