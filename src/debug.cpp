#include <debug.hpp>

#include <fstream>
#include <string>

std::string Logger::get_last_lines() {
    std::string str = "";
    for (auto s : Logger::last_lines) {
        str += s + "\n";
    }
    str.pop_back();
    str.shrink_to_fit();
    return str;
}

void Logger::write(std::string str) {
    if (!Logger::log_file.is_open()) {
        Logger::log_file.open(FILE_NAME);
    }
    Logger::log_file << str << "\n";
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