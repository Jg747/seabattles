#include <iostream>
#include <chrono>

#include <debug.hpp>
#include <player.hpp>
#include <client.hpp>

int main(int argc, char *argv[]) {
	Logger::debug = true;
	
	time_t seed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	srand(time(&seed));

	try {
		Client c;
		Player::set_id_start(1);
		while(c.start());
		c.stop_server();
	} catch (std::exception &e) {
		std::cout << e.what();
	}

	Logger::stop();
    
	return 0;
}