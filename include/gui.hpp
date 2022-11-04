#ifndef __gui_h__
#define __gui_h__

#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif

#include <string>

using std::string;

#include <single_match.hpp>
#include <multi_match.hpp>
#include <common.hpp>

#define PERCENT_SEA 75

#define CORRECT(x, b) (x < 0 ? 0 : (x > b - 1 ? b - 1 : x))
#define CORRECT_W(x) CORRECT(x, COLS)
#define CORRECT_H(y) CORRECT(y, LINES)

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
		bool start();
	
	private:
		WINDOW *game_wrapper[2];
		WINDOW *start_menu[2];

		WINDOW *sea_border[2];
		WINDOW *sea[BOARD_SIZE + 1][BOARD_SIZE + 1];
		WINDOW *actions[2];

		Match *m;

		void del_array_win(WINDOW *array[], int len);
		void init_gui();
		void init_game_windows();
		void init_sea();

		bool singleplayer();
		bool multiplayer();

		int game_menu();
		int diff_menu();
		int actions_menu(enum action_e a);
		bool place_ships(Player *p);
		int place_a_ship(Player *p, int index);
		enum game_status_e make_actions(Player &attacker, Player &defender);
		int attack(Player &attacker, Player &defender);
		
		int multi_menu();
		void wait_conn_menu();
		bool join_menu();
		void waiting_host();

		void get_win_size(WINDOW *w, int &width, int &height);
		void write_on_window(WINDOW *w, string str);
		void mvwrite_on_window(WINDOW *w, int x, int y, string str);
		string get_input(WINDOW *w);
		int menu_cursor(WINDOW *w, int x, int y, int noptions, string symbol, bool step_last);
		
		void write_fleet_type(string who);
		void paint_sea(Board &p_board);
		void paint_enemy_sea(Player &p);
		void paint_actions_menu(enum action_e a, int &width, int &height);
		void paint_ship(Player &p, int index);
		void color_tile(int i, int j, enum colors color);
		void paint_attack(Board *b, int x, int y);
};

#endif