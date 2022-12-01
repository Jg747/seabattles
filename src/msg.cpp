#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <string.h>

#include <pugi/pugixml.hpp>
#include <msg.hpp>

const char *MSG_TYPE_STR[] = {
	"MSG_PLAYER_GET_OWN_ID"
};

template<typename T>
static bool is_a_string(T value) {
	return 
		std::is_same<T, std::string>::value ||
		std::is_same<T, char*>::value ||
		std::is_same<T, const std::string>::value ||
		std::is_same<T, const char*>::value;
}

template<typename T>
static void add_value(pugi::xml_node &elem, T value) {
	std::string temp;
	if (!is_a_string(value)) {
		std::stringstream stream;
		stream << value;
		stream >> temp;
		if (stream.fail()) {
			return;
		}
	} else {
		temp = value;
	}
	elem.append_child(pugi::node_pcdata).set_value(temp.c_str());
}

static pugi::xml_document init_msg(enum msg_type_e type) {
	pugi::xml_document doc;
	pugi::xml_node msg = doc.append_child("message");
	pugi::xml_node msg_type = msg.append_child("type");
	msg_type.append_child(pugi::node_pcdata).set_value(MSG_TYPE_STR[type]);
	return doc;
}

static std::string close_msg(pugi::xml_document &doc) {
	std::string str;
	std::ostringstream stream;
	doc.save(stream);
	return stream.str();
}

template<typename T>
static void add_node(pugi::xml_document &doc, std::string elem, T value) {
	pugi::xml_node msg = doc.child("message");
	pugi::xml_node data;
	if (!(data = msg.child("data"))) {
		data = msg.append_child("data");
	}

	pugi::xml_node element = data.append_child(elem.c_str());
	add_value(element, value);
}

template<typename T>
static void add_node(pugi::xml_document &doc, std::string elem, T values[], int len) {
	pugi::xml_node msg = doc.child("message");
	pugi::xml_node data;
	if (!(data = msg.child("data"))) {
		data = msg.append_child("data");
	}

	pugi::xml_node element = data.append_child(elem.c_str());
	for (int i = 0; i < len; i++) {
		pugi::xml_node col = element.append_child("element");
		add_value(col, values[i]);
	}
}

template<typename T>
static void add_node(pugi::xml_document &doc, std::string elem, T **values, int rows, int cols) {
	pugi::xml_node msg = doc.child("message");
	pugi::xml_node data;
	if (!(data = msg.child("data"))) {
		data = msg.append_child("data");
	}

	pugi::xml_node element = data.append_child(elem.c_str());
	for (int i = 0; i < rows; i++) {
		pugi::xml_node row = element.append_child("row");
		for (int j = 0; j < cols; j++) {
			pugi::xml_node col = row.append_child("element");
			add_value(col, values[i][j]);
		}
	}
}

template<typename T>
static void add_node(pugi::xml_document &doc, std::string elem, std::vector<T> values) {
	pugi::xml_node msg = doc.child("message");
	pugi::xml_node data;
	if (!(data = msg.child("data"))) {
		data = msg.append_child("data");
	}

	pugi::xml_node element = data.append_child(elem.c_str());
	for (auto v : values) {
		pugi::xml_node col = element.append_child("element");
		add_value(col, v);
	}
}

// ----------------------------------------------------
//                     MSG CREATION                    
// ----------------------------------------------------

std::string create_message(enum msg_type_e type, void *ptr) {
	pugi::xml_document doc = init_msg(type);

	switch (type) {

	}

	return close_msg(doc);
}

std::string create_message(enum msg_type_e type) {
	return create_message(type, NULL);
}

// ----------------------------------------------------
//                      MSG PARSING                     
// ----------------------------------------------------

void parse_message(std::string &str, void *ptr) {
	// ...
}

enum msg_type_e get_msg_type(std::string str) {
	// ...
}
