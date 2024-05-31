#include <iostream>
#include "Automaton.h"


int main(int argc, char **argv) {
	printf("--------------------------------------------\n");

	Automaton A = Automaton("./samples/B.txt");
	std::cout << std::endl << A.toString() << std::endl;

	printf("--------------------------------------------\n");

	Automaton SupCloA = A.safetyClosure(Sup);
	std::cout << std::endl << SupCloA.toString() << std::endl;

	printf("--------------------------------------------\n");

	Automaton LimInfCloA = A.safetyClosure(LimInf);
	std::cout << std::endl << LimInfCloA.toString() << std::endl;

	printf("--------------------------------------------\n");
	Automaton LimSupCloA = A.safetyClosure(LimSup);
	std::cout << std::endl << LimSupCloA.toString() << std::endl;

	printf("--------------------------------------------\n");
	Automaton LimAvgCloA = A.safetyClosure(LimAvg);
	std::cout << std::endl << LimAvgCloA.toString() << std::endl;

	printf("--------------------------------------------\n");

	return EXIT_SUCCESS;
}
