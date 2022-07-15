#ifndef __gui_h__
#define __gui_h__

#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif

#include <string>

using std::string;

#include <match.h>
#include <common.h>

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
		WINDOW *actions[2];
		WINDOW *submenu[2];

		Match *m;

		void del_array_win(WINDOW *array[]);
		void init_gui();
		void init_game_windows();

		int game_menu();
		void place_ships();

		void get_win_size(WINDOW *w, int &x, int &y);
		void write_on_window(WINDOW *w, string str);
		void mvwrite_on_window(WINDOW *w, int x, int y, string str);
		string get_input(WINDOW *w);
		void paint_grid(WINDOW *w);
};

#endif