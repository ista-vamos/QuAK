#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "Automaton.h"
#include "Map.h"
#include <set>
#include "State.h"


int main(int argc, char **argv) {
	Automaton A = Automaton("./samples/B.txt");
	std::cout << A.toString() << std::endl;
	A.emptiness();
	return EXIT_SUCCESS;
}
