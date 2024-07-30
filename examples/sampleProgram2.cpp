#include <iostream>
#include <vector>

#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Monitor.h"

int main(int argc, char **argv) {

	Automaton* B = new Automaton("samples/B.txt");
	B->print();

	Monitor* M = new Monitor(A, Avg);

	std::ifstream stream("samples/wordfile.txt");
  	if (!stream.is_open()) {
    	std::cerr << "Failed opening file: " << trace << "\n";
    	abort();
  	}

  	std::string symbol;
  	while (stream) {
    	stream >> symbol;
    	std::cout << symbol << " -> " << M->next(symbol) << "\n" << std::flush;
	}

	delete M;
	delete B;

	return EXIT_SUCCESS;
}


