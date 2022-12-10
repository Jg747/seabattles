#include <string>

#include <player.hpp>
#include <debug.hpp>
#include <common.hpp>
#include <board.hpp>
#include <match.hpp>

void Player::set_id_start(int start) {
	Player::id = start;
}

Player::Player(bool is_host) {
	b = new Board();
	missed_shots = 0;
	hit_shots = 0;
	loser = false;
	winner = false;
	turn = false;
	name.player_id = Player::id;
	Player::id++;
	ai = false;
	sunk_ships = 0;
	own_sunk_ships = 0;
	host = is_host;
	ask_board = true;
	dead = false;
	placed_ships = false;
}

Player::~Player() {
	delete b;
}

Board *Player::get_board() {
	return this->b;
}
		
void Player::reset_player() {
	b->reset_board();
	missed_shots = 0;
	hit_shots = 0;
}

enum grade_e Player::get_grade() {
	enum grade_e grade = S;
	int shots = hit_shots + missed_shots;
	int hit_perc;

	if (shots != 0) {
		hit_perc = hit_shots * 100 / shots;
	} else {
		hit_perc = 100;
	}

	if (hit_perc >= 90) {
		grade = S;
	} else if (hit_perc >= 75) {
		grade = A;
	} else if (hit_perc >= 50) {
		grade = B;
	} else if (hit_perc >= 25) {
		grade = C;
	} else {
		grade = D;
	}

	return grade;
}

string Player::get_name() {
	return this->name.name;
}

void Player::set_name(string name) {
	this->name.name = name;
}

void Player::set_id(int id) {
	this->name.player_id = id;
}

int Player::get_id() {
	return this->name.player_id;
}

enum game_difficulty_e Player::get_diff() {
	return this->diff;
}

void Player::set_diff(enum game_difficulty_e diff) {
	this->diff = diff;
}

void Player::set_ask_board(bool state) {
	this->ask_board = state;
}

bool Player::do_ask_board() {
	return this->ask_board;
}

int *Player::get_last_attack_x() {
	return &last_x;
}

int *Player::get_last_attack_y() {
	return &last_y;
}

void Player::set_turn(bool state) {
	this->turn = state;
}

bool Player::his_turn() {
	return this->turn;
}

void Player::add_player_to_attack(Player *p) {
	struct attacked_player atk = { 
		.defender = NULL,
		.attacked = false,
		.atk = {
			.x = -1,
			.y = -1,
			.direction = RIGHT,
			.back_x = -1,
			.back_y = -1,
			.back_direction = RIGHT,
		},
	};
	atk.defender = p;
	this->attacks.push_back(atk);
}

