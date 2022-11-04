#include <multi_match.hpp>

MultiMatch::MultiMatch() {
	serv = NULL;
	player_eliminations = 0;
}

MultiMatch::~MultiMatch() {
	if (serv != NULL) {
		delete serv;
	}
}

bool MultiMatch::check_connection(string str) {
	return true;
}