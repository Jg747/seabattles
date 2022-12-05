#include <string>
#include <cmath>
#include <unistd.h>
#include <thread>

#include <gui.hpp>
#include <msg.hpp>
#include <lang.hpp>
#include <debug.hpp>
#include <server.hpp>
#include <client.hpp>
#include <player.hpp>
#include <board.hpp>
#include <ship.hpp>

using std::to_string;

Gui::Gui(Client *c) {
	init_gui();
	in_zone = M_PRE_GAME;
	print_window = true;
	this->client = c;
}

Gui::~Gui() {
	clear();
	endwin();
	del_array_win(game_wrapper, 2);
	del_array_win(start_menu, 2);
	del_array_win(sea_border, 2);
	del_array_win(actions, 2);
	for (int i = 0; i < BOARD_SIZE + 1; i++) {
		for (int j = 0; j < BOARD_SIZE + 1; j++) {
			delwin(sea[i][j]);
		}
	}
}

void Gui::del_array_win(WINDOW* array[], int len) {
	for (int i = 0; i < len; i++) {
		if (array[i] != NULL) {
			delwin(array[i]);
		}
	}
}

void Gui::init_gui() {
	initscr();
	start_color();
	
	init_pair((short)COLOR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
	init_pair((short)COLOR_BLUE_TILE, COLOR_BLUE, COLOR_BLUE);
	init_pair((short)COLOR_AQUA_TILE, COLOR_CYAN, COLOR_CYAN);
	init_pair((short)COLOR_SHIP, COLOR_WHITE, COLOR_WHITE);
	init_pair((short)COLOR_HIT, COLOR_YELLOW, COLOR_YELLOW);
	init_pair((short)COLOR_NOT_HIT, COLOR_RED, COLOR_RED);
	init_pair((short)COLOR_SUNK, COLOR_GREEN, COLOR_GREEN);
	init_pair((short)COLOR_CORRECT_PLACE, COLOR_GREEN, COLOR_GREEN);
	init_pair((short)COLOR_INCORRECT_PLACE, COLOR_RED, COLOR_RED);
	init_pair((short)COLOR_SELECT_HIT, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair((short)COLOR_ALREADY_HIT, COLOR_WHITE, COLOR_WHITE);
	init_pair((short)COLOR_TEXT_GREEN, COLOR_GREEN, COLOR_BLACK);

	curs_set(0);
	noecho();
	bkgd(COLOR_PAIR(COLOR_DEFAULT));

	//Borders
	game_wrapper[1] = newwin(LINES, COLS, 0, 0);
	box(game_wrapper[1], ACS_VLINE, ACS_HLINE);

	start_menu[1] = newwin(LINES-3, COLS-2, 2, 1);
	box(start_menu[1], ACS_VLINE, ACS_HLINE);

	//Title
	game_wrapper[0] = newwin(LINES-2, COLS-2, 1, 1);
	mvwprintw(game_wrapper[0], 0, (COLS/2)-(string(string(TITLE) + " " + string(GAME_VERSION)).length()/2), "%s %s", TITLE, GAME_VERSION);
	
	//Execution
	start_menu[0] = newwin(LINES-5, COLS-4, 3, 2);

	refresh();
	wrefresh(game_wrapper[1]);
	wrefresh(game_wrapper[0]);
	wrefresh(start_menu[1]);
	wrefresh(start_menu[0]);
}

void Gui::init_game_windows() {
	int col = COLS * PERCENT_SEA / 100;
	int line = LINES - 3;

	sea_border[1] = newwin(line, col + 1, 2, 1);
	box(sea_border[1], ACS_VLINE, ACS_HLINE);

	sea_border[0] = newwin(line - 2, col - 1, 3, 2);

	wrefresh(sea_border[1]);
	wrefresh(sea_border[0]);

	// SEA
	init_sea();

	actions[1] = newwin(line, (COLS * (100 - PERCENT_SEA) / 100) - 3 + (COLS - 2 - (col + 1 + (COLS * (100 - PERCENT_SEA) / 100) - 3)), 2, 2 + col);
	box(actions[1], ACS_VLINE, ACS_HLINE);

	int max_lines, max_cols;
	get_win_size(actions[1], max_cols, max_lines);
	actions[0] = newwin(max_lines - 2, max_cols - 2, 3, 3 + col);

	wrefresh(actions[1]);
	wrefresh(actions[0]);
}

void Gui::init_sea() {
	int col = COLS * PERCENT_SEA / 100;
	int line = LINES - 3;

	int x = 2, y = 3;
	int x_inc = col / (BOARD_SIZE + 1);
	int y_inc = line / (BOARD_SIZE + 1);

	for (int i = 0; i < BOARD_SIZE + 1; i++, x += x_inc, y += y_inc);

	int x_off = (col - x) / 2;
	int y_off = (line - y) / 2;

	y = 3 + y_off;

	char col_name = 'A';
	int row_name = 1;
	bool blue = true;

	write_fleet_type("YOUR");

	for (int i = 0; i < BOARD_SIZE + 1; i++) {
		x = 2 + x_off;
		for (int j = 0; j < BOARD_SIZE + 1; j++) {
			sea[i][j] = newwin(y_inc, x_inc, y, x);
			if (i != 0 && j != 0) {
				if (y_inc > 1) {
					box(sea[i][j], ACS_VLINE, ACS_HLINE);
				}
				if (blue) {
					wbkgd(sea[i][j], COLOR_PAIR(COLOR_BLUE_TILE));
				} else {
					wbkgd(sea[i][j], COLOR_PAIR(COLOR_AQUA_TILE));
				}
				blue = !blue;
			} else if (i == 0) {
				if (j != 0) {
					mvwrite_on_window(sea[i][j], x_inc/2, y_inc/2, string(1, col_name));
					col_name++;
				}
			} else if (j == 0) {
				mvwrite_on_window(sea[i][j], x_inc/2, y_inc/2, to_string(row_name));
				row_name++;
			}
			wrefresh(sea[i][j]);
			x += x_inc;
		}
		blue = !blue;
		y += y_inc;
	}
}

void Gui::write_on_window(WINDOW *w, string str) {
	wprintw(w, "%s", str.c_str());
}

void Gui::mvwrite_on_window(WINDOW *w, int x, int y, string str) {
	mvwprintw(w, y, x, "%s", str.c_str());
}

string Gui::get_input(WINDOW *w) {
	string buf = "";
	int c;
	keypad(w, true);
	while ((c = wgetch(w)) != '\n') {
		switch (c) {
			case KEY_BACKSPACE:
			case '\b':
			case 127:
				buf.pop_back();
				break;
			default:
				buf.push_back(c);
				break;
		}
	}
	keypad(w, false);
	return buf;
}

void Gui::new_zone(enum input_zone_e new_zone) {
	in_zone = new_zone;
	print_window = true;
}

void Gui::get_win_size(WINDOW *w, int &width, int &height) {
	getmaxyx(w, height, width);
}

int Gui::menu_cursor(WINDOW *w, int x, int y, int noptions, string symbol, bool step_last) {
	string space;
	for (int i = 0; symbol[i]; i++) {
		space.push_back(' ');
	}

	int ch = 0;

	if (in_zone != M_PRE_GAME) {
		keypad(w, true);
		ch = wgetch(w);
		keypad(w, false);
		switch (ch) {
			case KEY_UP:
				if (select > 0) {
					mvwrite_on_window(w, x, y + select - 1, symbol);
					mvwrite_on_window(w, x, y + select + (step_last && select == noptions - 1 ? 1 : 0), space);
					wrefresh(w);
					select--;
				} else {
					select = noptions - 1;
					mvwrite_on_window(w, x, y, space);
					mvwrite_on_window(w, x, y + select + (step_last && select == noptions - 1 ? 1 : 0), symbol);
					wrefresh(w);
				}
				break;
			case KEY_DOWN:
				if (select < noptions - 1) {
					mvwrite_on_window(w, x, y + select, space);
					mvwrite_on_window(w, x, y + select + (step_last && select == noptions - 2 ? 1 : 0) + 1, symbol);
					wrefresh(w);
					select++;
				} else {
					mvwrite_on_window(w, x, y + select + (step_last && select == noptions - 1 ? 1 : 0), space);
					select = 0;
					mvwrite_on_window(w, x, y, symbol);
					wrefresh(w);
				}
				break;
			case '\n':
				return select;
				break;
			default: break;
		}

		return -1;
	} else {
		select = 0;
		while (ch != '\n') {
			keypad(w, true);
			ch = wgetch(w);
			keypad(w, false);
			switch (ch) {
				case KEY_UP:
					if (select > 0) {
						mvwrite_on_window(w, x, y + select - 1, symbol);
						mvwrite_on_window(w, x, y + select + (step_last && select == noptions - 1 ? 1 : 0), space);
						wrefresh(w);
						select--;
					} else {
						select = noptions - 1;
						mvwrite_on_window(w, x, y, space);
						mvwrite_on_window(w, x, y + select + (step_last && select == noptions - 1 ? 1 : 0), symbol);
						wrefresh(w);
					}
					break;
				case KEY_DOWN:
					if (select < noptions - 1) {
						mvwrite_on_window(w, x, y + select, space);
						mvwrite_on_window(w, x, y + select + (step_last && select == noptions - 2 ? 1 : 0) + 1, symbol);
						wrefresh(w);
						select++;
					} else {
						mvwrite_on_window(w, x, y + select + (step_last && select == noptions - 1 ? 1 : 0), space);
						select = 0;
						mvwrite_on_window(w, x, y, symbol);
						wrefresh(w);
					}
					break;
				default: break;
			}
		}
		
		return select;
	}
}

void Gui::write_fleet_type(string who) {
	int width, height;
	get_win_size(sea_border[0], width, height);
	mvwrite_on_window(sea_border[0], width/2 - 5 < 0 ? 0 : width/2 - 5, 0, string(who + " FLEET"));
	wrefresh(sea_border[0]);
}

void Gui::paint_placement_sea() {
	int height, width;
	bool blue = false;
	int i, j, k;

	int **board = dummy->get_board()->get_board();
	Ship **ships = dummy->get_board()->get_ships();

	write_fleet_type("YOUR");
	get_win_size(sea[0][0], width, height);
	
	// SEGMENTATION FAULT
	for (i = 1; i < BOARD_SIZE + 1; i++) {
		for (j = 1; j < BOARD_SIZE + 1; j++) {
			if (height > 1) {
				box(sea[i][j], ACS_VLINE, ACS_HLINE);
			}
			if (board[i-1][j-1] > 0) {
				if (board[i-1][j-1] == DAMAGE) {
					wbkgd(sea[i][j], COLOR_PAIR(COLOR_NOT_HIT));
				} else if (board[i-1][j-1] > DAMAGE) {
					for (k = 0; k < SHIPS_COUNT; k++) {
						if (ships[k]->point_intersect(j-1, i-1)) {
							break;
						}
					}
					if (ships[k]->is_sunk()) {
						wbkgd(sea[i][j], COLOR_PAIR(COLOR_SUNK));
					} else {
						wbkgd(sea[i][j], COLOR_PAIR(COLOR_HIT));
					}
				} else {
					wbkgd(sea[i][j], COLOR_PAIR(COLOR_SHIP));
				}
			} else {
				if (blue) {
					wbkgd(sea[i][j], COLOR_PAIR(COLOR_BLUE_TILE));
				} else {
					wbkgd(sea[i][j], COLOR_PAIR(COLOR_AQUA_TILE));
				}
			}
			wrefresh(sea[i][j]);
			blue = !blue;
		}
		if (BOARD_SIZE % 2 == 0) {
			blue = !blue;
		}
	}
	//-------
}

int Gui::game_menu() {
	int x, y;
	get_win_size(start_menu[0], x, y);
	
	wclear(start_menu[0]);
	mvwrite_on_window(start_menu[0], x/2 - 2, y/2 - 4, "MENU");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 2, "> Singleplayer");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 1, "  Multiplayer");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2 + 1, "  Exit");
	wrefresh(start_menu[0]);
	
	return menu_cursor(start_menu[0], x/2 - 7, y/2 - 2, 3, ">", true);
}

