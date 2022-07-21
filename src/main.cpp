#include <gui.hpp>
#include <debug.hpp>

int main(int argc, char *argv[]) {
	bool debug = true;
	
	if (debug) {
		START_DEBUG();
	}
	
	Gui g;
	while (g.start());
	
	if (debug) {
		STOP_DEBUG();
	}
    
	return 0;
}