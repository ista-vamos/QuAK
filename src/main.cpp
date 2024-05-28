#include <iostream>
#include "Automaton.h"


int main(int argc, char **argv) {
	Automaton B = Automaton("./samples/B.txt");
	std::cout << B.toString() << std::endl;
	//B.toto();

	Automaton A = Automaton("./samples/A.txt");
	std::cout << A.toString() << std::endl;
	//A.toto();

	return EXIT_SUCCESS;
}