int Gui::diff_menu() {
	int x, y;
	get_win_size(start_menu[0], x, y);
	
	wclear(start_menu[0]);
	mvwrite_on_window(start_menu[0], x/2 - 8, y/2 - 4, "SELECT DIFFICULTY");
	mvwrite_on_window(start_menu[0], x/2 - 3, y/2 - 2, "> Normal");
	mvwrite_on_window(start_menu[0], x/2 - 3, y/2 - 1, "  Hard");
	mvwrite_on_window(start_menu[0], x/2 - 3, y/2, "  Impossible");
	mvwrite_on_window(start_menu[0], x/2 - 3, y/2 + 2, "  Back");
	wrefresh(start_menu[0]);

	return menu_cursor(start_menu[0], x/2 - 3, y/2 - 2, 4, ">", true);
}

void Gui::paint_actions_menu(enum action_e a, int &width, int &height) {
	if (!print_window) {
		return;
	}

	get_win_size(actions[0], width, height);
	wclear(actions[0]);

	select = 0;

	switch (a) {
		case PLACE_SHIPS:
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 - 4, "SHIP PLACEMENT");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 - 2, "> Carrier");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 - 1, "  Battleship");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2, "  Destroyer");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 1, "  Submarine");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 2, "  Patrol boat");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 3, "  Confirm");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 5, "  Quit");
			width = width/2 - 6 < 0 ? 0 : width/2 - 6;
			height = height/2 - 2;
			break;
		case PLACE_A_SHIP:
			mvwrite_on_window(actions[0], width/2 - 7 < 0 ? 0 : width/2 - 7, height/2 - 2, "SHIP PLACEMENT");
			mvwrite_on_window(actions[0], width/2 - 7 < 0 ? 0 : width/2 - 7, height/2, "Arrow keys to move");
			mvwrite_on_window(actions[0], width/2 - 7 < 0 ? 0 : width/2 - 7, height/2 + 2, "R to rotate");
			mvwrite_on_window(actions[0], width/2 - 7 < 0 ? 0 : width/2 - 7, height/2 + 3, "ENTER to place");
			mvwrite_on_window(actions[0], width/2 - 7 < 0 ? 0 : width/2 - 7, height/2 + 4, "C to change ship");
			width = width/2 - 7 < 0 ? 0 : width/2 - 7;
			height = height/2 - 1;
			break;
		case GAME:
			mvwrite_on_window(actions[0], width/2 - 2, height/2 - 2, "GAME");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2, "> Attack");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 1, "  See field");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 2, "  Forfeit");
			width = width/2 - 6 < 0 ? 0 : width/2 - 6;
			height = height/2;
			break;
		case SEE_FIELD:
			mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2, "[press any key to return]");
			break;
		case ATTACK:
			mvwrite_on_window(actions[0], width/2 - 4 < 0 ? 0 : width/2 - 4, height/2 - 2, "ATTACKING");
			mvwrite_on_window(actions[0], width/2 - 7 < 0 ? 0 : width/2 - 7, height/2, "Arrow keys to move");
			mvwrite_on_window(actions[0], width/2 - 7 < 0 ? 0 : width/2 - 7, height/2 + 2, "ENTER to attack");
			mvwrite_on_window(actions[0], width/2 - 7 < 0 ? 0 : width/2 - 7, height/2 + 3, "Q to return");
			width = width/2 - 7 < 0 ? 0 : width/2 - 7;
			height = height/2 - 1;
			break;
		case FORFEIT:
			mvwrite_on_window(actions[0], width/2 - 2, height/2 - 2, "GAME");
			mvwrite_on_window(actions[0], width/2 - 6, height/2, "Are you sure?");
			mvwrite_on_window(actions[0], width/2 - 3, height/2 + 2, "> Yes");
			mvwrite_on_window(actions[0], width/2 - 3, height/2 + 3, "  No");
			width = width/2 - 3;
			height = height/2 + 2;
			break;
		case ALLY:
			// Implementation only in multiplayer
			break;
	}
	print_window = false;
}

