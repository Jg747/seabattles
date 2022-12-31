#include <string>
#include <cmath>
#include <unistd.h>
#include <thread>

#include <debug.hpp>
#include <gui.hpp>
#include <msg.hpp>
#include <lang.hpp>
#include <server.hpp>
#include <client.hpp>
#include <player.hpp>
#include <board.hpp>
#include <ship.hpp>
#include <thread_manager.hpp>

using std::to_string;

Gui::Gui(Client *c, struct thread_manager_t *mng) {
	init_gui();
	this->client = c;
	this->mng = mng;
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
	if (Logger::debug) {
		del_array_win(debug_win, 2);
		Logger::win = NULL;
	}
}

void Gui::del_array_win(WINDOW* array[], int len) {
	for (int i = 0; i < len; i++) {
		if (array[i] != NULL) {
			delwin(array[i]);
		}
	}
}

void Gui::debug_window() {
	if (Logger::debug) {
		box(debug_win[1], ACS_VLINE, ACS_HLINE);
		wrefresh(debug_win[1]);
		wclear(debug_win[0]);
		wprintw(debug_win[0], "%s", Logger::get_last_lines().c_str());
		wrefresh(debug_win[0]);
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
	if (!Logger::debug) {
		mvwprintw(game_wrapper[0], 0, (COLS/2)-(string(string(TITLE) + " " + string(GAME_VERSION)).length()/2), "%s %s", TITLE, GAME_VERSION);
	} else {
		mvwprintw(game_wrapper[0], 0, (COLS/6)-string(string(TITLE) + string(GAME_VERSION)).length()/2, "%s %s", TITLE, GAME_VERSION);
	}

	//Debug
	if (Logger::debug) {
		Logger::lines = round((LINES / 100.0) * (float)PERCENT_DEBUG) - 2;

		debug_win[1] = newwin(Logger::lines + 2, (COLS / 3 * 2) + 2, 0, COLS - ((COLS / 3 * 2) + 2));
		box(debug_win[1], ACS_VLINE, ACS_HLINE);

		debug_win[0] = newwin(Logger::lines, (COLS / 3) * 2, 1, COLS - 1 - (COLS / 3) * 2);
		scrollok(debug_win[0], true);
		
		Logger::win = debug_win[0];
	}

	//Execution
	start_menu[0] = newwin(LINES-5, COLS-4, 3, 2);

	refresh();
	wrefresh(game_wrapper[1]);
	wrefresh(game_wrapper[0]);
	wrefresh(start_menu[1]);
	wrefresh(start_menu[0]);

	debug_window();

	Logger::write("[gui] Created GUI");
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

	debug_window();
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

	write_fleet_type("Your");

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

void Gui::get_win_size(WINDOW *w, int &width, int &height) {
	getmaxyx(w, height, width);
}

int Gui::menu_cursor(WINDOW *w, int x, int y, int noptions, string symbol, bool step_last) {
	string space;
	for (int i = 0; symbol[i]; i++) {
		space.push_back(' ');
	}

	int ch = 0;
	int select = 0;
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
		
	Logger::write("[gui] selected: " + to_string(select));
	return select;
}

void Gui::write_fleet_type(string who) {
	int width, height;
	get_win_size(sea_border[0], width, height);
	int w = width/2 - 5 < 0 ? 0 : width/2 - 5;
	if (Logger::debug) {
		w -= width/4;
	}
	mvwrite_on_window(sea_border[0], w, 0, string(who + " fleet"));
	wrefresh(sea_border[0]);
}

void Gui::paint_placement_sea() {
	int height, width;
	bool blue = false;
	int i, j, k;

	int **board = dummy->get_board()->get_board();
	Ship **ships = dummy->get_board()->get_ships();

	get_win_size(sea[0][0], width, height);
	
	// SEGMENTATION FAULT
	for (i = 0; i < BOARD_SIZE; i++) {
		for (j = 0; j < BOARD_SIZE; j++) {
			if (height > 1) {
				box(sea[i+1][j+1], ACS_VLINE, ACS_HLINE);
			}
			if (board[i][j] > 0) {
				if (board[i][j] == DAMAGE) {
					wbkgd(sea[i+1][j+1], COLOR_PAIR(COLOR_NOT_HIT));
				} else if (board[i][j] > DAMAGE) {
					for (k = 0; k < SHIPS_COUNT; k++) {
						if (ships[k]->point_intersect(j, i)) {
							break;
						}
					}
					if (ships[k]->is_sunk()) {
						wbkgd(sea[i+1][j+1], COLOR_PAIR(COLOR_SUNK));
					} else {
						wbkgd(sea[i+1][j+1], COLOR_PAIR(COLOR_HIT));
					}
				} else {
					wbkgd(sea[i+1][j+1], COLOR_PAIR(COLOR_SHIP));
				}
			} else {
				if (blue) {
					wbkgd(sea[i+1][j+1], COLOR_PAIR(COLOR_BLUE_TILE));
				} else {
					wbkgd(sea[i+1][j+1], COLOR_PAIR(COLOR_AQUA_TILE));
				}
			}
			blue = !blue;
		}
		if (BOARD_SIZE % 2 == 0) {
			blue = !blue;
		}
	}
	//-------

	for (int i = 0; i < BOARD_SIZE + 1; i++) {
		for (int j = 0; j < BOARD_SIZE + 1; j++) {
			wrefresh(sea[i][j]);
		}
	}
	write_fleet_type("Your");

	debug_window();
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

	debug_window();

	Logger::write("[gui] now in gamemode selection");

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

	debug_window();

	Logger::write("[gui] now in difficulty selection");

	return menu_cursor(start_menu[0], x/2 - 3, y/2 - 2, 4, ">", true);
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
			mvwrite_on_window(actions[0], width/2 - 4, height/2 - 2, "YOUR TURN");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2, "> Attack");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 1, "  See field");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 2, "  Forfeit");
			width = width/2 - 6 < 0 ? 0 : width/2 - 6;
			height = height/2;
			break;
		case GAME_WAITING_TURN:
			mvwrite_on_window(actions[0], width/2 - 8, height/2 - 2, "WAITING YOUR TURN");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2, "> See field");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 1, "  Forfeit");
			width = width/2 - 6 < 0 ? 0 : width/2 - 6;
			height = height/2;
			break;
		case GAME_SPECTATOR:
			mvwrite_on_window(actions[0], width/2 - 4, height/2 - 2, "SPECTATOR");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2, "> See field");
			mvwrite_on_window(actions[0], width/2 - 6 < 0 ? 0 : width/2 - 6, height/2 + 1, "  Forfeit");
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

	wrefresh(actions[1]);
	wrefresh(actions[0]);
	debug_window();
}

