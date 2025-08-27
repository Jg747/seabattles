#include <ai.hpp>

#include <string>

#include <player.hpp>

Ai::Ai(int number, enum game_difficulty_e difficulty) {
    set_ai(true);
	set_diff(difficulty);
	set_name("ai_" + std::to_string(number));
	ai_place_ships();
}
        
void Ai::ai_attack(Player *p) {
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

void Ai::reset_ai_atk(Player *p) {
	if (this->ai) {
		struct attacked_player *atk = get_attack(p);
		if (atk != NULL) {
			atk->atk.direction = RIGHT;
			atk->atk.x = -1;
			atk->atk.y = -1;
		}
	}
}

void Ai::ai_place_ships() {
    Board *b;
	Ship **ships;
	int rand_x;
	int rand_y;
	int rand_r;
	
	b = get_board();
	ships = b->get_ships();

	for (int i = 0; i < SHIPS_COUNT; ) {
		rand_x = (rand() % 10001) / 1000;
		rand_y = (rand() % 10001) / 1000;
		rand_r = (rand() % 4001) / 1000;
		if (Ship::evaluate_pos(rand_x, rand_y, ships[i]->get_len(), (enum rotation_e)rand_r)) {
			ships[i]->set_x(rand_x);
			ships[i]->set_y(rand_y);
			ships[i]->set_rotation((enum rotation_e)rand_r);
			ships[i]->set_placed(true);
			if (b->insert_on_board(ships[i])) {
				i++;
			}
		}
	}
	set_placed_ships(true);
}