int Gui::actions_menu(enum action_e a) {
	int x, y;
	
	wclear(actions[0]);
	paint_actions_menu(a, x, y);
	wrefresh(actions[0]);

	switch (a) {
		case PLACE_SHIPS:
			return menu_cursor(actions[0], x, y, 7, ">", true);
			break;
		case PLACE_A_SHIP:
			return 0;
			break;
		case GAME:
			return menu_cursor(actions[0], x, y, 3, ">", false);
			break;
		case SEE_FIELD:
			wgetch(actions[0]);
			new_zone(M_ACTIONS);
			break;
		case ATTACK:
			return 0;
			break;
		case FORFEIT:
			return menu_cursor(actions[0], x, y, 2, ">", false);
			break;
		case ALLY:
			// Implementation only in multiplayer
			break;
	}

	return 0;
}

void Gui::color_tile(int i, int j, enum colors color) {
	wbkgd(sea[i+1][j+1], COLOR_PAIR(color));
	wrefresh(sea[i+1][j+1]);
}

void Gui::paint_ship(int index) {
	Board *board = dummy->get_board();
	Ship *ship = (board->get_ships())[index];
	
	int x = ship->get_x();
	int y = ship->get_y();
	enum rotation_e rotate = ship->get_rotation();
	int len = ship->get_len();
	enum colors color;

	if (ship->is_placed()) {
		color = COLOR_SHIP;
	} else if (!board->check_intersection(ship)) {
		color = COLOR_INCORRECT_PLACE;
	} else {
		color = COLOR_CORRECT_PLACE;
	}

	paint_placement_sea();

	switch (rotate) {
		case UP:
			for (int i = y; i > y - len; i--) {
				color_tile(i, x, color);
			}
			break;
		case RIGHT:
			for (int i = x; i < x + len; i++) {
				color_tile(y, i, color);
			}
			break;
		case DOWN:
			for (int i = y; i < y + len; i++) {
				color_tile(i, x, color);
			}
			break;
		case LEFT:
			for (int i = x; i > x - len; i--) {
				color_tile(y, i, color);
			}
			break;
	}
}