int Gui::actions_menu(enum action_e a) {
	int x, y;
	
	paint_actions_menu(a, x, y);

	switch (a) {
		case PLACE_SHIPS:
			return menu_cursor(actions[0], x, y, 7, ">", true);
			break;
		case GAME:
			return menu_cursor(actions[0], x, y, 3, ">", false);
			break;
		case GAME_WAITING_TURN:
			return menu_cursor(actions[0], x, y, 2, ">", false);
			break;
		case FORFEIT:
			return menu_cursor(actions[0], x, y, 2, ">", false);
			break;
		case GAME_SPECTATOR:
			// Implementation only in multiplayer
			return menu_cursor(actions[0], x, y, 2, ">", false);
			break;
		case ALLY:
			// Implementation only in multiplayer
			break;
		default:
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
	while (ch != '\n') {
		keypad(actions[0], TRUE);
		ch = wgetch(actions[0]);
		keypad(actions[0], FALSE);
		switch (ch) {
			case KEY_UP:
				b->insert_ship(index, MOVE_UP);
				paint_ship(index);
				break;
			case KEY_RIGHT:
				b->insert_ship(index, MOVE_RIGHT);
				paint_ship(index);
				break;
			case KEY_DOWN:
				b->insert_ship(index, MOVE_DOWN);
				paint_ship(index);
				break;
			case KEY_LEFT:
				b->insert_ship(index, MOVE_LEFT);
				paint_ship(index);
				break;
			case 'r':
				b->insert_ship(index, ROTATE);
				paint_ship(index);
				break;
			case 'c':
				paint_placement_sea();
				return -1;
				break;
		}
	}

	ch = b->insert_ship(index, PLACE);
	if (ch) {
		paint_ship(index);
	} else {
		paint_placement_sea();
	}

	return ch;
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
	wait_for(mng, ACK);
	recv = client->get_msg(ACK);

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
	wait_for(mng, MSG_MATCH_END);
	r_msg = client->get_msg(MSG_MATCH_END);

	if (msg != NULL) {
		*msg = r_msg;
	}
}

bool Gui::place_ships() {
	bool confirm = false;
	int choice, width, height, exit_loop;

	get_win_size(actions[0], width, height);

	paint_placement_sea();
	while (!confirm) {
		choice = actions_menu(PLACE_SHIPS);
		if (choice == 5) {
			if (dummy->get_board()->remaining_ships() < SHIPS_COUNT) {
				mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 7, "[Must place all ships!]");
				wrefresh(actions[0]);
				sleep(2);
				mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 7, "                       ");
				wrefresh(actions[0]);
			} else {
				if (!send_board()) {
					mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 7, "[Invalid placement!]");
					wrefresh(actions[0]);
					sleep(2);
					mvwrite_on_window(actions[0], width/2 - 12 < 0 ? 0 : width/2 - 12, height/2 + 7, "                    ");
					wrefresh(actions[0]);
				} else {
					confirm = true;
				}
			}
		} else if (choice == 6) {
			if (actions_menu(FORFEIT) == 0) {
				send_forfeit(NULL);
				return false;
			}
		} else {
			exit_loop = 0;
			while (!exit_loop) {
				exit_loop = place_a_ship(choice);
			}
		}
	}
	
	return true;
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

	debug_window();
}

