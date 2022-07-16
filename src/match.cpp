#include <match.hpp>
#include <common.hpp>
#include <ship.hpp>

#include <iostream>

using std::cout;
using std::cin;

Match::Match(enum gamemode mode) {
	this->mode = mode;
	this->last_id = 1;
	reset_board();
	for (int i = 0; i < SHIPS_COUNT; i++) {
		ships[i] = new Ship(i);
	}
	assign_ids();
}

Match::~Match() {

}

void Match::reset_board() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			board[i][j] = 0;
		}
	}
}

void Match::assign_ids() {
	for (int i = 0; i < SHIPS_COUNT; i++) {
		ships[i]->set_id(last_id);
		last_id += BOARD_SIZE + 1;
	}
}

bool Match::check_intersection(Ship *&ship) {
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
	return true;
}

bool Match::insert_on_board(Ship *&ship) {
	int counter = 0;
	switch (ship->getRotation()) {
		case UP:
			if (!check_intersection(ship)) {
				return false;
			}
			for (int i = ship->getY(); i > ship->getY() - ship->getLen(); i--, counter++) {
				board[i][ship->getX()] = ship->get_id() + counter;
			}
			break;
		case RIGHT:
			if (!check_intersection(ship)) {
				return false;
			}
			for (int i = ship->getX(); i < ship->getX() + ship->getLen(); i++, counter++) {
				board[ship->getY()][i] = ship->get_id() + counter;
			}
			break;
		case DOWN:
			if (!check_intersection(ship)) {
				return false;
			}
			for (int i = ship->getY(); i < ship->getY() + ship->getLen(); i++, counter++) {
				board[i][ship->getX()] = ship->get_id() + counter;
			}
			break;
		case LEFT:
			if (!check_intersection(ship)) {
				return false;
			}
			for (int i = ship->getX(); i > ship->getX() - ship->getLen(); i--, counter++) {
				board[ship->getY()][i] = ship->get_id() + counter;
			}
			break;
	}
	ships_remaining++;
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
	if (ships[index]->is_placed()) {
		remove_from_board(ships[index]);
	}
	ships[index]->place_ship(cmd);
	return insert_on_board(ships[index]);
}