int Gui::place_a_ship(int index) {
	actions_menu(PLACE_A_SHIP);
	int width, height;
	get_win_size(actions[0], width, height);

	Board *b = dummy->get_board();

	if (b->get_ships()[index]->is_placed()) {
		b->insert_ship(index, REMOVE);
	}

	paint_ship(index);

	int ch = 0;
	keypad(actions[0], TRUE);
	ch = wgetch(actions[0]);
	keypad(actions[0], FALSE);
	switch (ch) {
		case 'w':
		case KEY_UP:
			b->insert_ship(index, MOVE_UP);
			paint_ship(index);
			break;
		case 'd':
		case KEY_RIGHT:
			b->insert_ship(index, MOVE_RIGHT);
			paint_ship(index);
			break;
		case 's':
		case KEY_DOWN:
			b->insert_ship(index, MOVE_DOWN);
			paint_ship(index);
			break;
		case 'a':
		case KEY_LEFT:
			b->insert_ship(index, MOVE_LEFT);
			paint_ship(index);
			break;
		case 'r':
			b->insert_ship(index, ROTATE);
			paint_ship(index);
			break;
		case 'c':
			new_zone(M_PLACE_SHIPS);
			return 0;
			break;
		case '\n':
			ch = b->insert_ship(index, PLACE);
			if (ch == 1) {
				paint_ship(index);
				new_zone(M_PLACE_SHIPS);
			}
			return ch == 1;
			break;
	}

	return -1;
}

