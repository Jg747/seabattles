#include <match.hpp>
#include <common.hpp>
#include <debug.hpp>

#include <iostream>
#include <chrono>

using std::to_string;
using std::string;

void Match::set_time(time_t &time) {
	time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

string Match::get_duration() {
	std::chrono::time_point<std::chrono::system_clock> s = std::chrono::system_clock::from_time_t(start_time);
	std::chrono::time_point<std::chrono::system_clock> e = std::chrono::system_clock::from_time_t(end_time);
	std::chrono::duration<double> difference = e - s;
	long seconds = difference.count();
	int min = seconds / 60;
	seconds = seconds % 60;
	return to_string(min) + " : " + ((int)(seconds / 10) == 0 ? "0" : "") + to_string(seconds);
}

enum game_difficulty_e Match::get_difficulty() {
	return this->difficulty;
}

void Match::set_difficulty(enum game_difficulty_e diff) {
	this->difficulty = diff;
}

void Match::reset_match() {
	this->status = PROGRESS;
}

void Match::set_mode(enum gamemode_e mode) {
	this->mode = mode;
}

enum gamemode_e Match::get_mode() {
	return this->mode;
}

enum game_status_e Match::get_status() {
	return this->status;
}

void Match::set_status(enum game_status_e new_status) {
	this->status = new_status;
}