#include <iostream>
#include <chrono>

#include <player.hpp>
#include <client.hpp>
#include <debug.hpp>

int main(int argc, char *argv[]) {
	bool debug = true;
	
	time_t seed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	srand(time(&seed));

	if (debug) {
		START_DEBUG();
	}
	
	try {
		Client c;
		Player::set_id_start(1);
		while(c.start());
	} catch (std::exception &e) {
		std::cout << e.what();
	}
	
	if (debug) {
		STOP_DEBUG();
	}
    
	return 0;
}