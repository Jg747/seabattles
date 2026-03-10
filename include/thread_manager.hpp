#ifndef __thread_manager_h__
#define __thread_manager_h__

#include <condition_variable>
#include <mutex>
#include <vector>

#include <msg.hpp>
#include <server.hpp>
#include <client.hpp>

struct thread_manager_t {
    std::condition_variable locker;
    std::mutex mtx;
    std::unique_lock<std::mutex> lk{(mtx)};

    std::vector<enum msg_type_e> waiting_list;
};

void wait_for(struct thread_manager_t *mng, std::vector<enum msg_type_e> *list);
void wait_for(struct thread_manager_t *mng, enum msg_type_e type);
bool is_waited(struct thread_manager_t *mng, enum msg_type_e type);

void lock(struct thread_manager_t *mng);
void unlock(struct thread_manager_t *mng);

class Server;
class Client;

void thread_server(Server *s);
void thread_receiver(Client *c);

#endif