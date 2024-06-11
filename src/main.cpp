#include <iostream>
#include "Automaton.h"


int main(int argc, char **argv) {
	printf("--------------------------------------------\n");
	Automaton* A = new Automaton("./samples/test2.txt");
	std::cout << std::endl << A->toString() << std::endl;
	printf("--------------------------------------------\n");
	Automaton* B = new Automaton("./samples/test3.txt");
	std::cout << std::endl << B->toString() << std::endl;
	printf("--------------------------------------------\n");

	// TODO: these are sensitive to the sink state's weight. how should we handle these? maybe it's not necessary after all
	std::cout << "Empty(A,2): " << A->isEmpty(Inf, 2) << std::endl;
	// std::cout << "Empty(A,2): " << A->complete(Inf)->isEmpty(Inf, 2) << std::endl; // this crashes because the sink weight causes problems to top value computation
	std::cout << "Universal(A,0): " << A->isUniversal_det(Sup,0) << std::endl;
	std::cout << "Universal(A,0): " << A->complete(Sup)->isUniversal_det(Sup,0) << std::endl;

	// std::cout << "Included(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;
	// std::cout << "Included(B,A): " << B->isIncludedIn_det(Sup, A) << std::endl;
	// std::cout << "Equivalent(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;



	return EXIT_SUCCESS;
}
