#ifndef __match_h__
#define __match_h__

#include <vector>
#include <string>

#include <common.hpp>
#include <player.hpp>

using std::string;

class Match {
	private:
		enum gamemode_e mode;
		enum game_difficulty_e difficulty;
		enum game_status_e status;

		std::vector<Player*> *players;
		int player_eliminations;

	public:
		time_t start_time;
		time_t end_time;

		Match();
		~Match();

		bool add_player(Player *p);
		bool remove_player(Player *p);
		bool remove_player(int id);
		void next_turn();

		std::vector<Player*> *get_players();
		Player *get_player_by_id(int id);

		bool all_attacked(Player *p);
		bool is_winner(Player *p);
		bool eliminated(Player *p);

		static void set_time(time_t &time);
		static string get_duration(time_t start, time_t end);

		time_t get_start_time();

		void start_match();
		bool can_start();

		void set_mode(enum gamemode_e mode);
		enum gamemode_e get_mode();
		void set_difficulty(enum game_difficulty_e diff);
		enum game_difficulty_e get_difficulty();
		enum game_status_e get_status();
		void set_status(enum game_status_e new_status);
};

#endif