bool Gui::send_board() {
	msg_creation msg;
	msg_parsing recv;

	msg.msg_type = MSG_PLAYER_SHIP_PLACEMENT;
	Ship **ship_array = dummy->get_board()->get_ships();
	struct ship_t *ship;
	for (int i = 0; i < SHIPS_COUNT; i++) {
		ship = &msg.data.player_ship_placement.array[i];
		ship->type = ship_array[i]->get_type();
		ship->x = ship_array[i]->get_x();
		ship->y = ship_array[i]->get_y();
		ship->rotation = ship_array[i]->get_rotation();
	}

	client->send_message(&msg);
	client->receive_message(&recv);

	if (recv.msg_type == ACK) {
		return true;
	}
	return false;
}

void Gui::send_forfeit(msg_parsing *msg) {
	msg_creation c_msg;
	msg_parsing r_msg;

	c_msg.msg_type = MSG_PLAYER_QUIT;
	client->send_message(&c_msg);
	client->receive_message(&r_msg);

	if (msg != NULL) {
		*msg = r_msg;
	}
}

int Gui::place_ships() {
	int choice, width, height;

	get_win_size(actions[0], width, height);

	choice = actions_menu(PLACE_SHIPS);
	if (choice == 5) {
		if (dummy->get_board()->remaining_ships() < SHIPS_COUNT) {
			mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 7, "[Must place all ships!]");
			wrefresh(actions[0]);
			sleep(2);
			mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 7, "                       ");
			wrefresh(actions[0]);
		} else {
			if (send_board()) {
				new_zone(M_NO_INPUT);
			} else {
				mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 7, "[Invalid placement!]");
				wrefresh(actions[0]);
				sleep(2);
				mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 7, "                    ");
				wrefresh(actions[0]);
			}
		}
	} else if (choice == 6) {
		if (actions_menu(FORFEIT) == 0) {
			send_forfeit(NULL);
			return -2;
		}
	} else if (choice >= 0 && choice <= 4) {
		return choice;
	}
	
	return -1;
}

