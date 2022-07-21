#ifndef __DEBUG_H__
#define __DEBUG_H__ 

#include <stdbool.h>
#include <stdio.h>

extern FILE* debug_log_fd;
extern bool debug_mode;

#define DEBUG_SET(mode) (debug_mode = mode);
#define START_DEBUG() debug_log_fd = fopen("debug.log", "w"); debug_mode = true;
#define STOP_DEBUG() if (debug_mode) { fclose(debug_log_fd); debug_mode = false; }
#define DEBUG_WRITE_LOG(fmt, args...) if (debug_mode) { fprintf(debug_log_fd, fmt, ##args); }

#endif