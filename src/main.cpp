#include <gui.hpp>
#include <debug.hpp>
#include <player.hpp>

int main(int argc, char *argv[]) {
	bool debug = false;

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