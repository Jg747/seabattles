#ifndef __player_h__
#define __player_h__

#include <player.hpp>
#include <board.hpp>
#include <match.hpp>

#include <vector>

class Player {
	protected:
		string name;
		int player_id;
		bool ai;
		std::vector<struct attacked_player> attacks;
		bool can_attack;

		enum game_difficulty_e diff;

		Board *b;
		int missed_shots;
		int hit_shots;
		int sunk_ships;
		int own_sunk_ships;

		bool winner;
		bool loser;

		bool host;

		inline static int id;

		struct attacked_player *get_attack_by_id(int id);
	
	public:
		Player(enum game_difficulty_e e);
		Player(enum game_difficulty_e e, string name);
		~Player();

		static void set_id(int start);

		Board *get_board();
		void reset_player();
		enum grade_e get_grade();
		string get_name();
		void set_name(string name);
		int get_id();
		void set_can_attack(bool state);
		bool his_turn();
		void add_player_to_attack(Player &p);

		bool is_winner();
		void set_winner(bool state);
		bool is_loser();
		void set_loser(bool state);

		int get_hits();
		int get_misses();
		int get_sunk_ships();
		int remaining_ships();
		void dec_remaining_ships();
		void inc_hits();
		void inc_misses();
		void inc_sunk_ships();


		bool is_ai();
		void set_ai(bool state);
		void ai_attack(Player &p);
		void reset_ai_atk(Player &p);

		bool is_host();

		string get_info();
		string attacks_to_string();
};

#endif