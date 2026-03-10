#ifndef __board_h__
#define __board_h__

#include <ship.hpp>
#include <common.hpp>

class Board {
	private:
		int **board;
		Ship **ships;
		char last_id;
		unsigned char ships_remaining;

		void assign_ids();

	public:
		Board();
		~Board();
		void reset_board();
		bool insert_on_board(Ship *&ship);
		void remove_from_board(Ship *&ship);
		bool is_hit(int x, int y);
		bool check_intersection(Ship *&ship);
		bool insert_ship(int index, enum command_e cmd);
		int remaining_ships();

		int **get_board();
		Ship **get_ships();

		string get_info();
};

#endif