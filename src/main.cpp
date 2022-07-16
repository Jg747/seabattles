#include <gui.hpp>
#include <debug.hpp>

int main(int argc, char *argv[]) {
	START_DEBUG()
	Gui g;
	while (g.start());
	STOP_DEBUG()
    return 0;
}