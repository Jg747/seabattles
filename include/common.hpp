#ifndef __common_h__
#define __common_h__

#define SERVER_PORT 10747
#define SERVER_IP "127.0.0.1"

#define BOARD_SIZE 10

#define SHIPS_COUNT 5
#define SHOTS_TO_WIN 5 + 4 + 3 + 3 + 2

#define DAMAGE 0xFFFFFF

extern const int ships_len[];

enum gamemode_e {
	SINGLEPLAYER,
	MULTYPLAYER
};

enum ship_e {
	CARRIER = 0x00,
	BATTLESHIP = 0x01,
	DESTROYER = 0x02,
	SUBMARINE = 0x03,
	PATROL = 0x04
};

enum rotation_e {
	UP = 0x00,
	RIGHT = 0x01,
	DOWN = 0x02,
	LEFT = 0x03
};

enum action_e {
	PLACE_SHIPS,
	PLACE_A_SHIP,
	GAME,
	SEE_FIELD,
	ALLY,
	ATTACK,
	FORFEIT
};

enum command_e {
	MOVE_UP,
	MOVE_RIGHT,
	MOVE_DOWN,
	MOVE_LEFT,
	ROTATE,
	PLACE,
	REMOVE
};

enum game_status_e {
	WIN,
	LOSE,
	PROGRESS,
	WAITING,
	QUITTING,
	NO_ATK
};

enum grade_e {
	S = 'S',
	A = 'A',
	B = 'B',
	C = 'C',
	D = 'D'
};

enum game_difficulty_e {
	NORMAL,
	HARD,
	IMPOSSIBLE
};

struct ai_last_atk {
	int x;
	int y;
	enum rotation_e direction;
	int back_x;
	int back_y;
	enum rotation_e back_direction;
};

struct attacked_player {
	int player_id;
	bool attacked;
	struct ai_last_atk atk;
};

#endif