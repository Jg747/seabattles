#ifndef __ship_h__
#define __ship_h__

#include <common.hpp>

#include <string>

using std::string;

class Ship {
	private:
		enum ship_e type;
		char length;
		enum rotation_e rotation;
		char pos_x;
		char pos_y;
		bool isPlaced;
		char id;

	public:
		Ship(int type);
		Ship(enum ship_e type);
		string info();
		int place_ship(enum command_e cmd);

		int getX();
		int getY();
		bool is_placed();
		int getLen();
		enum rotation_e getRotation();
		int get_id();
		void set_id(int id);
};

#endif