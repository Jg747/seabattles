/*
Cose da sistemare alla fine:
- debug: sistemare il file
- server: scommentare la get_ip nel costruttore
- signal_handlers: fixare i vari signal handlers per chiudere decentemente il programma in caso di crash
- gui: gestire in caso di resize
- gui: per qualche ragione non ci sono i box() nelle celle sea[][] se height > 1
- gui: a game finito bisogna eliminare tutte le window e ricrearle [creare un clear_windows()]
*/

#include <iostream>
#include <chrono>

#include <debug.hpp>
#include <player.hpp>
#include <client.hpp>
#include <sig_handlers.hpp>

Client *c;

int main(int argc, char *argv[]) {
	Logger::debug = false;
	
	time_t seed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	srand(time(&seed));

	struct thread_manager_t mng;
	init_signals();
	c = NULL;

	try {
		c = new Client(&mng);
		while(c->start());
	} catch (std::exception &e) {
		std::cout << e.what();
	}

	stop_program();
    
	return 0;
}