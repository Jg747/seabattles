#include <sig_handlers.hpp>

#include <csignal>

#include <debug.hpp>
#include <client.hpp>

extern Client *c;

void stop_program() {
    if (c != NULL) {
		delete c;
	}
	c = NULL;

	Logger::stop();
}

void init_signals() {
    //signal(SIGINT, int_handler);
    //signal(SIGTERM, int_handler);
    //signal(SIGKILL, int_handler);
    //signal(SIGSEGV, int_handler);

    //signal(SIGWINCH, resize_handler);

    //signal(SIGUSR1, stop_recv_handler);
}

void int_handler(int sig) {
    stop_program();
    exit(sig);
}

void resize_handler(int sig) {
    
}

void stop_recv_handler(int sig) {

}