void Gui::paint_enemy_sea(Player *defender) {
	write_fleet_type(defender->get_name());
	int **matrix;
	matrix = defender->get_board()->get_board();

	int height, width;
	get_win_size(sea[0][0], width, height);

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			if (height > 1) {
				box(sea[i][j], ACS_VLINE, ACS_HLINE);
			}
			color_tile(i, j, (enum colors)matrix[i][j]);
		}
	}
}

void Gui::view_field(Player *defender) {
	msg_creation c_msg;
	msg_parsing r_msg;

	c_msg.msg_type = MSG_PLAYER_GET_BOARD;
	c_msg.data.player_get_board.id = defender->get_id();

	do {
		client->send_message(&c_msg);
		client->receive_message(&r_msg);
	} while (r_msg.msg_type != ACK_MSG_GET_BOARD);

	int **matrix;
	matrix = defender->get_board()->get_board();
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			matrix[i][j] = r_msg.data.ack_get_board.board.matrix[i][j];
		}
	}
	paint_enemy_sea(defender);
}

void Gui::paint_attack(int **board, int y, int x) {
	if (board[y][x] == COLOR_NOT_HIT || board[y][x] == COLOR_HIT || board[y][x] == COLOR_SUNK) {
		color_tile(y, x, COLOR_ALREADY_HIT);
	} else {
		color_tile(y, x, COLOR_SELECT_HIT);
	}
}

bool Gui::attack_at(Player *defender, int x, int y) {
	int **matrix;
	matrix = defender->get_board()->get_board();
	if (matrix[y][x] != COLOR_AQUA_TILE && matrix[y][x] != COLOR_BLUE_TILE) {
		return false;
	}

	msg_creation c_msg;
	msg_parsing p_msg;

	c_msg.msg_type = MSG_PLAYER_ATTACK;
	c_msg.data.player_attack.player.player_id = defender->get_id();
	c_msg.data.player_attack.x = x;
	c_msg.data.player_attack.y = y;

	do {
		client->send_message(&c_msg);
		client->receive_message(&p_msg);
	} while (p_msg.msg_type != ACK_MSG_MATCH_ATTACK_STATUS && p_msg.msg_type != ACK_MSG_MATCH_ATTACK_ERR);
	
	if (p_msg.msg_type == ACK_MSG_MATCH_ATTACK_ERR) {
		return false;
	}
	return true;
}

