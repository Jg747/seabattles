#include <debug.hpp>

#include <fstream>
#include <string>
#include <mutex>

std::string Logger::get_last_lines() {
    std::string str = "";
    for (auto s : Logger::last_lines) {
        str += s + "\n";
    }
    str.pop_back();
    str.shrink_to_fit();
    return str;
}

static std::mutex m;

void Logger::write(std::string str) {
    std::unique_lock<std::mutex> lock(m);

    if (!Logger::log_file.is_open()) {
        Logger::log_file.open(FILE_NAME, std::ios::app | std::ios::ate);
        //Logger::log_file.open(FILE_NAME);
    }
    Logger::log_file << str << "\n";
    
    Logger::log_file.close(); // DA RIMUOVERE!!!
    
    if (Logger::win != NULL) {
        Logger::last_lines.push_back(str);
        if (Logger::last_lines.size() > Logger::lines) {
            Logger::last_lines.pop_front();
        }
        str = Logger::get_last_lines();
        wclear(Logger::win);
        wprintw(Logger::win, "%s", str.c_str());
        wrefresh(Logger::win);
    }
}

void Logger::stop() {
    if (Logger::debug && Logger::log_file.is_open()) {
        Logger::log_file.close();
    }
}

void Logger::write(int **matrix, int rows, int cols) {
    std::string x = "";
    for (int i = 0; i < rows; i++) {
        x += "[ ";
        for (int j = 0; j < cols; j++) {
            x += std::to_string(matrix[i][j]) + "\t";
        }
        x.pop_back();
        x += " ]\n";
    }
    Logger::write(x);
}

void Logger::write(int matrix[][10], int rows, int cols) { // RIMUOVERE
    std::string x = "";
    for (int i = 0; i < rows; i++) {
        x += "[ ";
        for (int j = 0; j < cols; j++) {
            x += std::to_string(matrix[i][j]) + "\t";
        }
        x.pop_back();
        x += " ]\n";
    }
    Logger::write(x);
}