#include <ship.h>
#include <common.h>

const int ships_len[] = {5, 4, 3, 3, 2};

#include <iostream>
#include <string>

using std::cout;
using std::cin;
using std::string;
using std::to_string;

Ship::Ship(int type) {
	this->type = (enum ship_e)type;
	this->isPlaced = false;
	this->rotation = RIGHT;
	this->pos_x = 0;
	this->pos_y = BOARD_SIZE - 1;
	this->length = ships_len[this->type];
}

Ship::Ship(enum ship_e type) {
	this->type = type;
	this->isPlaced = false;
	this->rotation = RIGHT;
	this->pos_x = 0;
	this->pos_y = BOARD_SIZE - 1;
	this->length = ships_len[type];
}

string Ship::info() {
	return "NAVE: " + to_string(this->type) + "\n"
			+ "isPlaced: " + to_string(isPlaced) + "\n"
			+ "len: " + to_string(length) + "\n"
			+ "x: " + to_string(pos_x) + "\n"
			+ "y: " + to_string(pos_y) + "\n"
			+ "rotation: " + to_string(rotation) + "\n";
}

int Ship::place_ship(enum command_e cmd) {
	this->isPlaced = false;
	while (!this->isPlaced) {
		switch (cmd) {
			case MOVE_UP:
				if (pos_y-1 >= 0) {
					if (this->rotation == UP) {
						if (this->pos_y - this->length >= 0) {
							this->pos_y--;
						}
					} else {
						this->pos_y--;
					}
				}
				break;
			case MOVE_RIGHT:
				if (pos_x+1 < BOARD_SIZE) {
					if (this->rotation == RIGHT) {
						if (this->pos_x + this->length < BOARD_SIZE) {
							this->pos_x++;
						}
					} else {
						this->pos_x++;
					}
				}
				break;
			case MOVE_DOWN:
				if (pos_y+1 < BOARD_SIZE) {
					if (this->rotation == DOWN) {
						if (this->pos_y + this->length < BOARD_SIZE) {
							this->pos_y++;
						}
					} else {
						this->pos_y++;;
					}
				}
				break;
			case MOVE_LEFT:
				if (pos_x-1 >= 0) {
					if (this->rotation == LEFT) {
						if (this->pos_x - this->length >= 0) {
							this->pos_x--;
						}
					} else {
						this->pos_x--;
					}
				}
				break;
			case ROTATE:
				switch (this->rotation) {
					case UP:
						this->pos_x -= (this->length - 1);
						if (pos_x < 0) {
							pos_x = 0;
						}
						this->rotation = RIGHT;
						break;
					case RIGHT:
						this->pos_y -= (this->length - 1);
						if (pos_y < 0) {
							pos_y = 0;
						}
						this->rotation = DOWN;
						break;
					case DOWN:
						this->pos_x += (this->length - 1);
						if (pos_x >= BOARD_SIZE) {
							pos_x = BOARD_SIZE - 1;
						}
						this->rotation = LEFT;
						break;
					case LEFT:
						this->pos_y += (this->length - 1);
						if (pos_y >= BOARD_SIZE) {
							pos_y = BOARD_SIZE - 1;
						}
						this->rotation = UP;
						break;
				}
				break;
			case PLACE:
				this->isPlaced = true;
				break;
		}
	}
	return 0;
}

int Ship::getX() {
	return (int)this->pos_x;
}

int Ship::getY() {
	return (int)this->pos_y;
}

bool Ship::is_placed() {
	return this->isPlaced;
}

int Ship::getLen() {
	return this->length;
}

enum rotation_e Ship::getRotation() {
	return this->rotation;
}

int Ship::get_id() {
	return (int)this->id;
}

void Ship::set_id(int id) {
	this->id = (char)id;
}