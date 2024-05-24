
#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <fstream>


class Parser {
private:
	std::string filename;
	int line_counter = -1;
	std::ifstream file;

	std::string initial = "";
	SetStd<std::string> states;
	SetStd<std::string> alphabet;
	SetStd<std::pair<std::pair<std::string, int>,std::pair<std::string, std::string>>> edges;

	Parser (std::string filename);
	~Parser();
	void abort(std::string message);
	std::string readEdge (std::string line);
public:
	friend class Automaton;
};



#endif /* PARSER_H_ */