bool Gui::attack(Player *defender) {
	int *x, *y;

	x = defender->get_last_attack_x();
	y = defender->get_last_attack_y();

	if (defender->do_ask_board()) {
		actions_menu(ATTACK);
		view_field(defender);
		*x = 0;
		*y = 0;
		defender->set_ask_board(false);
	} else {
		paint_enemy_sea(defender);
	}

	paint_attack(defender->get_board()->get_board(), *y, *x);

	int ch;
	keypad(actions[0], TRUE);
	ch = wgetch(actions[0]);
	keypad(actions[0], FALSE);
	
	switch (ch) {
		case 'w':
		case KEY_UP:
			if ((*y) - 1 >= 0) {
				(*y)--;
				paint_enemy_sea(defender);
				paint_attack(defender->get_board()->get_board(), *y, *x);
			}
			break;
		case 'd':
		case KEY_RIGHT:
			if ((*x) + 1 < BOARD_SIZE) {
				(*x)++;
				paint_enemy_sea(defender);
				paint_attack(defender->get_board()->get_board(), *y, *x);
			}
			break;
		case 's':
		case KEY_DOWN:
			if ((*y) + 1 < BOARD_SIZE) {
				(*y)++;
				paint_enemy_sea(defender);
				paint_attack(defender->get_board()->get_board(), *y, *x);
			}
			break;
		case 'a':
		case KEY_LEFT:
			if ((*x) - 1 >= 0) {
				(*x)--;
				paint_enemy_sea(defender);
				paint_attack(defender->get_board()->get_board(), *y, *x);
			}
			break;
		case 'q':
			new_zone(M_ACTIONS);
			return false;
			break;
		case '\n':
			if (attack_at(defender, *x, *y)) {
				defender->set_ask_board(true);
				new_zone(M_ACTIONS);
				return true;
			}
			return false;
			break;
	}

	return false;
}

void Gui::make_actions(Player *defender) {
	int choice = actions_menu(GAME);
	switch (choice) {
		case 0:
			new_zone(M_ATTACK);
			break;
		case 1:
			view_field(defender);
			new_zone(M_SEE_FIELD);
			break;
		case 2:
			new_zone(M_FORFEIT);
			break;
	}
}

bool Gui::pregame() {
	new_zone(M_PRE_GAME);

	int value = game_menu();
    if (value == 2) {
        return false;
    }

    if (value == SINGLEPLAYER) {
		this->mode = SINGLEPLAYER;
		dummy = new Player(true);
		client->create_server();

        value = diff_menu();
        if (value == 3) {
            return false;
        }

		if (!init_singleplayer_game((enum game_difficulty_e)value, 2)) {
			return false;
		}

		new_zone(M_NO_INPUT);
    } else if (value == MULTIPLAYER) {
		this->mode = MULTIPLAYER;
        // ...
    }

    init_game_windows();

	return true;
}

bool Gui::init_singleplayer_game(enum game_difficulty_e diff, int num_ai) {
	msg_creation msg;
	msg_parsing recv;

	msg.msg_type = MSG_HOST_INIT_MATCH;
	msg.data.host_init_match.difficulty = diff;
	msg.data.host_init_match.ais = num_ai;
	client->send_message(&msg);
	client->receive_message(&recv);
	if (recv.msg_type != ACK_MSG_MATCH_INIT_MATCH || recv.data.ack_match_init_match.status != GS_OK) {
		return false;
	}

	msg.msg_type = MSG_HOST_START_MATCH;
	client->send_message(&msg);
	client->receive_message(&recv);
	if (recv.msg_type != ACK) {
		return false;
	}

	msg.msg_type = MSG_PLAYER_GET_OWN_ID;
	msg.data.player_get_own_id.username = string(DEFAULT_PLAYER_NAME);
	client->send_message(&msg);
	client->receive_message(&recv);

	if (recv.msg_type != ACK_MSG_PLAYER_GET_OWN_ID) {
		return false;
	}
	dummy->set_name(string(DEFAULT_PLAYER_NAME));
	dummy->set_id(recv.data.ack_player_get_own_id.player_id);
	dummy->set_diff(diff);

	return true;
}

