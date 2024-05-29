#include <iostream>
#include "Automaton.h"


int main(int argc, char **argv) {
	printf("--------------------------------------------\n");

	Automaton B = Automaton("./samples/B.txt");
	std::cout << std::endl << B.toString() << std::endl;

	printf("--------------------------------------------\n");

	Automaton A = Automaton("./samples/A.txt");
	std::cout << std::endl << A.toString() << std::endl;

	printf("--------------------------------------------\n");
	return EXIT_SUCCESS;
}
