#ifndef __sig_handlers_h__
#define __sig_handlers_h__

#include <client.hpp>

void init_signals();

void int_handler(int sig);
void resize_handler(int sig);
void stop_recv_handler(int sig);

void stop_program();

#endif