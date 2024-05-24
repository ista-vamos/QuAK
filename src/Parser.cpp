#include <sstream>
#include <cstring> // errno, atoi
#include "Set.h"
#include "Map.h"
#include "Symbol.h"
#include "State.h"
#include "utility.h"
#include "Parser.h"


void Parser::abort(std::string message) {
	std::cerr << "@Error: parsing " << message.c_str() << std::endl;
	std::cerr << "File: " << this->filename.c_str() << std::endl;
	std::cerr << "Line: " << this->line_counter << std::endl;
	fflush(stdout);fflush(stderr);
	exit(EXIT_FAILURE);
}


std::string Parser::readEdge (std::string line) {
	// -- expected shape: symbol : weight, from -> to #comment
	size_t index;
	if (line.empty()) return "";

	index = line.find("->");
	if (index == std::string::npos) abort("edge without '->'");
	line[index] = ' '; line[index+1] = ' ';

	index = line.find(',');
	if (index == std::string::npos) abort("edge without ','");
	line[index] = ' ';

	index = line.find(':');
	if (index == std::string::npos) abort("edge without ':'");
	line[index] = ' ';

	index = line.find('#');
	if (index != std::string::npos) {
		unsigned int i = index;
		while (i < line.length()) {
			line[i++] = ' ';
		}
	}

	std::istringstream buffer(line);

	std::string symbolname;
	buffer >> symbolname;
	if (symbolname.empty()) abort("transition without symbol");
	this->alphabet.insert(symbolname);
	parser_verbose("Parser: Symbol = '%s'\n", symbolname.c_str());

	std::string weightname;
	buffer >> weightname;
	if (weightname.empty()) abort("transition without weight");
	int weight = atoi(weightname.c_str());
	parser_verbose("Parser: Weight = '%d'\n", weight);

	std::string fromname;
	buffer >> fromname;
	if (fromname.empty()) abort("transition without source state");
	this->states.insert(fromname);
	parser_verbose("Parser: From = '%s'\n", fromname.c_str());

	std::string toname;
	buffer >> toname;
	if (toname.empty()) abort("transition without destination state");
	this->states.insert(toname);
	parser_verbose("Parser: To = '%s'\n", toname.c_str());

	std::pair<std::pair<std::string, int>,std::pair<std::string, std::string>> edge;
	edge.first.first = symbolname;
	edge.first.second = weight;
	edge.second.first = fromname;
	edge.second.second = toname;
	this->edges.insert(edge);
	parser_verbose("Parser: Edge: %s -- %s : %d --> %s\n", fromname.c_str(), symbolname.c_str(), weight, toname.c_str());

	return fromname;
}


// handles lines without the substring "->"
/*
static State* readWeight(std::string line, parser_t* parser) {
	size_t index = line.find(':');
	for (unsigned int i = 0; i< line.length(); i++) {
		if (line[i] == ':' || line[i] == '#')  line[i] = ' ';
	}
	std::istringstream buffer(line);
	std::string statename;
	buffer >> statename;
	printf("Name = '%s'\n", statename.c_str());
	//state.add(line);

	int weight;
	if (index != std::string::npos) {
		buffer >> weight;
		printf("Weight = '%d'\n", weight);
	} else {
		printf("Weight = '1'\n");
	}

	State* state = new StateRel(statename, weight);
	printf("State: %s\n", state->toString().c_str());
	//state.add
	return state;
}
*/


// handles the first line
/*
static void readInitial(std::string line, parser_t* parser) {
	// -- if 'init' unspecified then take the first defined state
	if (line.find("->") != std::string::npos)
		parser->initial = readEdge(line, parser);
	else {
		parser->initial = readWeight(line, parser);
	}
}
*/




/*
static std::list<std::string> mytokenizer(std::string line, const std::string tokens) {
	std::list<std::string> list = {};

	unsigned int i, j;
	j = 0;
	for (i = 0; i < line.length(); i++) {
		if (tokens.find(line[i]) != std::string::npos) {
			if (j < i) {
				list.push_front(line.substr(j, i-j));
			}
			j = i+1;
		}
	}
	if (j < line.length())
		list.push_front(line.substr(j, line.length()-j));

	return list;
}
*/


Parser::Parser(std::string filename) {
	this->filename = filename;
	this->line_counter = -1;
	this->file.open(filename);


	if (this->file.is_open() == false) {
		std::cerr << "@Error: opening file " << filename << std::endl;
		std::cerr << "Message: " << strerror(errno) << std::endl;
		fflush(stdout);fflush(stderr);
		exit(EXIT_FAILURE);
	}


	std::string line;
	while (this->initial == "" && getline(this->file, line)) {
		line_counter++;
		this->initial = readEdge(line);
	}

	while (getline(this->file, line)) {
		line_counter++;
		readEdge(line);
	}

	if (this->initial == "") abort("empty file");
	states.erase(this->initial);

	this->file.close();
}

Parser::~Parser() {
	delete_verbose("@Detail: 3 SetStd will be deleted (parser)\n");
}


/*
//-- This is a usefull function
static void strip(std::string &s) {
	unsigned int i = 0;
	while(i<s.length() && s[i]==' ') i++;
	s.erase(0, i);
	i = s.length()-1;
	while(i>=0 && s[i]==' ') i--;
	s.erase(i+1, s.length());
}
*/
