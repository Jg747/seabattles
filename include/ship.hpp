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
		int id;
		int hits;

	public:

		Ship(int type);
		Ship(enum ship_e type);
		string info();
		int place_ship(enum command_e cmd);
		bool point_intersect(int x, int y);
		void add_hit();
		void reset_hits();

		int get_x();
		int get_y();
		bool is_placed();
		int get_len();
		enum ship_e get_type();
		enum rotation_e get_rotation();
		int get_id();
		void set_id(int id);
		void set_placed(bool state);
		void set_x(int x);
		void set_y(int y);
		void set_rotation(enum rotation_e r);
		bool is_sunk();
		int taken_hits();
		string pos_to_string();
		string type_to_string();

		static bool evaluate_pos(int x, int y, int len, enum rotation_e rotate);
		
		string get_info();
};

#endif