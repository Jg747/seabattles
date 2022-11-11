#include <iostream>
#include <string>
#include <vector>

#include <msg.hpp>
#include <player.hpp>
#include <board.hpp>
#include <ship.hpp>
#include <server.hpp>

using namespace std;

int main() {
	string str;
	fstream file("test.txt", ios::out);
	Player *p = new Player(NULL, "test");
	Player *p2 = new Player(NULL, "testone");
	vector<struct client*> clients;
	struct client *temp;
	temp = new struct client;
	temp->p = p;
	clients.push_back(temp);
	temp = new struct client;
	temp->p = p2;
	clients.push_back(temp);
	int id = 4;
	str = convert_msg_to_print(create_message(MSG_GET_PLAYER_BOARD_R, p2));
	print_msg(str, file);
	return 0;
}