void Gui::view_field(Player *defender) {
	Logger::write("[gui] Asking for " + defender->get_name() + " board");
	ask_board(defender, false);
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

	client->send_message(&c_msg);
	wait_for(mng, ACK_MSG_MATCH_ATTACK_STATUS);
	p_msg = client->get_msg(ACK_MSG_MATCH_ATTACK_STATUS);
	
	if (p_msg.msg_type != ACK_MSG_MATCH_ATTACK_STATUS) {
		return false;
	}

	if (p_msg.data.ack_match_attack_status.status == FAILED_ATTACK) {
		return false;
	}
	return true;
}

void Gui::ask_board(Player *p, bool lost) {
	msg_creation c_msg;
	msg_parsing r_msg;

	if (lost) {
		c_msg.msg_type = MSG_PLAYER_GET_BOARD_LOST;
	} else {
		c_msg.msg_type = MSG_PLAYER_GET_BOARD;
	}
	c_msg.data.player_get_board.id = p->get_id();

	client->send_message(&c_msg);
	wait_for(mng, ACK_MSG_GET_BOARD);
	r_msg = client->get_msg(ACK_MSG_GET_BOARD);

	int **matrix;
	matrix = p->get_board()->get_board();
	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			matrix[i][j] = r_msg.data.ack_get_board.board.matrix[i][j];
		}
	}
}

bool Gui::attack(Player *defender) {
	int x = 0;
	int y = 0;

	actions_menu(ATTACK);
	ask_board(defender, false);
	paint_enemy_sea(defender);
	paint_attack(defender->get_board()->get_board(), y, x);

	int **board = defender->get_board()->get_board();

	retry:
	int ch = 0;
	keypad(actions[0], true);
	while (ch != '\n') {
		ch = wgetch(actions[0]);
		switch (ch) {
			case 'w':
			case KEY_UP:
				if (y - 1 >= 0) {
					y--;
					paint_enemy_sea(defender);
					paint_attack(board, y, x);
				}
				break;
			case 'd':
			case KEY_RIGHT:
				if (x + 1 < BOARD_SIZE) {
					x++;
					paint_enemy_sea(defender);
					paint_attack(board, y, x);
				}
				break;
			case 's':
			case KEY_DOWN:
				if (y + 1 < BOARD_SIZE) {
					y++;
					paint_enemy_sea(defender);
					paint_attack(board, y, x);
				}
				break;
			case 'a':
			case KEY_LEFT:
				if (x - 1 >= 0) {
					x--;
					paint_enemy_sea(defender);
					paint_attack(board, y, x);
				}
				break;
			case 'q':
				keypad(actions[0], false);
				return false;
				break;
		}
	}
	keypad(actions[0], false);

	Logger::write("[gui] Trying attacking " + defender->get_name() + " [" + std::to_string(y) + ", " + std::to_string(x) + "]");
	if (!attack_at(defender, x, y)) {
		Logger::write("[gui] Attack failed");
		goto retry;
	}
	Logger::write("[gui] Attack successful");

	return true;
}

