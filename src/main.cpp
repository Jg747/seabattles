#include <gui.h>
#include <debug.h>

int main(int argc, char *argv[]) {
	START_DEBUG()
	Gui g;
	g.start();
	STOP_DEBUG()
    return 0;
}