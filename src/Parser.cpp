#include <sstream>
#include <cstring> // errno
#include "utility.h"
#include "Parser.h"



std::string name = "";
int line_counter = 0;


void abort(std::string message) {
	std::cerr << "@Error: parsing " << message.c_str() << std::endl;
	std::cerr << "File: " << name.c_str() << std::endl;
	std::cerr << "Line: " << line_counter << std::endl;
	fflush(stdout);fflush(stderr);
	exit(EXIT_FAILURE);
}


std::string readEdge (std::string line, Parser* parser) {
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
	parser->alphabet.insert(symbolname);
	parser_verbose("Parser: Symbol = '%s'\n", symbolname.c_str());

	std::string weightname;
	buffer >> weightname;
	if (weightname.empty()) abort("transition without weight");
	std::istringstream string_to_weight(weightname);
	weight_t weight;
	string_to_weight >> weight;
	parser->weights.insert(weight);
	if (string_to_weight.eof() == false) abort("non-integer weight");
	parser_verbose("Parser: Weight = '%s'\n", std::to_string(weight).c_str());

	std::string fromname;
	buffer >> fromname;
	if (fromname.empty()) abort("transition without source state");
	parser->states.insert(fromname);
	parser_verbose("Parser: From = '%s'\n", fromname.c_str());

	std::string toname;
	buffer >> toname;
	if (toname.empty()) abort("transition without destination state");
	parser->states.insert(toname);
	parser_verbose("Parser: To = '%s'\n", toname.c_str());

	std::pair<std::pair<std::string, weight_t>,std::pair<std::string, std::string>> edge;
	edge.first.first = symbolname;
	edge.first.second = weight;
	edge.second.first = fromname;
	edge.second.second = toname;
	parser->edges.insert(edge);
	parser_verbose("Parser: Edge: %s -- %s : %d --> %s\n", fromname.c_str(), symbolname.c_str(), weight, toname.c_str());

	return fromname;
}



void readFile (std::string filename, Parser* parser) {
	name = filename;
	line_counter = 0;
	std::ifstream file(name);

	if (file.is_open() == false) {
		std::cerr << "@Error: opening file " << name << std::endl;
		std::cerr << "Message: " << strerror(errno) << std::endl;
		fflush(stdout);fflush(stderr);
		exit(EXIT_FAILURE);
	}


	std::string line;
	while (parser->initial == "" && getline(file, line)) {
		line_counter++;
		parser->initial = readEdge(line, parser);
	}

	while (getline(file, line)) {
		line_counter++;
		readEdge(line, parser);
	}

	if (parser->initial == "") abort("empty file");
	file.close();
}


Parser::Parser() {}


Parser::Parser(std::string filename) {
	readFile(filename, this);
}


Parser::Parser(std::string path, MapStd<std::string, Symbol*>* symbol_register) {
	for (std::pair<std::string, Symbol*> pair : *symbol_register) {
		this->alphabet.insert(pair.first);
	}
	readFile(path, this);
}


Parser::~Parser() {
	delete_verbose("@Detail: 4 SetStd will be deleted (parser)\n");
}