void Gui::make_actions() {
	int choice;
	if (dummy->his_turn()) {
		choice = actions_menu(GAME);
		switch (choice) {
			case 0:
				if (this->mode == SINGLEPLAYER) {
					Player *defender = p_list[1];
					if (!attack(defender)) {
						Logger::write("[gui] Aborted attacking" + defender->get_name());
					}
				} else if (this->mode == MULTIPLAYER) {
					// player selection -> attack del player
				}
				break;
			case 1:
				if (this->mode == SINGLEPLAYER) {
					Player *defender = p_list[1];
					actions_menu(SEE_FIELD);
					view_field(defender);
					getch();
				} else if (this->mode == MULTIPLAYER) {
					// player selection -> see_field del player
				}
				break;
			case 2:
				if (actions_menu(FORFEIT) == 0) {
					msg_parsing r_msg;
					send_forfeit(&r_msg);
					end_game_win(&r_msg);
				}
				break;
		}
	} else {
		choice = actions_menu(GAME_WAITING_TURN);
		switch (choice) {
			case 0:
				if (this->mode == SINGLEPLAYER) {
					Player *defender = p_list[1];
					actions_menu(SEE_FIELD);
					view_field(defender);
					getch();
				} else if (this->mode == MULTIPLAYER) {
					// player selection -> see_field del player
				}
				break;
			case 1:
				if (actions_menu(FORFEIT) == 0) {
					msg_parsing r_msg;
					send_forfeit(&r_msg);
					end_game_win(&r_msg);
				}
				break;
		}
	}
}

void Gui::make_actions_spectator() {
	int choice = actions_menu(GAME_SPECTATOR);
	switch (choice) {
		case 0:
			// view field
			break;
		case 1:
			// quit (forfeit)
			break;
	}
}

int Gui::pregame() {
	int value = game_menu();
    if (value == 2) {
        return 0;
    }

    if (value == SINGLEPLAYER) {
		this->mode = SINGLEPLAYER;

        value = diff_menu();
        if (value == 3) {
            return 2;
        }

		dummy = new Player();
		creating_server_win();
		client->create_server();
		usleep(500000);
		
		if (!client->connect_to_server(SERVER_IP, SERVER_PORT)) {
			delete dummy;
			Logger::write("[client][ERROR] " + client->get_error());
			client->stop_server();
			conn_err(NULL);
			return 2;
		}

		if (!init_singleplayer_game((enum game_difficulty_e)value, 1)) {
			delete dummy;
			Logger::write("[client][ERROR] " + client->get_error());
			client->stop_server();
			return 2;
		}
    } else if (value == MULTIPLAYER) {
		return 2; // da rimuore
		this->mode = MULTIPLAYER;
        // ...
    }
	game_starting();

	usleep(500000);
	init_game_windows();

	return 1;
}

