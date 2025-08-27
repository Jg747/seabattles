#ifndef __gui_h__
#define __gui_h__

#include <string>
#include <list>
#include <map>
#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif

#include <client.hpp>
#include <player.hpp>
#include <common.hpp>
#include <thread_manager.hpp>

using std::string;

#define PERCENT_SEA 75
#define PERCENT_DEBUG 20

#define CORRECT(x, b) (x < 0 ? 0 : (x > b - 1 ? b - 1 : x))
#define CORRECT_W(x) CORRECT(x, COLS)
#define CORRECT_H(y) CORRECT(y, LINES)

enum colors {
	COLOR_SHIP = 0x01,				// COLOR_WHITE COLOR_WHITE
	COLOR_BLUE_TILE = 0x02,			// COLOR_BLUE COLOR_BLUE
	COLOR_AQUA_TILE = 0x03,			// COLOR_CYAN COLOR_CYAN
	COLOR_NOT_HIT = 0x04,			// COLOR_RED COLOR_RED
	COLOR_HIT = 0x05,				// COLOR_YELLOW, COLOR_YELLOW
	COLOR_SELECT_PLACE = 0x06,		// COLOR_GREEN COLOR_GREEN
	COLOR_SELECT_HIT = 0x07,		// COLOR_MAGENTA COLOR_MAGENTA
	
	COLOR_SUNK = 0x06, 				// COLOR_GREEN COLOR_GREEN
	COLOR_ALREADY_HIT = 0x04, 		// COLOR_RED COLOR_RED
	COLOR_CORRECT_PLACE = 0x06, 	// COLOR_GREEN COLOR_GREEN
	COLOR_INCORRECT_PLACE = 0x04, 	// COLOR_RED COLOR_RED
};

class Client;

class Gui {
	public:
		Gui(Client *c, struct thread_manager_t *mng);
		~Gui();
		
		std::map<int, Player*> *get_player_list();
		void set_player_list(std::map<int, Player*> player_list);

		int pregame();
		void start_game();

		void game_starting();
		void turn(bool turn);
		void set_new_board(msg_parsing *msg);
		void conn_err(msg_parsing *msg);
		void got_kicked(msg_parsing *msg);
		void end_game_win(msg_parsing *msg);

		void stop_game(bool state);
	
	private:
		WINDOW *game_wrapper[2];
		WINDOW *start_menu[2];

		WINDOW *sea_border[2];
		WINDOW *sea[BOARD_SIZE + 1][BOARD_SIZE + 1];
		WINDOW *actions[2];

		WINDOW *debug_win[2];

		enum gamemode_e mode;

		Client *client;
		Player *dummy;
		std::map<int, Player*> p_list;
		bool stop;

		struct thread_manager_t *mng;

		void debug_window();
		void reset_start_menu();

		void del_array_win(WINDOW *array[], int len);
		void init_gui();
		void init_game_windows();
		void init_sea();

		void get_win_size(WINDOW *w, int &width, int &height);
		void write_on_window(WINDOW *w, string str);
		void mvwrite_on_window(WINDOW *w, int x, int y, string str);
		string get_input(WINDOW *w);
		int menu_cursor(WINDOW *w, int x, int y, int noptions, string symbol, bool step_last);

		bool init_singleplayer_game(enum game_difficulty_e diff, int num_ai);
		void wait_turn_packet();

		int game_menu();
		int diff_menu();
		int actions_menu(enum action_e a);

		void ask_board(Player *p, bool lost);
		bool send_board();
		bool place_ships();
		int place_a_ship(int index);
		void paint_ship(int index);

		void paint_enemy_sea(Player *defender);
		void paint_placement_sea();
		
		void send_forfeit(msg_parsing *msg);
		void make_actions();
		void make_actions_spectator();

		void view_field(Player *defender);
		bool attack_at(Player *defender, int x, int y);
		void paint_attack(int **board, int x, int y);
		bool attack(Player *defender);
		
		void write_fleet_type(string who);
		void paint_actions_menu(enum action_e a, int &width, int &height);
		void color_tile(int i, int j, enum colors color);

		void creating_server_win();

		int multi_menu();
		void wait_conn_menu();
		bool join_menu();
		void waiting_host();
};

#endif