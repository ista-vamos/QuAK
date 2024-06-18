
#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <fstream>
#include "Map.h"
#include "Symbol.h"
#include "Set.h"
#include "State.h"
#include "Set.h"
#include "Weight.h"

class Parser {
private:
	std::string filename;
	int line_counter;
	std::ifstream file;

	std::string initial = "";
	SetStd<std::string> states;
	SetStd<std::string> alphabet;
	SetStd<weight_t> weights;
	SetStd<std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>>> edges;

	Parser (std::string filename);
	Parser (std::string filename, MapStd<std::string, Symbol*>* symbol_register);
	~Parser();
	void abort(std::string message);
	std::string readEdge (std::string line);
public:
	friend class Automaton;
};



#endif /* PARSER_H_ */



