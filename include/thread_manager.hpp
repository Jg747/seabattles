#ifndef __thread_manager_h__
#define __thread_manager_h__

#include <condition_variable>
#include <mutex>

#include <msg.hpp>
#include <server.hpp>
#include <client.hpp>

struct thread_manager_t {
    std::condition_variable locker;
    std::mutex mtx;
    std::unique_lock<std::mutex> lk{(mtx)};

    enum msg_type_e waiting_message_low;
    enum msg_type_e waiting_message_high;
};

void reset_waiting_msg(struct thread_manager_t *mng);
void wait_for(struct thread_manager_t *mng, enum msg_type_e type);
void wait_for(struct thread_manager_t *mng, enum msg_type_e low, enum msg_type_e high);

void lock(struct thread_manager_t *mng);
void unlock(struct thread_manager_t *mng);

class Server;
class Client;

void thread_server(Server *s);
void thread_receiver(Client *c);

#endif