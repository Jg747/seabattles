#ifndef __common_h__
#define __common_h__

#include <string>

#define SERVER_PORT 42069
#define SERVER_IP "127.0.0.1"

#define MAX_CLIENTS 8
#define RECV_BUF_LEN (1000+1)

#define DEFAULT_PLAYER_NAME "player"

#define BOARD_SIZE 10

#define SHIPS_COUNT 5
#define SHOTS_TO_WIN 5 + 4 + 3 + 3 + 2

#define DAMAGE 0xFFFFFF

extern const int ships_len[];

extern const char *GENERIC_STATUS_STR[];
#define GENERIC_STATUS_STR_LEN 2

enum gamemode_e {
	SINGLEPLAYER,
	MULTIPLAYER
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

enum command_e {
	MOVE_UP,
	MOVE_RIGHT,
	MOVE_DOWN,
	MOVE_LEFT,
	ROTATE,
	PLACE,
	REMOVE
};

enum action_e {
	PLACE_SHIPS,
	PLACE_A_SHIP,
	GAME,
	GAME_WAITING_TURN,
	GAME_SPECTATOR,
	SEE_FIELD,
	ALLY,
	ATTACK,
	FORFEIT
};

enum game_status_e {
	NOT_RUNNING,
	RUNNING
};

enum player_status_e {
	WIN,        // player has won
	LOSE,       // player has lost
	WAITING,    // player is waiting (in his board)
	ATTACKING,  // player is attacking an enemy
	QUIT,       // player has quit
	LOOKING     // player is looking enemy board
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

struct player_info {
	std::string name;
	int player_id;
};

enum attack_status_e {
	FAILED_ATTACK,
	MISSED,
	HIT,
	HIT_SUNK,

	NOT_YOUR_TURN,
	NOT_SAME_PLAYER,
	DEAD_CANNOT_ATTACK,
	INVALID_ATTACK
};

enum generic_status_e {
	GS_OK,
	GS_ERROR
};

class Player;

struct attacked_player {
	Player *defender;
	bool attacked;
	struct ai_last_atk atk;
};

#endif