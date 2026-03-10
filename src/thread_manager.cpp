#include <thread_manager.hpp>

#include <msg.hpp>
#include <debug.hpp>
#include <client.hpp>
#include <server.hpp>

void wait_for(struct thread_manager_t *mng, std::vector<enum msg_type_e> *list) {
    mng->waiting_list = *list;
    lock(mng);
}

void wait_for(struct thread_manager_t *mng, enum msg_type_e type) {
    mng->waiting_list.clear();
    mng->waiting_list.push_back(type);
    lock(mng);
}

bool is_waited(struct thread_manager_t *mng, enum msg_type_e type) {
    for (const auto t : mng->waiting_list) {
        if (t == type) {
            return true;
        }
    }
    return false;
}

void lock(struct thread_manager_t *mng) {
    mng->locker.wait(mng->lk);
}

void unlock(struct thread_manager_t *mng) {
    mng->locker.notify_one();
    mng->waiting_list.clear();
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