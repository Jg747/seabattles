#include <gui.hpp>
#include <debug.hpp>
#include <player.hpp>

#include <chrono>

int main(int argc, char *argv[]) {
	bool debug = true;
	
	time_t seed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	srand(time(&seed));

	if (debug) {
		START_DEBUG();
	}
	
	Gui g;
	Player::set_id(1);
	while (g.start());
	
	if (debug) {
		STOP_DEBUG();
	}
    
	return 0;
}