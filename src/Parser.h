
#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <fstream>


class Parser {
private:
	std::string filename;
	int line_counter;
	std::ifstream file;

	std::string initial = "";
	MapStd<std::string,unsigned int> states;
	SetStd<std::string> alphabet;
	SetStd<weight_t> weights;
	SetStd<std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>>> edges;
	//MapStd<std::string, MapStd<std::string, unsigned int>*> successors;
	//MapStd<std::string, MapStd<std::string, unsigned int>*> predecessors;

	Parser (std::string filename);
	~Parser();
	void abort(std::string message);
	std::string readEdge (std::string line);
public:
	friend class Automaton;
};



#endif /* PARSER_H_ */