bool Gui::init_singleplayer_game(enum game_difficulty_e diff, int num_ai) {
	msg_creation msg;
	msg_parsing recv;

	msg.msg_type = MSG_PLAYER_GET_OWN_ID;
	msg.data.player_get_own_id.username = string(DEFAULT_PLAYER_NAME);
	client->send_message(&msg);
	wait_for(mng, ACK_MSG_PLAYER_GET_OWN_ID);
	recv = client->get_msg(ACK_MSG_PLAYER_GET_OWN_ID);
	if (recv.msg_type != ACK_MSG_PLAYER_GET_OWN_ID) {
		return false;
	}
	dummy->set_name(string(DEFAULT_PLAYER_NAME));
	dummy->set_id(recv.data.ack_player_get_own_id.player_id);
	dummy->set_diff(diff);

	msg.msg_type = MSG_HOST_INIT_MATCH;
	msg.data.host_init_match.difficulty = diff;
	msg.data.host_init_match.ais = num_ai;
	client->send_message(&msg);
	wait_for(mng, ACK_MSG_MATCH_INIT_MATCH);
	recv = client->get_msg(ACK_MSG_MATCH_INIT_MATCH);
	if (recv.msg_type != ACK_MSG_MATCH_INIT_MATCH || recv.data.ack_match_init_match.status != GS_OK) {
		return false;
	}

	msg.msg_type = MSG_HOST_START_MATCH;
	client->send_message(&msg);
	wait_for(mng, ACK);
	recv = client->get_msg(ACK);
	if (recv.msg_type != ACK) {
		return false;
	}

	return true;
}

void Gui::game_starting() {
	box(start_menu[1], ACS_VLINE, ACS_HLINE);
	int width, height;
	get_win_size(start_menu[0], width, height);
	
	std::string temp = "Starting match.......";
	mvwrite_on_window(start_menu[0], width/2 - temp.length()/2, height/2, temp);

	wrefresh(start_menu[1]);
	wrefresh(start_menu[0]);
	debug_window();
	usleep(1000000);
}

void Gui::creating_server_win() {
	box(start_menu[1], ACS_VLINE, ACS_HLINE);
	int width, height;
	get_win_size(start_menu[0], width, height);
	
	std::string temp = "Creating server......";
	mvwrite_on_window(start_menu[0], width/2 - temp.length()/2, height/2, temp);

	wrefresh(start_menu[1]);
	wrefresh(start_menu[0]);
	debug_window();
}

void Gui::turn(bool turn) {
	dummy->set_turn(turn);
	int x, y;
	if (turn) {
		paint_actions_menu(GAME, x, y);
	} else {
		paint_actions_menu(GAME_WAITING_TURN, x, y);
	}
}

void Gui::set_new_board(msg_parsing *msg) {
	Board *b = dummy->get_board();
	Ship **ships = b->get_ships();
	int **matrix = b->get_board();

	for (int i = 0; i < SHIPS_COUNT; i++) {
		ships[i]->reset_hits();
	}

	for (int i = 0; i < BOARD_SIZE; i++) {
		for (int j = 0; j < BOARD_SIZE; j++) {
			switch (msg->data.match_new_board.board.matrix[i][j]) {
				case COLOR_SHIP:
					matrix[i][j] = 1;
					break;
				case COLOR_HIT:
					matrix[i][j] = 1 + DAMAGE;
					break;
				case COLOR_NOT_HIT:
					matrix[i][j] = DAMAGE;
					break;
				case COLOR_SUNK:
					for (int k = 0; k < SHIPS_COUNT; k++) {
						if (ships[k]->point_intersect(j, i)) {
							ships[k]->add_hit();
						}
					}
					break;
				default:
					matrix[i][j] = 0;
					break;
			}
		}
	}

	/*
	if (giocatore nella schermata del suo campo) {
		paint_placement_sea();
	}
	*/
}

