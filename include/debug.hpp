#ifndef __debug_h__
#define __debug_h__ 

#include <fstream>
#include <string>
#include <deque>

#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif

#define FILE_NAME "debug.log"

class Logger {
    public:
        static inline bool debug = false;
        static inline WINDOW* win = NULL;
        static inline size_t lines = 0;
        
        static std::string get_last_lines();
        static void write(std::string str);
        static void write(int **matrix, int rows, int cols);
        static void write(int matrix[][10], int rows, int cols);//rimuovere
        static void stop();

    private:
        static inline std::ofstream log_file;
        static inline std::deque<std::string> last_lines;
};

#endif