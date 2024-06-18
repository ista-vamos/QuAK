#include <iostream>
#include "Automaton.h"
#include "Word.h"
#include "FORQ/TargetOf.h"
#include "FORQ/FixpointLoop.h"
#include "FORQ/FixpointStem.h"
#include "FORQ/ContextOf.h"
#include "FORQ/StateRelation.h"
#include "FORQ/PostContextVariable.h"
#include "FORQ/PostTargetVariable.h"



bool inclusion (Automaton* A, Automaton* B) {

	printf("MAX prefixes fix-point: computing ..\n");
	unsigned int iter_W = 1;
	FixpointStem* postIrev = new FixpointStem(A->getInitial(), B->getInitial(), true);
	while (postIrev->apply()) { iter_W++; }
	printf("MAX prefixes fix-point: done ( %u iterations )\n", iter_W);
	/*
	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		State* stateA = A->getStates()->at(stateA_id);
		SetStd<std::pair<TargetOf*, Word*>>* targets = postIrev->getSetOfTargetsOrNULL(stateA);
		printf("%s ->\n", stateA->getName().c_str());
		for (std::pair<TargetOf*, Word*> pair : *targets) {
			printf("\t %s\n", pair.second->toString().c_str());
		}
	}
	*/

	printf("MIN prefixes fix-point: computing ..\n");
	unsigned int iter_U = 1;
	FixpointStem* postI = new FixpointStem(A->getInitial(), B->getInitial(), false);
	while (postI->apply()) { iter_U++; }
	printf("MIN prefixes fix-point: done ( %u iterations )\n", iter_U);
	/*
	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		State* stateA = A->getStates()->at(stateA_id);
		SetStd<std::pair<TargetOf*, Word*>>* targets = postI->getSetOfTargetsOrNULL(stateA);
		printf("%s ->\n", stateA->getName().c_str());
		for (std::pair<TargetOf*, Word*> pair : *targets) {
			printf("\t %s\n", pair.second->toString().c_str());
		}
	}
	*/


	unsigned int final_counter = 0;
	unsigned int membership_counter = 0;
	unsigned int call_V = 0;
	for (unsigned int stateA_id = 0; stateA_id < A->getStates()->size(); ++stateA_id) {
		State* stateA = A->getStates()->at(stateA_id);
		final_counter++;

		unsigned int max_counter = 0;
		SetStd<std::pair<TargetOf*, Word*>>* setW = postIrev->getSetOfTargetsOrNULL(stateA);
		if (setW == NULL) continue;
		for (std::pair<TargetOf*, Word*> pairW : *setW) {
			call_V++;
			max_counter++;

			TargetOf* W = pairW.first;
			Word* word_of_W = pairW.second;

			unsigned int iter_V_local = 1;
			FixpointLoop* postF = new FixpointLoop(stateA, W, B->getWeights()->size());
			while (postF->apply()) { iter_V_local++; }

			unsigned int period_counter = 0;
			SetStd<std::pair<ContextOf*, std::pair<Word*,weight_t>>>* setV = postF->getSetOfContextsOrNULL(stateA);
			if (setV == NULL) continue;
			for (std::pair<ContextOf*, std::pair<Word*,weight_t>> pairV : *setV) {
				period_counter++;
				ContextOf* V = pairV.first;
				Word* word_of_V = pairV.second.first;
				weight_t valueA = pairV.second.second;
				if (valueA <= B->getMinWeightValue()) continue;

				//if (relevance_test(W, V) == false) continue;
				SetStd<std::pair<TargetOf*, Word*>>* setU = postI->getSetOfTargetsOrNULL(stateA);
				if (setU == NULL) continue;
				for (std::pair<TargetOf*, Word*> pairU : *setU) {
					TargetOf* U = pairU.first;
					Word* word_of_U = pairU.second;

					if (U->smaller_than(W) == true) {
						membership_counter++;
						int valueB = B->membership(U, word_of_V);
						TargetOf* tmp = new TargetOf();
						tmp->add(stateA);
						int value = A->membership(tmp, word_of_V);
						if (valueA != value) {
							printf("inconsistent values\n");
						}

						if (valueB < valueA) {
							printf("witness: %s cycle{ %s }\n", word_of_U->toString().c_str(), word_of_V->toString().c_str());
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}


/*
bool relevance_test (TargetOf* W, ContextOf* V) {
	for (SetOfStates set : V.getFirst().values())
		if (set->smaller_than(W) == false) return false;
	return true;
}
*/





int main(int argc, char **argv) {
	printf("--------------------------------------------\n");

	Automaton* toto = new Automaton("./samples/bigb_SUBSET.txt");
	toto->print();
	//inclusion(toto, toto);

	printf("--------------------------------------------\n");

	//Automaton* totoInfSafe = toto->safetyClosure(Inf);
	//inclusion(toto, totoInfSafe);// MapVec error
	//printf("--------------------------------------------\n");
	//Automaton* totoSupSafe = toto->safetyClosure(Sup);
	//inclusion(toto, totoSupSafe);// error
	//printf("--------------------------------------------\n");
	//Automaton* titi = new Automaton("./samples/bigb_SUPERSET.txt");
	//inclusion(toto, titi); // wrong answer
	//printf("--------------------------------------------\n");


/*
	Automaton* A = new Automaton("./samples/test3.txt");
	std::cout << std::endl << A->toString() << std::endl;
	printf("--------------------------------------------\n");

	std::cout << "Deterministic(A): " << A->isDeterministic() << std::endl;
	std::cout << "Complete(A): " << A->isComplete() << std::endl;

	printf("--------------------------------------------\n");
	Automaton* B = A->complete(Sup); // TODO: problem with sccs
	std::cout << std::endl << B->toString() << std::endl;
	printf("--------------------------------------------\n");

	std::cout << "Deterministic(B): " << B->isDeterministic() << std::endl;
	std::cout << "Complete(B): " << B->isComplete() << std::endl;

	printf("--------------------------------------------\n");
	Automaton* C = B->complete(Sup); // TODO: problem with "complete"
	std::cout << std::endl << C->toString() << std::endl;
	printf("--------------------------------------------\n");
*/

	// std::cout << "Deterministic(B): " << B->isDeterministic() << std::endl;
	// std::cout << "Empty(A,0): " << A->isEmpty(Sup, 0) << std::endl;
	// std::cout << "Empty(A,1000): " << A->isEmpty(Sup, 1000) << std::endl;

/*
	Automaton* A = new Automaton("./samples/test2.txt");
	std::cout << std::endl << A->toString() << std::endl;
	printf("--------------------------------------------\n");
	Automaton* B = new Automaton("./samples/test3.txt");
	std::cout << std::endl << B->toString() << std::endl;
	printf("--------------------------------------------\n");

	// TODO: these are sensitive to the sink state's weight. how should we handle these? maybe it's not necessary after all
	// Fixme: I think complete should be dropped
	// Fixme: and, the product should be modified to construct only what is useful
	std::cout << "Empty(A,2): " << A->isEmpty(Inf, 2) << std::endl;
	// std::cout << "Empty(A,2): " << A->complete(Inf)->isEmpty(Inf, 2) << std::endl; // this crashes because the sink weight causes problems to top value computation
	std::cout << "Universal(A,0): " << A->isUniversal_det(Sup,0) << std::endl;
	std::cout << "Universal(A,0): " << A->complete(Sup)->isUniversal_det(Sup,0) << std::endl;
*/

	// std::cout << "Included(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;
	// std::cout << "Included(B,A): " << B->isIncludedIn_det(Sup, A) << std::endl;
	// std::cout << "Equivalent(A,B): " << A->isIncludedIn_det(Sup, B) << std::endl;

	return EXIT_SUCCESS;
}
