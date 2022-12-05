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
		Match(enum gamemode_e e);
		~Match();

		bool add_player(Player *p);
		bool remove_player(Player *p);
		bool remove_player(int id);

		static void set_time(time_t &time);
		string get_duration();

		void start_match();

		void set_mode(enum gamemode_e mode);
		enum gamemode_e get_mode();
		void set_difficulty(enum game_difficulty_e diff);
		enum game_difficulty_e get_difficulty();
		void reset_match();
		enum game_status_e get_status();
		void set_status(enum game_status_e new_status);
};

#endif