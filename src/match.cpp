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

Match::Match() {
	players = new std::vector<Player*>();
}

Match::Match(enum gamemode_e e) {
	players = new std::vector<Player*>();
	mode = e;
}

Match::~Match() {
	for (auto p : *players) {
		delete p;
	}
	delete players;
}

bool Match::add_player(Player *p) {
	if (players->size() < MAX_CLIENTS) {
		players->push_back(p);
		return true;
	}
	return false;
}

bool Match::remove_player(Player *p) {
	for (auto c : *players) {
		if (c == p) {
			delete c;
			return true;
		}
	}
	return false;
}

bool Match::remove_player(int id) {
	for (auto p : *players) {
		if (id == p->get_id()) {
			delete p;
			return true;
		}
	}
	return false;
}

void Match::start_match() {
	int rand_x;
	int rand_y;
	int rand_r;
	Board *b;
	Ship **ships;
	
	for (auto p : *players) {
		for (auto enemy : *players) {
			if (p != enemy) {
				p->add_player_to_attack(*enemy);
			}
		}

		if (p->is_ai()) {
			b = p->get_board();
			ships = b->get_ships();
			for (int i = 0; i < SHIPS_COUNT; ) {
				rand_x = (rand() % 10001) / 1000;
				rand_y = (rand() % 10001) / 1000;
				rand_r = (rand() % 4001) / 1000;
				if (Ship::evaluate_pos(rand_x, rand_y, ships[i]->getLen(), (enum rotation_e)rand_r)) {
					ships[i]->setX(rand_x);
					ships[i]->setY(rand_y);
					ships[i]->setRotation((enum rotation_e)rand_r);
					ships[i]->set_placed(true);
					if (b->insert_on_board(ships[i])) {
						i++;
					}
				}
			}
		}
	}

	Match::set_time(start_time);
}