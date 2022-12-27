#include <thread_manager.hpp>

#include <msg.hpp>
#include <debug.hpp>
#include <client.hpp>
#include <server.hpp>

void reset_waiting_msg(struct thread_manager_t *mng) {
    mng->waiting_message_high = UNKNOWN;
    mng->waiting_message_low = UNKNOWN;
}

void wait_for(struct thread_manager_t *mng, enum msg_type_e type) {
    mng->waiting_message_low = type;
    mng->waiting_message_high = type;
    lock(mng);
}

void wait_for(struct thread_manager_t *mng, enum msg_type_e low, enum msg_type_e high) {
    mng->waiting_message_low = low;
    mng->waiting_message_high = high;
    lock(mng);
}

void lock(struct thread_manager_t *mng) {
    mng->locker.wait(mng->lk);
}

void unlock(struct thread_manager_t *mng) {
    mng->locker.notify_one();
    reset_waiting_msg(mng);
}

// Server thread, non toccare con quelle tue luride manine
void thread_server(Server *s) {
	Logger::write("[server] Starting server thread...");
	s->start();
}

void thread_receiver(Client *c) {
    while (!c->is_stop()) {
        c->do_from_socket();
    }
}