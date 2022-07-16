#include <gui.hpp>
#include <lang.hpp>
#include <debug.hpp>

#include <string>
#include <cmath>

using std::to_string;

Gui::Gui() {
	init_gui();
}

Gui::~Gui() {
	clear();
	endwin();
	del_array_win(game_wrapper, 2);
	del_array_win(start_menu, 2);
	del_array_win(my_grid, 2);
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

	curs_set(0);

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

	my_grid[1] = newwin(line, col + 1, 2, 1);
	box(my_grid[1], ACS_VLINE, ACS_HLINE);

	my_grid[0] = newwin(line - 2, col - 1, 3, 2);

	wrefresh(my_grid[1]);
	wrefresh(my_grid[0]);

	// SEA
	init_sea();

	actions_border = newwin(line, (COLS * (100 - PERCENT_SEA) / 100) - 3 + (COLS - 2 - (col + 1 + (COLS * (100 - PERCENT_SEA) / 100) - 3)), 2, 2 + col);
	box(actions_border, ACS_VLINE, ACS_HLINE);

	wrefresh(actions_border);
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

	char col_name[] = "A";
	int row_name = 1;
	bool blue = true;

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
					mvwrite_on_window(sea[i][j], x_inc/2, y_inc/2, string(col_name));
					col_name[0]++;
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
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2, "  Exit");
	wrefresh(start_menu[0]);

	return menu_cursor(start_menu[0], x/2 - 7, y/2 - 2, 3, ">");
}

int Gui::menu_cursor(WINDOW *w, int x, int y, int noptions, string symbol) {
	keypad(w, TRUE);

	string space;
	for (char c : symbol) {
		space.push_back(' ');
	}

	int ch = 0, select = 0;
	while (ch != '\n') {
		ch = wgetch(w);
		switch (ch) {
			case KEY_UP:
				if (select > 0) {
					mvwrite_on_window(w, x, y + select - 1, symbol);
					mvwrite_on_window(w, x, y + select, space);
					wrefresh(w);
					select--;
				}
				break;
			case KEY_DOWN:
				if (select < noptions - 1) {
					mvwrite_on_window(w, x, y + select, space);
					mvwrite_on_window(w, x, y + select + 1, symbol);
					wrefresh(w);
					select++;
				}
				break;
			default: break;
		}
	}

	keypad(w, FALSE);

	return select;
}

void Gui::paint_sea() {
	int height, width;
	get_win_size(sea[0][0], width, height);

	bool blue = true;
	for (int i = 1; i < BOARD_SIZE + 1; i++) {
		for (int j = 1; j < BOARD_SIZE + 1; j++) {
			if (height > 1) {
				box(sea[i][j], ACS_VLINE, ACS_HLINE);
			}
			if (blue) {
				wbkgd(sea[i][j], COLOR_PAIR(COLOR_BLUE_TILE));
			} else {
				wbkgd(sea[i][j], COLOR_PAIR(COLOR_AQUA_TILE));
			}
			blue = !blue;
		}
		blue = !blue;
	}
}

void Gui::place_ships() {
	//paint_grid()
	wrefresh(my_grid[0]);
}

void Gui::start() {
	int choice = game_menu();

	if (choice == 2) {
		return;
	}

	choice = 0;

	init_game_windows();
	
	m = new Match((enum gamemode)choice);
	
	getch();
}

