#ifndef __match_h__
#define __match_h__

#include <common.hpp>
#include <ship.hpp>

#include <chrono>

class Match {
	public:
		Match(enum gamemode mode);
		~Match();
		void reset(enum gamemode g);
		static void set_time(time_t &time);
		string get_duration();
		enum grade_e get_grade();
		
		bool insert_ship(int index, enum command_e cmd);
		int remaining_ships();
		bool check_intersection(Ship *&ship, bool my_board);

		// Singleplayer
		void generate_match();
		void ai_attack();
	
		int board[BOARD_SIZE][BOARD_SIZE];
		int enemy_board[BOARD_SIZE][BOARD_SIZE];
		Ship *ships[SHIPS_COUNT];
		Ship *enemy[SHIPS_COUNT];

		// Stats
		enum game_status_e status;
		int ai_hits; 				// Needed to check if end match
		int missed_shots;
		int hit_shots;
		time_t start_time;
		time_t end_time;
		
		int player_eliminations;	// Multiplayer only
		
	private:
		enum gamemode mode;
		unsigned char ships_remaining;
		char last_id;

		void reset_board();
		bool insert_on_board(Ship *&ship, bool my_board);
		void remove_from_board(Ship *&ship);
		void assign_ids();
		bool is_hit(int x, int y);
};

#endif