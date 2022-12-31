#include <board.hpp>
#include <ship.hpp>
#include <debug.hpp>

Board::Board() {
	this->last_id = 1;
	this->board = new int* [BOARD_SIZE];
	for (int i = 0; i < BOARD_SIZE; i++) {
		this->board[i] = new int [BOARD_SIZE];
	}
	this->ships = new Ship* [SHIPS_COUNT];
	for (int i = 0; i < SHIPS_COUNT; i++) {
		this->ships[i] = new Ship(i);
	}
	this->ships_remaining = 0;
	reset_board();
	assign_ids();
}

Board::~Board() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		delete[] this->board[i];
	}
	delete[] this->board;
	this->board = NULL;
	for (int i = 0; i < SHIPS_COUNT; i++) {
		delete this->ships[i];
	}
	delete[] this->ships;
	this->ships = NULL;
}

void Board::reset_board() {
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			board[i][j] = 0;
		}
	}
	this->last_id = 1;
	assign_ids();
	for (int i = 0; i < SHIPS_COUNT; i++) {
		ships[i]->set_x(0);
		ships[i]->set_y(BOARD_SIZE - 1);
		ships[i]->reset_hits();
		ships[i]->set_placed(false);
	}
	this->ships_remaining = 0;
}

void Board::assign_ids() {
	for (int i = 0; i < SHIPS_COUNT; i++) {
		ships[i]->set_id(last_id);
		last_id += BOARD_SIZE + 1;
	}
}

int Board::remaining_ships() {
	return (int)this->ships_remaining;
}

bool Board::is_hit(int x, int y) {
	return board[y][x] >= DAMAGE;
}

bool Board::insert_ship(int index, enum command_e cmd) {
	if (cmd == REMOVE) {
		ships[index]->set_placed(false);
		remove_from_board(ships[index]);
		return true;
	}
	
	if (cmd == PLACE) {
		ships[index]->set_placed(true);
		return insert_on_board(ships[index]);
	}
	
	ships[index]->place_ship(cmd);
	return true;
}

void Board::remove_from_board(Ship *&ship) {
	enum rotation_e rotation = ship->get_rotation();
	int x = ship->get_x();
	int y = ship->get_y();
	int len = ship->get_len();

	switch (rotation) {
		case UP:
			for (int i = y; i > y - len; i--) {
				board[i][x] = 0;
			}
			break;
		case RIGHT:
			for (int i = x; i < x + len; i++) {
				board[y][i] = 0;
			}
			break;
		case DOWN:
			for (int i = y; i < y + len; i++) {
				board[i][x] = 0;
			}
			break;
		case LEFT:
			for (int i = x; i > x - len; i--) {
				board[y][i] = 0;
			}
			break;
	}
	ships_remaining--;
}

bool Board::insert_on_board(Ship *&ship) {
	int counter = 0;
	switch (ship->get_rotation()) {
		case UP:
			if (!check_intersection(ship)) {
				ship->set_placed(false);
				return false;
			}
			for (int i = ship->get_x(); i > ship->get_y() - ship->get_len(); i--, counter++) {
				board[i][ship->get_x()] = ship->get_id() + counter;
			}
			break;
		case RIGHT:
			if (!check_intersection(ship)) {
				ship->set_placed(false);
				return false;
			}
			for (int i = ship->get_x(); i < ship->get_x() + ship->get_len(); i++, counter++) {
				board[ship->get_y()][i] = ship->get_id() + counter;
			}
			break;
		case DOWN:
			if (!check_intersection(ship)) {
				ship->set_placed(false);
				return false;
			}
			for (int i = ship->get_y(); i < ship->get_y() + ship->get_len(); i++, counter++) {
				board[i][ship->get_x()] = ship->get_id() + counter;
			}
			break;
		case LEFT:
			if (!check_intersection(ship)) {
				ship->set_placed(false);
				return false;
			}
			for (int i = ship->get_x(); i > ship->get_x() - ship->get_len(); i--, counter++) {
				board[ship->get_y()][i] = ship->get_id() + counter;
			}
			break;
	}
	ships_remaining++;
	return true;
}

bool Board::check_intersection(Ship *&ship) {
	switch (ship->get_rotation()) {
		case UP:
			for (int i = ship->get_y(); i > ship->get_y() - ship->get_len(); i--) {
				if (board[i][ship->get_x()] > 0) {
					return false;
				}
			}
			break;
		case RIGHT:
			for (int i = ship->get_x(); i < ship->get_x() + ship->get_len(); i++) {
				if (board[ship->get_y()][i] > 0) {
					return false;
				}
			}
			break;
		case DOWN:
			for (int i = ship->get_y(); i < ship->get_y() + ship->get_len(); i++) {
				if (board[i][ship->get_x()] > 0) {
					return false;
				}
			}
			break;
		case LEFT:
			for (int i = ship->get_x(); i > ship->get_x() - ship->get_len(); i--) {
				if (board[ship->get_y()][i] > 0) {
					return false;
				}
			}
			break;
	}
	return true;
}

int **Board::get_board() {
	return this->board;
}

Ship **Board::get_ships() {
	return this->ships;
}

string Board::get_info() {
	string str = "BOARD\n";
	for (int i = 0; i < BOARD_SIZE; i++) {
		str += "[\t";
		for (int j = 0; j < BOARD_SIZE; j++) {
			str += std::to_string(board[i][j]) + "\t";
		}
		str += "]\n";
	}
	str += "ships_remainig: " + std::to_string((int)ships_remaining) + "\n";
	str += "SHIPS\n";
	for (int i = 0; i < SHIPS_COUNT; i++) {
		str += ships[i]->get_info() + "\n";
	}
 	return str;
}