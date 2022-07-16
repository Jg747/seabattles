#ifndef __gui_h__
#define __gui_h__

#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif

#include <string>

using std::string;

#include <match.hpp>
#include <common.hpp>

#define PERCENT_SEA 75

enum colors {
	COLOR_SHIP = 0x01,
	COLOR_BLUE_TILE = 0x02,
	COLOR_AQUA_TILE = 0x03,
	COLOR_SELECT_PLACE = 0x04,
	COLOR_SELECT_HIT = 0x05,
	COLOR_HIT = 0x06,
	COLOR_NOT_HIT = 0x07
};

class Gui {
	public:
		Gui();
		~Gui();
		void start();
	
	private:
		WINDOW *game_wrapper[2];
		WINDOW *start_menu[2];

		WINDOW *my_grid[2];
		WINDOW *enemy_grid[2];
		WINDOW *sea[BOARD_SIZE + 1][BOARD_SIZE + 1];

		WINDOW *actions_border;
		WINDOW *actions[2];
		WINDOW *submenu[2];

		Match *m;

		void del_array_win(WINDOW *array[], int len);
		void init_gui();
		void init_game_windows();
		void init_sea();

		int game_menu();
		void place_ships();

		void get_win_size(WINDOW *w, int &width, int &height);
		void write_on_window(WINDOW *w, string str);
		void mvwrite_on_window(WINDOW *w, int x, int y, string str);
		string get_input(WINDOW *w);
		int menu_cursor(WINDOW *w, int x, int y, int noptions, string symbol);
		void paint_sea();
};

#endif