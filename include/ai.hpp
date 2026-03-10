#ifndef __ai_h__
#define __ai_h__

#include <player.hpp>

class Ai : public Player {
    public:
        Ai(int number, enum game_difficulty_e difficulty);

        void ai_attack(Player *p);
		void ai_place_ships();
		void reset_ai_atk(Player *p);
};

#endif