void Player::ai_attack(Player *p) {
	if (!this->ai) {
		return;
	}
	
	srand(time(NULL));

	Board *b = p->get_board();

	int rand_x = 0;
	int rand_y = 0;
	int **board = b->get_board();
	struct attacked_player *atk = get_attack(p);
	struct ai_last_atk *ai_atk = &atk->atk;

	switch (diff) {
		case NORMAL:
			// Completely random
			do {
				rand_x = rand() % BOARD_SIZE;
				rand_y = rand() % BOARD_SIZE;
			} while (b->is_hit(rand_x, rand_y));
			board[rand_y][rand_x] += DAMAGE;
			break;
		case HARD:
			// Scan around a hit point to find others
			if (ai_atk->x == -1) {
				do {
					rand_x = rand() % BOARD_SIZE;
					rand_y = rand() % BOARD_SIZE;
				} while (b->is_hit(rand_x, rand_y));
				board[rand_y][rand_x] += DAMAGE;
				if (board[rand_y][rand_x] > DAMAGE) {
					ai_atk->x = rand_x;
					ai_atk->y = rand_y;
					ai_atk->back_x = rand_x;
					ai_atk->back_y = rand_y;
				}
			} else {
				bool exit = false;
				int cycle = 0;
				do {
					switch (ai_atk->direction) {
						case UP:
							if (!b->is_hit(ai_atk->x, ai_atk->y - 1) && ai_atk->y - 1 >= 0) {
								board[ai_atk->y - 1][ai_atk->x] += DAMAGE;
								rand_y = ai_atk->y - 1;
								rand_x = ai_atk->x;
								if (board[ai_atk->y - 1][ai_atk->x] == DAMAGE) {
									ai_atk->direction = RIGHT;
									if (board[ai_atk->y][ai_atk->x] > DAMAGE) {
										ai_atk->direction = ai_atk->back_direction;
										ai_atk->x = ai_atk->back_x;
										ai_atk->y = ai_atk->back_y;
									}
								} else {
									cycle = 0;
									ai_atk->y -= 1;
									ai_atk->back_direction = DOWN;
								}
								exit = true;
							} else {
								ai_atk->direction = RIGHT;
								if (ai_atk->y - 1 < 0 && board[ai_atk->y][ai_atk->x] > DAMAGE) {
									ai_atk->direction = ai_atk->back_direction;
									ai_atk->x = ai_atk->back_x;
									ai_atk->y = ai_atk->back_y;
								}
							}
							break;
						case RIGHT:
							if (!b->is_hit(ai_atk->x + 1, ai_atk->y) && ai_atk->x + 1 < BOARD_SIZE) {
								board[ai_atk->y][ai_atk->x + 1] += DAMAGE;
								rand_y = ai_atk->y;
								rand_x = ai_atk->x + 1;
								if (board[ai_atk->y][ai_atk->x + 1] == DAMAGE) {
									ai_atk->direction = DOWN;
									if (board[ai_atk->y][ai_atk->x] > DAMAGE) {
										ai_atk->direction = ai_atk->back_direction;
										ai_atk->x = ai_atk->back_x;
										ai_atk->y = ai_atk->back_y;
									}
								} else {
									cycle = 0;
									ai_atk->x += 1;
									ai_atk->back_direction = LEFT;
								}
								exit = true;
							} else {
								ai_atk->direction = DOWN;
								if (ai_atk->x + 1 >= BOARD_SIZE && board[ai_atk->y][ai_atk->x] > DAMAGE) {
									ai_atk->direction = ai_atk->back_direction;
									ai_atk->x = ai_atk->back_x;
									ai_atk->y = ai_atk->back_y;
								}
							}
							break;
						case DOWN:
							if (!b->is_hit(ai_atk->x, ai_atk->y + 1) && ai_atk->y + 1 < BOARD_SIZE) {
								board[ai_atk->y + 1][ai_atk->x] += DAMAGE;
								rand_y = ai_atk->y + 1;
								rand_x = ai_atk->x;
								if (board[ai_atk->y + 1][ai_atk->x] == DAMAGE) {
									ai_atk->direction = LEFT;
									if (board[ai_atk->y][ai_atk->x] > DAMAGE) {
										ai_atk->direction = ai_atk->back_direction;
										ai_atk->x = ai_atk->back_x;
										ai_atk->y = ai_atk->back_y;
									}
								} else {
									cycle = 0;
									ai_atk->y += 1;
									ai_atk->back_direction = UP;
								}
								exit = true;
							} else {
								ai_atk->direction = LEFT;
								if (ai_atk->y + 1 >= BOARD_SIZE && board[ai_atk->y][ai_atk->x] > DAMAGE) {
									ai_atk->direction = ai_atk->back_direction;
									ai_atk->x = ai_atk->back_x;
									ai_atk->y = ai_atk->back_y;
								}
							}
							break;
						case LEFT:
							if (!b->is_hit(ai_atk->x - 1, ai_atk->y) && ai_atk->x - 1 >= 0) {
								board[ai_atk->y][ai_atk->x - 1] += DAMAGE;
								rand_y = ai_atk->y;
								rand_x = ai_atk->x - 1;
								if (board[ai_atk->y][ai_atk->x - 1] == DAMAGE) {
									ai_atk->direction = UP;
									if (board[ai_atk->y][ai_atk->x] > DAMAGE) {
										ai_atk->direction = ai_atk->back_direction;
										ai_atk->x = ai_atk->back_x;
										ai_atk->y = ai_atk->back_y;
									}
								} else {
									cycle = 0;
									ai_atk->x -= 1;
									ai_atk->back_direction = RIGHT;
								}
								exit = true;
							} else {
								ai_atk->direction = UP;
								if (ai_atk->x - 1 < 0 && board[ai_atk->y][ai_atk->x] > DAMAGE) {
									ai_atk->direction = ai_atk->back_direction;
									ai_atk->x = ai_atk->back_x;
									ai_atk->y = ai_atk->back_y;
								}
							}
							break;
					}
					cycle++;
				} while (!exit && cycle < 20);
				if (cycle >= 20) {
					reset_ai_atk(p);
				}
			}
			break;
		case IMPOSSIBLE:
			// Hits 100% times
			for (rand_y = 0; rand_y < BOARD_SIZE; rand_y++) {
				for (rand_x = 0; rand_x < BOARD_SIZE; rand_x++) {
					if (board[rand_y][rand_x] > 0 && board[rand_y][rand_x] < DAMAGE) {
						board[rand_y][rand_x] += DAMAGE;
						goto cazzo;
					}
				}
			}
			break;
	}

	cazzo:
	Ship **ships = b->get_ships();
	if (board[rand_y][rand_x] > DAMAGE) {
		hit_shots++;
		for (int i = 0; i < SHIPS_COUNT; i++) {
			if (ships[i]->point_intersect(rand_x, rand_y)) {
				ships[i]->add_hit();
				if (ships[i]->is_sunk()) {
					p->dec_remaining_ships();
					if (diff == HARD) {
						reset_ai_atk(p);
					}
				}
				break;
			}
		}
	}
}

