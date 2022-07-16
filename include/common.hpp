#ifndef __common_h__
#define __common_h__

#define BOARD_SIZE 10
#define SHIPS_COUNT 5
#define DAMAGE 0xFFFFFF

extern const int ships_len[];

enum gamemode {
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
	QUITTING
};

#endif