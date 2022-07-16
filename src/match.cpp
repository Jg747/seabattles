#include <match.hpp>
#include <common.hpp>
#include <ship.hpp>
#include <debug.hpp>

#include <iostream>
#include <time.h>
#include <stdlib.h>

using std::cout;
using std::cin;

Match::Match(enum gamemode mode) {
	this->mode = mode;
	this->last_id = 1;
	reset_board();
	for (int i = 0; i < SHIPS_COUNT; i++) {
		ships[i] = new Ship(i);
		enemy[i] = new Ship(i);
	}
	assign_ids();
	this->missed_shots = 0;
	this->hit_shots = 0;
}

Match::~Match() {
	for (int i = 0; i < SHIPS_COUNT; i++) {
		delete ships[i];
	}
}

void Match::reset_board() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			board[i][j] = 0;
			enemy_board[i][j] = 0;
		}
	}
}

void Match::assign_ids() {
	for (int i = 0; i < SHIPS_COUNT; i++) {
		ships[i]->set_id(last_id);
		enemy[i]->set_id(last_id);
		last_id += BOARD_SIZE + 1;
	}
}

bool Match::check_intersection(Ship *&ship, bool my_board) {
	if (my_board) {
		switch (ship->getRotation()) {
			case UP:
				for (int i = ship->getY(); i > ship->getY() - ship->getLen(); i--) {
					if (board[i][ship->getX()] > 0) {
						return false;
					}
				}
				break;
			case RIGHT:
				for (int i = ship->getX(); i < ship->getX() + ship->getLen(); i++) {
					if (board[ship->getY()][i] > 0) {
						return false;
					}
				}
				break;
			case DOWN:
				for (int i = ship->getY(); i < ship->getY() + ship->getLen(); i++) {
					if (board[i][ship->getX()] > 0) {
						return false;
					}
				}
				break;
			case LEFT:
				for (int i = ship->getX(); i > ship->getX() - ship->getLen(); i--) {
					if (board[ship->getY()][i] > 0) {
						return false;
					}
				}
				break;
		}
	} else {
		switch (ship->getRotation()) {
			case UP:
				for (int i = ship->getY(); i > ship->getY() - ship->getLen(); i--) {
					if (enemy_board[i][ship->getX()] > 0) {
						return false;
					}
				}
				break;
			case RIGHT:
				for (int i = ship->getX(); i < ship->getX() + ship->getLen(); i++) {
					if (enemy_board[ship->getY()][i] > 0) {
						return false;
					}
				}
				break;
			case DOWN:
				for (int i = ship->getY(); i < ship->getY() + ship->getLen(); i++) {
					if (enemy_board[i][ship->getX()] > 0) {
						return false;
					}
				}
				break;
			case LEFT:
				for (int i = ship->getX(); i > ship->getX() - ship->getLen(); i--) {
					if (enemy_board[ship->getY()][i] > 0) {
						return false;
					}
				}
				break;
		}
	}
	return true;
}

bool Match::insert_on_board(Ship *&ship, bool my_board) {
	int counter = 0;
	if (my_board) {
		switch (ship->getRotation()) {
			case UP:
				if (!check_intersection(ship, my_board)) {
					ship->set_placed(false);
					return false;
				}
				for (int i = ship->getY(); i > ship->getY() - ship->getLen(); i--, counter++) {
					board[i][ship->getX()] = ship->get_id() + counter;
				}
				break;
			case RIGHT:
				if (!check_intersection(ship, my_board)) {
					ship->set_placed(false);
					return false;
				}
				for (int i = ship->getX(); i < ship->getX() + ship->getLen(); i++, counter++) {
					board[ship->getY()][i] = ship->get_id() + counter;
				}
				break;
			case DOWN:
				if (!check_intersection(ship, my_board)) {
					ship->set_placed(false);
					return false;
				}
				for (int i = ship->getY(); i < ship->getY() + ship->getLen(); i++, counter++) {
					board[i][ship->getX()] = ship->get_id() + counter;
				}
				break;
			case LEFT:
				if (!check_intersection(ship, my_board)) {
					ship->set_placed(false);
					return false;
				}
				for (int i = ship->getX(); i > ship->getX() - ship->getLen(); i--, counter++) {
					board[ship->getY()][i] = ship->get_id() + counter;
				}
				break;
		}
		ships_remaining++;
	} else {
		switch (ship->getRotation()) {
			case UP:
				if (!check_intersection(ship, my_board)) {
					ship->set_placed(false);
					return false;
				}
				for (int i = ship->getY(); i > ship->getY() - ship->getLen(); i--, counter++) {
					enemy_board[i][ship->getX()] = ship->get_id() + counter;
				}
				break;
			case RIGHT:
				if (!check_intersection(ship, my_board)) {
					ship->set_placed(false);
					return false;
				}
				for (int i = ship->getX(); i < ship->getX() + ship->getLen(); i++, counter++) {
					enemy_board[ship->getY()][i] = ship->get_id() + counter;
				}
				break;
			case DOWN:
				if (!check_intersection(ship, my_board)) {
					ship->set_placed(false);
					return false;
				}
				for (int i = ship->getY(); i < ship->getY() + ship->getLen(); i++, counter++) {
					enemy_board[i][ship->getX()] = ship->get_id() + counter;
				}
				break;
			case LEFT:
				if (!check_intersection(ship, my_board)) {
					ship->set_placed(false);
					return false;
				}
				for (int i = ship->getX(); i > ship->getX() - ship->getLen(); i--, counter++) {
					enemy_board[ship->getY()][i] = ship->get_id() + counter;
				}
				break;
		}
	}
	return true;
}

void Match::remove_from_board(Ship *&ship) {
	switch (ship->getRotation()) {
		case UP:
			for (int i = ship->getY(); i > ship->getY() - ship->getLen(); i--) {
				board[i][ship->getX()] = 0;
			}
			break;
		case RIGHT:
			for (int i = ship->getX(); i < ship->getY() + ship->getLen(); i++) {
				board[ship->getY()][i] = 0;
			}
			break;
		case DOWN:
			for (int i = ship->getY(); i < ship->getY() + ship->getLen(); i++) {
				board[i][ship->getX()] = 0;
			}
			break;
		case LEFT:
			for (int i = ship->getX(); i > ship->getY() - ship->getLen(); i--) {
				board[ship->getY()][i] = 0;
			}
			break;
	}
	ships_remaining--;
}

bool Match::insert_ship(int index, enum command_e cmd) {
	if (cmd == REMOVE) {
		ships[index]->set_placed(false);
		remove_from_board(ships[index]);
		return true;
	}
	
	if (cmd == PLACE) {
		ships[index]->set_placed(true);
		return insert_on_board(ships[index], true);
	}
	
	ships[index]->place_ship(cmd);
	return true;
}

int Match::remaining_ships() {
	return (int)this->ships_remaining;
}

void Match::generate_match() {
	srand(time(NULL));

	int rand_x;
	int rand_y;
	int rand_r;

	for (int i = 0; i < SHIPS_COUNT; ) {
		rand_x = (rand() % 10001) / 1000;
		rand_y = (rand() % 10001) / 1000;
		rand_r = (rand() % 4001) / 1000;
		if (Ship::evaluate_pos(rand_x, rand_y, enemy[i]->getLen(), (enum rotation_e)rand_r)) {
			enemy[i]->setX(rand_x);
			enemy[i]->setY(rand_y);
			enemy[i]->setRotation((enum rotation_e)rand_r);
			enemy[i]->set_placed(true);
			if (insert_on_board(enemy[i], false)) {
				i++;
			}
		}
	}
}

void Match::ai_attack() {
	
}