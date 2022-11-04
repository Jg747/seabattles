#include <single_match.hpp>
#include <common.hpp>
#include <ship.hpp>
#include <board.hpp>
#include <player.hpp>
#include <debug.hpp>

#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <chrono>

using std::cout;
using std::cin;
using std::to_string;

SingleMatch::SingleMatch() {
	this->set_difficulty(NORMAL);
	p = NULL;
	ai = NULL;
	reset_match();
}

SingleMatch::SingleMatch(enum game_difficulty_e diff) {
	this->set_difficulty(diff);
	p = NULL;
	ai = NULL;
	reset_match();
}

SingleMatch::~SingleMatch() {
	if (p != NULL) {
		delete p;
	}
	if (ai != NULL) {
		delete ai;
	}
}

void SingleMatch::reset_match() {
	this->Match::reset_match();
	if (p != NULL) {
		delete p;
	}
	if (ai != NULL) {
		delete ai;
	}
}

void SingleMatch::generate_match() {
	p = new Player(this, "YOUR");
	ai = new Player(this, "ENEMY");
	ai->set_ai(true);

	p->add_player_to_attack(*ai);
	ai->add_player_to_attack(*p);

	srand(time(NULL));

	int rand_x;
	int rand_y;
	int rand_r;

	Board *b = ai->get_board();
	Ship **ships = b->get_ships();

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

Player *SingleMatch::get_player() {
	return this->p;
}

Player *SingleMatch::get_ai() {
	return this->ai;
}