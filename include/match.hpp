#ifndef __match_h__
#define __match_h__

#include <common.hpp>
#include <ship.hpp>

class Match {
	public:
		Match(enum gamemode mode);
		~Match();
		
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
		int ai_hits; 			// Needed to check if end match
		int missed_shots;
		int hit_shots;
		// player eliminations
		// Match start time
		/*
		#include <iostream>
		#include <chrono>
		#include <ctime>    

		int main() {
			auto start = std::chrono::system_clock::now();
			// Some computation here
			auto end = std::chrono::system_clock::now();
		
			std::chrono::duration<double> elapsed_seconds = end-start;
			std::time_t end_time = std::chrono::system_clock::to_time_t(end);
		
			std::cout << "finished computation at " << std::ctime(&end_time)
					<< "elapsed time: " << elapsed_seconds.count() << "s" << 
					<< std::endl;
		}
		*/
		
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