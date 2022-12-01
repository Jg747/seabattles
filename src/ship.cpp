#include <ship.hpp>
#include <common.hpp>

const int ships_len[] = {5, 4, 3, 3, 2};

#include <string>

using std::string;
using std::to_string;

Ship::Ship(int type) {
	this->type = (enum ship_e)type;
	this->isPlaced = false;
	this->rotation = RIGHT;
	this->pos_x = 0;
	this->pos_y = BOARD_SIZE - 1;
	this->length = ships_len[this->type];
	this->hits = 0;
}

Ship::Ship(enum ship_e type) {
	this->type = type;
	this->isPlaced = false;
	this->rotation = RIGHT;
	this->pos_x = 0;
	this->pos_y = BOARD_SIZE - 1;
	this->length = ships_len[type];
	this->hits = 0;
}

string Ship::info() {
	return "SHIP: " + to_string(this->type) + "\n"
			+ "isPlaced: " + to_string(isPlaced) + "\n"
			+ "len: " + to_string(length) + "\n"
			+ "x: " + to_string(pos_x) + "\n"
			+ "y: " + to_string(pos_y) + "\n"
			+ "rotation: " + to_string(rotation) + "\n";
}

int Ship::place_ship(enum command_e cmd) {
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
		default: break;
	}
	return 0;
}

int Ship::get_x() {
	return (int)this->pos_x;
}

int Ship::get_y() {
	return (int)this->pos_y;
}

bool Ship::is_placed() {
	return this->isPlaced;
}

int Ship::get_len() {
	return this->length;
}

enum ship_e Ship::get_type() {
	return this->type;
}

enum rotation_e Ship::get_rotation() {
	return this->rotation;
}

int Ship::get_id() {
	return (int)this->id;
}

void Ship::set_id(int id) {
	this->id = (char)id;
}

void Ship::set_placed(bool state) {
	this->isPlaced = state;
}

bool Ship::evaluate_pos(int x, int y, int len, enum rotation_e rotate) {
	bool evaluate = true;
	
	if (rotate == UP) {
		if (y - len < 0) {
			evaluate = false;
		}
	}

	if (rotate == RIGHT) {
		if (x + len >= BOARD_SIZE) {
			evaluate = false;
		}
	}

	if (rotate == DOWN) {
		if (y + len >= BOARD_SIZE) {
			evaluate = false;
		}
	}

	if (rotate == LEFT) {
		if (x - len < 0) {
			evaluate = false;
		}
	}

	return evaluate;
}

void Ship::set_x(int x) {
	this->pos_x = x;
}

void Ship::set_y(int y) {
	this->pos_y = y;
}

void Ship::set_rotation(enum rotation_e r) {
	this->rotation = r;
}

bool Ship::point_intersect(int x, int y) {
	switch (rotation) {
		case UP:
			return y >= this->pos_y - this->length && y <= this->pos_y && x == this->pos_x;
			break;
		case RIGHT:
			return x <= this->pos_x + this->length && x >= this->pos_x && y == this->pos_y;
			break;
		case DOWN:
			return y <= this->pos_y + this->length && y >= this->pos_y && x == this->pos_x;
			break;
		case LEFT:
			return x >= this->pos_x - this->length && x <= this->pos_x && y == this->pos_y;
			break;
	}
	return false;
}

bool Ship::is_sunk() {
	return this->hits == this->length;
}

void Ship::add_hit() {
	this->hits++;
}

void Ship::reset_hits() {
	this->hits = 0;
}

int Ship::taken_hits() {
	return this->hits;
}

string Ship::pos_to_string() {
	switch (this->rotation) {
		case UP:
			return "up (" + to_string((int)this->rotation) + ")";
			break;
		case RIGHT:
			return "right (" + to_string((int)this->rotation) + ")";
			break;
		case DOWN:
			return "down (" + to_string((int)this->rotation) + ")";
			break;
		case LEFT:
			return "left (" + to_string((int)this->rotation) + ")";
			break;
		default:
			return "";
			break;
	}
}

string Ship::type_to_string() {
	switch (this->type) {
		case CARRIER:
			return "carrier (" + to_string((int)this->type) + ")";
			break;
		case BATTLESHIP:
			return "battleship (" + to_string((int)this->type) + ")";
			break;
		case DESTROYER:
			return "destroyer (" + to_string((int)this->type) + ")";
			break;
		case SUBMARINE:
			return "submarine (" + to_string((int)this->type) + ")";
			break;
		case PATROL:
			return "patrol (" + to_string((int)this->type) + ")";
			break;
		default:
			return "";
			break;
	}
}

string Ship::get_info() {
	string str = "Ship (id: " + to_string(this->get_id()) + ")\n";
	str += "type: " + this->type_to_string() + "\n";
	str += "len: " + to_string(this->getLen() + '0') + "\n";
	str += "rotation: " + this->pos_to_string() + "\n";
	str += "pos: " + to_string(this->getX()) + ", " + to_string(this->getY()) + "\n";
	str += "placed: " + string((this->is_placed() ? "true" : "false")) + "\n";
	str += "hits_taken: " + to_string(this->taken_hits()) + "\n";
	return str;
}