void Gui::end_game_win(msg_parsing *msg) {
	box(start_menu[1], ACS_VLINE, ACS_HLINE);
	int width, height;
	get_win_size(start_menu[0], width, height);

	struct stats_t *stats;
	if (msg->msg_type == MSG_MATCH_WIN) {
		mvwrite_on_window(start_menu[0], width/2 - 3, height/2 - 3, "YOU WON!");
		stats = &msg->data.match_win.info;
	} else if (msg->msg_type == MSG_MATCH_LOSE) {
		mvwrite_on_window(start_menu[0], width/2 - 3, height/2 - 3, "YOU LOST!");
		stats = &msg->data.match_lose.info;
	} else if (msg->msg_type == MSG_MATCH_END) {
		mvwrite_on_window(start_menu[0], width/2 - 3, height/2 - 3, "GAME END!");
		stats = &msg->data.match_end.info;
	} else if (msg->msg_type == ACK_MSG_MATCH_END) {
		mvwrite_on_window(start_menu[0], width/2 - 3, height/2 - 3, "GAME END!");
		stats = &msg->data.ack_match_end.info;
	} else {
		return;
	}

	mvwrite_on_window(start_menu[0], width/2 - 8, height/2 - 1, "Total shots: " + to_string(stats->hits + stats->missed));
	mvwrite_on_window(start_menu[0], width/2 - 8, height/2, "Shots missed: " + to_string(stats->missed));
	mvwrite_on_window(start_menu[0], width/2 - 8, height/2 + 1, "Hits: " + to_string(stats->hits));
	mvwrite_on_window(start_menu[0], width/2 - 8, height/2 + 2, "Match time [" + stats->duration + "]");
	mvwrite_on_window(start_menu[0], width/2 - 8, height/2 + 3, "Grade: " + string(1, stats->grade));
	
	mvwrite_on_window(start_menu[0], width/2 - 13, height/2 + 5, "[press any key to continue]");
	
	wrefresh(start_menu[1]);
	wrefresh(start_menu[0]);
	debug_window();
	getch();
}

void Gui::conn_err(msg_parsing *msg) {
	box(start_menu[1], ACS_VLINE, ACS_HLINE);
	int width, height;
	get_win_size(start_menu[0], width, height);

	std::string temp = "CONNECTION ERROR";
	mvwrite_on_window(start_menu[0], width/2 - temp.length()/2, height/2 - 3, temp);
	if (msg != NULL) {
		switch (msg->msg_type) {
			case MSG_CONN_ERR:
				temp = "Connection error, try again later";
				mvwrite_on_window(start_menu[0], width/2 - temp.length()/2, height/2, temp);
				break;
			case MSG_CONN_MATCH_STARTED:
				temp = "Can't connect, match already started";
				mvwrite_on_window(start_menu[0], width/2 - temp.length()/2, height/2, temp);
				break;
			case MSG_CONN_SERVER_FULL:
				temp = "Can't connect, server is full";
				mvwrite_on_window(start_menu[0], width/2 - temp.length()/2, height/2, temp);
				break;
			default:
				break;
		}
	} else {
		temp = "Connection error, try again later";
		mvwrite_on_window(start_menu[0], width/2 - temp.length()/2, height/2, temp);
	}

	mvwrite_on_window(start_menu[0], width/2 - 13, height/2 + 5, "[press any key to continue]");
	
	wrefresh(start_menu[1]);
	wrefresh(start_menu[0]);
	debug_window();
	getch();
}

void Gui::got_kicked(msg_parsing *msg) {
	box(start_menu[1], ACS_VLINE, ACS_HLINE);
	int width, height;
	get_win_size(start_menu[0], width, height);

	std::string temp = "KICKED BY HOST";
	mvwrite_on_window(start_menu[0], width/2 - temp.length()/2, height/2 - 3, temp);
	temp = "Reason: " + msg->data.match_got_kicked.reason;
	mvwrite_on_window(start_menu[0], width/2 - temp.length()/2, height/2, temp);

	mvwrite_on_window(start_menu[0], width/2 - 13, height/2 + 5, "[press any key to continue]");
	
	wrefresh(start_menu[1]);
	wrefresh(start_menu[0]);
	debug_window();
	getch();
}

std::map<int, Player*> *Gui::get_player_list() {
	return &this->p_list;
}

void Gui::set_player_list(std::map<int, Player*> player_list) {
	this->p_list = player_list;
}

void Gui::start_game() {
	place_ships();
}

/************************************************/
/*                 SERVER MENUS                 */
/************************************************/
void Gui::wait_conn_menu() {
	// TO-DO
}

int Gui::multi_menu() {
	/*int x, y;
	get_win_size(start_menu[0], x, y);
	wclear(start_menu[0]);
	mvwrite_on_window(start_menu[0], x/2 - 5, y/2 - 4, "MULTIPLAYER");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 2, "> Host a match");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 1, "  Join a match");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2, "  Back");
	wrefresh(start_menu[0]);

	return menu_cursor(start_menu[0], x/2 - 7, y/2 - 2, 4, ">", true);*/
	return 0;
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