void Player::reset_ai_atk(Player *p) {
	if (this->ai) {
		struct attacked_player *atk = get_attack(p);
		if (atk != NULL) {
			atk->atk.direction = RIGHT;
			atk->atk.x = -1;
			atk->atk.y = -1;
		}
	}
}

struct attacked_player *Player::get_attack(Player *p) {
	for (size_t i = 0; i < attacks.size(); i++) {
		if (attacks[i].defender->get_id() == p->get_id()) {
			return &attacks[i];
		}
	}
	return NULL;
}

bool Player::is_ai() {
	return this->ai;
}

void Player::set_ai(bool state) {
	this->ai = state;
}

int Player::remaining_ships() {
	return (SHIPS_COUNT - this->own_sunk_ships);
}

void Player::dec_remaining_ships() {
	this->own_sunk_ships++;
}

int Player::get_sunk_ships() {
	return this->sunk_ships;
}

bool Player::is_winner() {
	return this->winner;
}

void Player::set_winner(bool state) {
	this->winner = state;
	this->loser = !state;
}

bool Player::is_loser() {
	return this->loser;
}

void Player::set_loser(bool state) {
	this->winner = !state;
	this->loser = state;
}

int Player::get_hits() {
	return this->hit_shots;
}

int Player::get_misses() {
	return this->missed_shots;
}

void Player::inc_hits() {
	this->hit_shots++;
}

void Player::inc_misses() {
	this->missed_shots++;
}

void Player::inc_sunk_ships() {
	this->sunk_ships++;
}

string Player::attacks_to_string() {
	string str = "{\n";
	for (size_t i = 0; i < attacks.size(); i++) {
		str += "player_id: " + std::to_string(attacks[i].defender->get_id());
		str += "\nis_attacked: " + string(attacks[i].attacked ? "true" : "false");
		str += "\natk {\n";
		str += "x: " + std::to_string(attacks[i].atk.x) + "\n";
		str += "y: " + std::to_string(attacks[i].atk.y) + "\n";
		str += "direction: " + std::to_string(attacks[i].atk.direction) + "\n";
		str += "back_x: " + std::to_string(attacks[i].atk.back_x) + "\n";
		str += "back_y: " + std::to_string(attacks[i].atk.back_y) + "\n";
		str += "back_direction: " + std::to_string(attacks[i].atk.back_direction) + "\n";
		str += "}\n},\n";
	}
	return str;
}

string Player::get_info() {
	string str = "PLAYER (" + name.name + ")";
	str += "\nid: " + std::to_string(name.player_id);
	str += "\nis_ai: " + string(ai ? "true" : "false");
	str += "\ncan_attack: " + string(turn ? "true" : "false");
	str += "\nmissed_shots: " + std::to_string(missed_shots);
	str += "\nhit_shots: " + std::to_string(hit_shots);
	str += "\nsunk_ships: " + std::to_string(sunk_ships);
	str += "\nown_sunk_ships: " + std::to_string(own_sunk_ships);
	str += "\nwinner: " + string(winner ? "true" : "false");
	str += "\nloser: " + string(loser ? "true" : "false");
	str += "\nattacks: {\n" + this->attacks_to_string() + "}\n";
	str += b->get_info();
	return str;
}

bool Player::is_host() {
	return this->host;
}

void Player::set_death(bool state) {
	this->dead = state;
}

bool Player::is_dead() {
	return this->dead;
}

void Player::set_end_time() {
	Match::set_time(this->end_time);
}

time_t Player::get_end_time() {
	return this->end_time;
}

void Player::set_placed_ships(bool state) {
	this->placed_ships = state;
}

bool Player::has_placed_ships() {
	return this->placed_ships;
}