bool Gui::do_from_input() {
	int temp_value;

	switch (in_zone) {
		case M_NO_INPUT:
			break;
		case M_SEE_FIELD:
			actions_menu(SEE_FIELD);
			paint_placement_sea();
			break;
		case M_PLACE_SHIPS:
			value = place_ships();
			if (value == -2) {
				return false;
			}
			if (value == -1) {
				new_zone(M_NO_INPUT);
				return true;
			}
			new_zone(M_PLACE_A_SHIP);
			break;
		case M_PLACE_A_SHIP:
			temp_value = place_a_ship(value);
			if (temp_value > -1) {
				paint_placement_sea();
			}
			break;
		case M_FORFEIT:
			if (actions_menu(FORFEIT) == 0) {
				return false;
			}
			break;
		case M_ACTIONS:
			if (mode == SINGLEPLAYER) {
				make_actions(p_list.at(2));
			} else if (mode == MULTIPLAYER) {
				// ...
			}
			break;
		case M_ATTACK:
			if (mode == SINGLEPLAYER) {
				if (attack(p_list.at(2))) {
					paint_placement_sea();
				}
			} else if (mode == MULTIPLAYER) {
				// ...
			}
			break;
		default:
			break;
	}

	return true;
}

/************************************************/
/*                 SERVER MENUS                 */
/************************************************/
void Gui::wait_conn_menu() {
	// TO-DO
}

int Gui::multi_menu() {
	int x, y;
	get_win_size(start_menu[0], x, y);
	wclear(start_menu[0]);
	mvwrite_on_window(start_menu[0], x/2 - 5, y/2 - 4, "MULTIPLAYER");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 2, "> Host a match");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 1, "  Join a match");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2, "  Back");
	wrefresh(start_menu[0]);

	return menu_cursor(start_menu[0], x/2 - 7, y/2 - 2, 4, ">", true);
}

bool Gui::join_menu() {
	/*int width, height;
	get_win_size(start_menu[0], width, height);
	
	wclear(start_menu[0]);
	mvwrite_on_window(start_menu[0], width/2 - 5, height/2 - 4, "MULTIPLAYER");
	mvwrite_on_window(start_menu[0], width/2 - 6, height/2 - 2, "Enter host IP");
	mvwrite_on_window(start_menu[0], width/2 - 8, height/2 + 4, "> back      join");
	
	WINDOW *ip = newwin(3, 24, height/2 + 2, width/2 - 10 < 0 ? 0 : width/2 - 10);
	box(ip, ACS_VLINE, ACS_HLINE);

	wrefresh(start_menu[0]);
	wrefresh(ip);

	int car = 0, choice;
	string str;

	do {
		while (car != '\n') {
			keypad(start_menu[0], TRUE);
			car = wgetch(start_menu[0]);
			keypad(start_menu[0], FALSE);
			switch (car) {
				case KEY_LEFT:
					mvwrite_on_window(start_menu[0], width/2 - 8, height/2 + 4, "> back      join");
					wrefresh(start_menu[0]);
					choice = 0;
					break;
				case KEY_RIGHT:
					mvwrite_on_window(start_menu[0], width/2 - 8, height/2 + 4, "  back    > join");
					wrefresh(start_menu[0]);
					choice = 1;
					break;
				case KEY_BACKSPACE:
					if (!str.empty()) {
						str.pop_back();
						mvwprintw(ip, 1, 1, "%s ", str.c_str());
						wrefresh(ip);
					}
					break;
				default:
					if ((car >= '0' && car <= '9') || car == '.' || car == ':') {
						str.push_back(car);
						mvwprintw(ip, 1, 1, "%s", str.c_str());
						wrefresh(ip);
					}
					break;
			}
		}

		if (choice == 0) {
			return false;
		}

	} while (!((MultiMatch*)m)->check_connection(str));

	return true;*/
	return true;
}

void Gui::waiting_host() {

}