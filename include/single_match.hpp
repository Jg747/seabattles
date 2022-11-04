#ifndef __single_match_h__
#define __single_match_h__

#include <common.hpp>
#include <player.hpp>
#include <match.hpp>

#include <chrono>

class SingleMatch : public Match {
	private:
		Player *p;
		Player *ai;
	public:
		SingleMatch();
		SingleMatch(enum game_difficulty_e diff);
		~SingleMatch();

		void generate_match();
		void reset_match();

		Player *get_player();
		Player *get_ai();
};

#endif