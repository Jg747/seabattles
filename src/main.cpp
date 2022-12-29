/*
Cose da sistemare alla fine:
- debug: sistemare il file
- server: scommentare la get_ip nel costruttore
*/

#include <iostream>
#include <mutex>
#include <chrono>
#include <csignal>

#include <debug.hpp>
#include <player.hpp>
#include <client.hpp>
#include <sig_handlers.hpp>

Client *c;

int main(int argc, char *argv[]) {
	Logger::debug = true;
	
	time_t seed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	srand(time(&seed));

	struct thread_manager_t mng;
	init_signals();
	c = NULL;

	try {
		c = new Client(&mng);
		Player::set_id_start(1);
		while(c->start());
		c->stop_server();
	} catch (std::exception &e) {
		std::cout << e.what();
	}

	stop_program();
    
	return 0;
}