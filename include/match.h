#ifndef __match_h__
#define __match_h__

#include <common.h>
#include <ship.h>

class Match {
	public:
		Match(enum gamemode mode);
		~Match();
		
		bool insert_ship(int index, enum command_e cmd);
	
		int board[BOARD_SIZE][BOARD_SIZE];
		Ship *ships[SHIPS_COUNT];
		
	private:
		enum gamemode mode;
		unsigned char ships_remaining;
		char last_id;

		void reset_board();
		bool insert_on_board(Ship *&ship);
		void remove_from_board(Ship *&ship);
		bool check_intersection(Ship *&ship);
		void assign_ids();
};

#endif