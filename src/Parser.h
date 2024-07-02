
#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <fstream>
#include "Map.h"
#include "Set.h"


class Parser {
public:
	std::string initial = "";
	bool domain_defined = false;
	weight_t min_domain = 0;
	weight_t max_domain = 0;
	SetStd<std::string> states;
	SetStd<std::string> alphabet;
	SetSorted<weight_t> weights;
	SetStd<std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>>> edges;
	Parser(weight_t min_domain, weight_t max_domain);
	Parser (std::string filename);
	Parser (std::string filename, MapStd<std::string, Symbol*>* symbol_register);
	~Parser();
};



#endif /* PARSER_H_ */



