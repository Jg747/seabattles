#include <iostream>
#include <string>
#include <vector>

#include "msg.hpp"

using namespace std;

int main() {
	string str = create_message(MSG_PLAYER_GET_OWN_ID);
	cout << str;
	return 0;
}
