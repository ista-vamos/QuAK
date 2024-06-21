
#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <fstream>
#include "Map.h"
#include "Set.h"


class Parser {
public:
	std::string initial = "";
	SetStd<std::string> states;
	SetStd<std::string> alphabet;
	SetStd<weight_t> weights;
	SetStd<std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>>> edges;
	Parser ();
	Parser (std::string filename);
	Parser (std::string filename, MapStd<std::string, Symbol*>* symbol_register);
	~Parser();
};



#endif /* PARSER_H_ */



