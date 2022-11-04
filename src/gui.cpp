#include <gui.hpp>
#include <lang.hpp>
#include <debug.hpp>

#include <string>
#include <cmath>
#include <unistd.h>

using std::to_string;

Gui::Gui() {
	init_gui();
	this->m = NULL;
}

Gui::~Gui() {
	clear();
	endwin();
	del_array_win(game_wrapper, 2);
	del_array_win(start_menu, 2);
	if (this->m != NULL) {
		delete m;
		del_array_win(sea_border, 2);
		for (int i = 0; i < BOARD_SIZE + 1; i++) {
			for (int j = 0; j < BOARD_SIZE + 1; j++) {
				delwin(sea[i][j]);
			}
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
	
	init_pair((short)COLOR_SHIP, COLOR_WHITE, COLOR_WHITE);
	init_pair((short)COLOR_BLUE_TILE, COLOR_BLUE, COLOR_BLUE);
	init_pair((short)COLOR_AQUA_TILE, COLOR_CYAN, COLOR_CYAN);
	init_pair((short)COLOR_NOT_HIT, COLOR_RED, COLOR_RED);
	init_pair((short)COLOR_HIT, COLOR_YELLOW, COLOR_YELLOW);
	init_pair((short)COLOR_SELECT_PLACE, COLOR_GREEN, COLOR_GREEN);
	init_pair((short)COLOR_SELECT_HIT, COLOR_MAGENTA, COLOR_MAGENTA);

	curs_set(0);
	noecho();

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
	char buf[256];
	wgetnstr(w, buf, sizeof(buf));
	return string(buf);
}

void Gui::get_win_size(WINDOW *w, int &width, int &height) {
	getmaxyx(w, height, width);
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

int Gui::menu_cursor(WINDOW *w, int x, int y, int noptions, string symbol, bool step_last) {
	string space;
	for (int i = 0; symbol[i]; i++) {
		space.push_back(' ');
	}

	int ch = 0, select = 0;
	while (ch != '\n') {
		keypad(w, TRUE);
		ch = wgetch(w);
		keypad(w, FALSE);
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
	int width, height;
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

	return true;
}

void Gui::waiting_host() {

}

void Gui::write_fleet_type(string who) {
	int width, height;
	get_win_size(sea_border[0], width, height);
	mvwrite_on_window(sea_border[0], width/2 - 5 < 0 ? 0 : width/2 - 5, 0, string(who + " FLEET"));
	wrefresh(sea_border[0]);
}

void Gui::paint_sea(Board &p_board) {
	int height, width;
	bool blue = false;
	int i, j, k;

	int **board = p_board.get_board();
	Ship **ships = p_board.get_ships();

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
						wbkgd(sea[i][j], COLOR_PAIR(COLOR_SELECT_PLACE));
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

void Gui::paint_enemy_sea(Player &p) {
	int height, width;
	bool blue = false;
	int i, j, k;

	int **board = p.get_board()->get_board();
	Ship **ships = p.get_board()->get_ships();

	write_fleet_type(p.get_name());
	get_win_size(sea[0][0], width, height);
	
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
						wbkgd(sea[i][j], COLOR_PAIR(COLOR_SELECT_PLACE));
					} else {
						wbkgd(sea[i][j], COLOR_PAIR(COLOR_HIT));
					}
				} else if (debug_mode) {
					wbkgd(sea[i][j], COLOR_PAIR(COLOR_SHIP));
				} else {
					if (blue) {
						wbkgd(sea[i][j], COLOR_PAIR(COLOR_BLUE_TILE));
					} else {
						wbkgd(sea[i][j], COLOR_PAIR(COLOR_AQUA_TILE));
					}
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
}

void Gui::paint_actions_menu(enum action_e a, int &width, int &height) {
	get_win_size(actions[0], width, height);

	wclear(actions[0]);

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

void Gui::paint_ship(Player &p, int index) {
	Board *board = p.get_board();
	Ship *ship = (board->get_ships())[index];
	
	int x = ship->getX();
	int y = ship->getY();
	enum rotation_e rotate = ship->getRotation();
	int len = ship->getLen();
	enum colors color;

	if (ship->is_placed()) {
		color = COLOR_SHIP;
	} else if (!board->check_intersection(ship)) {
		color = COLOR_NOT_HIT;
	} else {
		color = COLOR_SELECT_PLACE;
	}

	paint_sea(*board);

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

int Gui::place_a_ship(Player *p, int index) {
	actions_menu(PLACE_A_SHIP);
	int width, height;
	get_win_size(actions[0], width, height);

	Board *b = p->get_board();

	if (b->get_ships()[index]->is_placed()) {
		b->insert_ship(index, REMOVE);
	}

	paint_ship(*p, index);

	int ch = 0;
	while (ch != '\n') {
		keypad(actions[0], TRUE);
		ch = wgetch(actions[0]);
		keypad(actions[0], FALSE);
		switch (ch) {
			case KEY_UP:
				b->insert_ship(index, MOVE_UP);
				paint_ship(*p, index);
				break;
			case KEY_RIGHT:
				b->insert_ship(index, MOVE_RIGHT);
				paint_ship(*p, index);
				break;
			case KEY_DOWN:
				b->insert_ship(index, MOVE_DOWN);
				paint_ship(*p, index);
				break;
			case KEY_LEFT:
				b->insert_ship(index, MOVE_LEFT);
				paint_ship(*p, index);
				break;
			case 'r':
				b->insert_ship(index, ROTATE);
				paint_ship(*p, index);
				break;
			case 'c':
				return -1;
				break;
		}
	}

	ch = b->insert_ship(index, PLACE);
	if (ch) {
		paint_ship(*p, index);
	}

	return ch;
}

bool Gui::place_ships(Player *p) {
	bool confirm = false;
	int choice, width, height, exit;

	get_win_size(actions[0], width, height);

	while (!confirm) {
		choice = actions_menu(PLACE_SHIPS);
		if (choice == 5) {
			if (!debug_mode) {
				if (p->get_board()->remaining_ships() < SHIPS_COUNT) {
					mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 7, "[Must place all ships!]");
					wrefresh(actions[0]);
					sleep(2);
					mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 6, "                       ");
					wrefresh(actions[0]);
				} else {
					confirm = true;
				}
			} else {
				confirm = true;
			}
		} else if (choice == 6) {
			if (actions_menu(FORFEIT) == 0) {
				return false;
			}
		} else {
			exit = 0;
			while (exit == 0) {
				exit = place_a_ship(p, choice);
				paint_sea(*(p->get_board()));
			}
		}
	}
	return true;
}

void Gui::paint_attack(Board *b, int x, int y) {
	int **board = b->get_board();
	Ship **ships = b->get_ships();

	if (board[y][x] > DAMAGE) {
		int i;
		for (i = 0; i < SHIPS_COUNT; i++) {
			if (ships[i]->point_intersect(x, y)) {
				break;
			}
		}
		if (!ships[i]->is_sunk()) {
			color_tile(y, x, COLOR_NOT_HIT);
		} else {
			color_tile(y, x, COLOR_HIT);
		}
	} else if (board[y][x] == DAMAGE) {
		color_tile(y, x, COLOR_HIT);
	} else {
		color_tile(y, x, COLOR_SELECT_HIT);
	}
}

int Gui::attack(Player &attacker, Player &defender) {
	actions_menu(ATTACK);

	int x = 0;
	int y = 0;

	paint_enemy_sea(defender);
	paint_attack(defender.get_board(), y, x);

	retry:
	int ch = 0;
	while (ch != '\n') {
		keypad(actions[0], TRUE);
		ch = wgetch(actions[0]);
		keypad(actions[0], FALSE);
		switch (ch) {
			case KEY_UP:
				if (y - 1 >= 0) {
					y--;
					paint_enemy_sea(defender);
					paint_attack(defender.get_board(), x, y);
				}
				break;
			case KEY_RIGHT:
				if (x + 1 < BOARD_SIZE) {
					x++;
					paint_enemy_sea(defender);
					paint_attack(defender.get_board(), x, y);
				}
				break;
			case KEY_DOWN:
				if (y + 1 < BOARD_SIZE) {
					y++;
					paint_enemy_sea(defender);
					paint_attack(defender.get_board(), x, y);
				}
				break;
			case KEY_LEFT:
				if (x - 1 >= 0) {
					x--;
					paint_enemy_sea(defender);
					paint_attack(defender.get_board(), x, y);
				}
				break;
			case 'q':
				return -1;
				break;
		}
	}

	int **board = defender.get_board()->get_board();
	Ship **enemy = defender.get_board()->get_ships();

	if (board[y][x] >= DAMAGE) {
		goto retry;
	}
	
	board[y][x] += DAMAGE;

	if (board[y][x] > DAMAGE) {
		for (int i = 0; i < SHIPS_COUNT; i++) {
			if (enemy[i]->point_intersect(x, y)) {
				attacker.inc_hits();
				enemy[i]->add_hit();
				if (enemy[i]->is_sunk()) {
					attacker.inc_sunk_ships();
					defender.dec_remaining_ships();
				}
				break;
			}
		}
	} else {
		attacker.inc_misses();
	}

	return 0;
}

enum game_status_e Gui::make_actions(Player &attacker, Player &defender) {
	int choice = actions_menu(GAME);
	switch (choice) {
		case 0:
			if (attack(attacker, defender) == -1) {
				return NO_ATK;
			}
			if (defender.remaining_ships() == 0) {
				return WIN;
			}
			break;
		case 1:
			paint_enemy_sea(defender);
			actions_menu(SEE_FIELD);
			paint_sea(*attacker.get_board());
			return NO_ATK;
			break;
		case 2:
			if (actions_menu(FORFEIT) == 0) {
				return QUITTING;
			}
			return NO_ATK;
			break;
	}
	return PROGRESS;
}

bool Gui::singleplayer() {
	Match::set_time(m->start_time);

	SingleMatch *m = (SingleMatch*)this->m;
	m->generate_match();
	
	Player *p = m->get_player();
	Player *ai = m->get_ai();
	
	if (!place_ships(p)) {
		return true;
	}

	while (m->get_status() == PROGRESS || m->get_status() == NO_ATK) {
		paint_sea(*p->get_board());
		m->set_status(make_actions(*p, *ai));
		if (m->get_status() == PROGRESS) {
			ai->ai_attack(*p);
			if (p->remaining_ships() == 0) {
				m->set_status(LOSE);
			}
		}
	}

	if (p->remaining_ships() == 0) {
		p->set_loser(true);
		ai->set_winner(true);
	} else if (ai->remaining_ships() == 0) {
		p->set_winner(true);
		ai->set_loser(true);
	}

	Match::set_time(m->end_time);

	if (m->get_status() != QUITTING) {
		box(start_menu[1], ACS_VLINE, ACS_HLINE);
		int width, height;
		get_win_size(start_menu[0], width, height);
		if (p->is_winner()) {
			mvwrite_on_window(start_menu[0], width/2 - 3, height/2 - 3, "YOU WON!");
		} else if (p->is_loser()) {
			mvwrite_on_window(start_menu[0], width/2 - 3, height/2 - 3, "YOU LOST!");
		}
		mvwrite_on_window(start_menu[0], width/2 - 8, height/2 - 1, "Total shots: " + to_string((p->get_hits() + p->get_misses())));
		mvwrite_on_window(start_menu[0], width/2 - 8, height/2, "Shots missed: " + to_string(p->get_misses()));
		mvwrite_on_window(start_menu[0], width/2 - 8, height/2 + 1, "Hits: " + to_string(p->get_hits()));
		mvwrite_on_window(start_menu[0], width/2 - 8, height/2 + 2, "Match time [" + m->get_duration() + "]");
		if (p->is_winner()) {
			mvwrite_on_window(start_menu[0], width/2 - 8, height/2 + 3, "Grade: " + string(1, p->get_grade()));
		}
		mvwrite_on_window(start_menu[0], width/2 - 13, height/2 + 5, "[press any key to continue]");
		wrefresh(start_menu[1]);
		wrefresh(start_menu[0]);
		getch();
	}

	return true;
}

bool Gui::start() {
	box(start_menu[1], ACS_VLINE, ACS_HLINE);
	wrefresh(start_menu[1]);
	wrefresh(start_menu[0]);

	int choice, diff;

	choice = game_menu();
	if (choice == 2) {
		return false;
	}
	
	switch (choice) {
		case SINGLEPLAYER:
			break;
		case MULTYPLAYER:
			break;
	}
	if (choice == SINGLEPLAYER) {
		diff = diff_menu();
		if (diff == 3) {
			return true;
		}
	} else {
		diff = multi_menu();
		switch (diff) {
			case 0:
				wait_conn_menu();
				break;
			case 1:
				if (join_menu()) {
					waiting_host();
				} else {
					return true;
				}
				break;
			case 2:
				return true;
				break;
		}
	}

	init_game_windows();

	switch (choice) {
		case SINGLEPLAYER:
			if (m == NULL) {
				m = new SingleMatch((enum game_difficulty_e)diff);
			} else {
				m->reset_match();
			}
			m->set_mode((enum gamemode_e)choice);
			m->set_difficulty((enum game_difficulty_e)diff);
			m->set_status(PROGRESS);
			return singleplayer();
			break;
		case MULTYPLAYER:
			// TO-DO
			break;
	}

	return true;
}