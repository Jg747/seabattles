#include <gui.h>
#include <lang.h>
#include <debug.h>

Gui::Gui() {
	init_gui();
}

Gui::~Gui() {
	clear();
	endwin();
	del_array_win(game_wrapper);
	del_array_win(start_menu);
}

void Gui::del_array_win(WINDOW* array[]) {
	for (int i = 0; i < 2; i++) {
		if (array[i] != NULL) {
			delwin(array[i]);
		}
	}
}

void Gui::init_gui() {
	initscr();

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

	keypad(start_menu[0], TRUE);

	refresh();
	wrefresh(game_wrapper[1]);
	wrefresh(game_wrapper[0]);
	wrefresh(start_menu[1]);
	wrefresh(start_menu[0]);
}

void Gui::init_game_windows() {
	//Borders
	my_grid[1] = newwin(BOARD_SIZE*2 + 1, (BOARD_SIZE*6), 2, 1);
	box(my_grid[1], ACS_VLINE, ACS_HLINE);

	//Title
	my_grid[0] = newwin(BOARD_SIZE*2 - 1, BOARD_SIZE*6 - 2, 3, 2);

	keypad(my_grid[0], TRUE);

	wrefresh(my_grid[1]);
	wrefresh(my_grid[0]);
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

void Gui::get_win_size(WINDOW *w, int &x, int &y) {
	getmaxyx(w, y, x);
}

int Gui::game_menu() {
	int x, y;
	get_win_size(start_menu[0], x, y);
	wclear(start_menu[0]);
	mvwrite_on_window(start_menu[0], x/2 - 2, y/2 - 4, "MENU");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 2, "> Singleplayer");
	mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 1, "  Multiplayer");
	wrefresh(start_menu[0]);

	int ch = 0, select = 0;
	while (ch != '\n') {
		ch = wgetch(start_menu[0]);
		DEBUG_WRITE_LOG("X: %d, Y: %d", x, y)
		switch (ch) {
			case KEY_UP:
				if (select > 0) {
					mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 2, ">");
					mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 1, " ");
					wrefresh(start_menu[0]);
					select--;
				}
				break;
			case KEY_DOWN:
				if (select < 1) {
					mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 2, " ");
					mvwrite_on_window(start_menu[0], x/2 - 7, y/2 - 1, ">");
					wrefresh(start_menu[0]);
					select++;
				}
				break;
			default: break;
		}
	}

	return select;
}

void Gui::paint_grid(WINDOW *w) {
	int rows, cols;
	get_win_size(w, cols, rows);
	
}

void Gui::place_ships() {
	paint_grid(my_grid[0]);
	wrefresh(my_grid[0]);
}

void Gui::start() {
	int choice = game_menu();
	choice = 0;

	init_game_windows();
	
	m = new Match((enum gamemode)choice);
	
	getch();
}

