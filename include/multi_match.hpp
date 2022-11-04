#ifndef __multi_match_hpp__
#define __multi_match_hpp__

#include <match.hpp>
#include <server.hpp>
#include <player.hpp>
#include <vector>

class MultiMatch : public Match {
	private:
		Server *serv;
		std::vector<Player> players;
		int player_eliminations;

	public:
		MultiMatch();
		~MultiMatch();

		std::vector<Player> *get_players();
		bool check_connection(string str);
};

#endif