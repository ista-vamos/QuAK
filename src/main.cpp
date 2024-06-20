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




 bool relevance_test (TargetOf* W, ContextOf* V, Automaton* B) {
 	for (std::pair<State*, TargetOf*> pair : *(V->at(0))) {
		if (pair.second->smaller_than(W) == false) return false;
 	}
 	return true;
 }




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
		//if (A->getStates()->at(stateA_id)->getMaxWeightValue() < 1) continue; //fixme:

		State* stateA = A->getStates()->at(stateA_id);
		final_counter++;

		unsigned int max_counter = 0;
		SetStd<std::pair<TargetOf*, Word*>>* setW = postIrev->getSetOfTargetsOrNULL(stateA);
		if (setW == NULL) continue;
		for (std::pair<TargetOf*, Word*> pairW : *setW) {
			call_V++;
			max_counter++;

			TargetOf* W = pairW.first;
			//Word* word_of_W = pairW.second; // unused

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

				if (relevance_test(W, V, B) == false) continue;
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
	printf("TRUE\n");
	return true;
}





int main(int argc, char **argv) {
/*
	printf("--------------------------------------------\n");

	Automaton* A = new Automaton("./samples/test3.txt");
	// bool flag = A->isLive(Sup);
	Automaton* B = A->toLimSup(LimInf);
	// Automaton* C = B->trim();

	A->print();
	B->print();
	// C->print();

	delete A;
	delete B;
	// delete C;
*/



	Automaton* A1 = new Automaton("./samples/slides_example_SUPERSET.txt");
	A1->print();



	Automaton* toto = NULL;
	Automaton* titi = NULL;
/*
	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/All_positive_numbers_have_a_predecessor_SUBSET.txt");
	titi = new Automaton("./samples/All_positive_numbers_have_a_predecessor_SUPERSET.txt", toto);
	std::cout << "All_positive_numbers_have_a_predecessor" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");

	titi = new Automaton("./samples/All_positive_numbers_have_a_predecessor_SUPERSET.txt");
	toto = new Automaton("./samples/All_positive_numbers_have_a_predecessor_SUBSET.txt", titi);
	std::cout << "All_positive_numbers_have_a_predecessor" << std::endl;
	inclusion(titi, toto);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/All_Sturmian_words_contain_cubes_SUBSET.txt");
	titi = new Automaton("./samples/All_Sturmian_words_contain_cubes_SUPERSET.txt", toto);
	std::cout << "All_Sturmian_words_contain_cubes" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");

	titi = new Automaton("./samples/All_Sturmian_words_contain_cubes_SUPERSET.txt");
	toto = new Automaton("./samples/All_Sturmian_words_contain_cubes_SUBSET.txt", titi);
	std::cout << "All_Sturmian_words_contain_cubes" << std::endl;
	inclusion(titi, toto);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/All_Sturmian_words_start_with_arbitarily_long_palindromes_SUBSET.txt");
	titi = new Automaton("./samples/All_Sturmian_words_start_with_arbitarily_long_palindromes_SUPERSET.txt", toto);
	std::cout << "All_Sturmian_words_start_with_arbitarily_long_palindromes" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");

	titi = new Automaton("./samples/All_Sturmian_words_start_with_arbitarily_long_palindromes_SUPERSET.txt");
	toto = new Automaton("./samples/All_Sturmian_words_start_with_arbitarily_long_palindromes_SUBSET.txt", titi);
	std::cout << "All_Sturmian_words_start_with_arbitarily_long_palindromes" << std::endl;
	inclusion(titi, toto);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

//	toto = new Automaton("./samples/bakeryV3_SUBSET.txt");
//	titi = new Automaton("./samples/bakeryV3_SUPERSET.txt", toto);
//	std::cout << "bakeryV3" << std::endl;
//	inclusion(toto, titi);
//	delete toto;
//	delete titi;

	printf("--------------------------------------------\n");

//	titi = new Automaton("./samples/bakeryV3_SUPERSET.txt");
//	toto = new Automaton("./samples/bakeryV3_SUBSET.txt", titi);
//	std::cout << "bakeryV3" << std::endl;
//	inclusion(titi, toto);
//	delete toto;
//	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/BuchiCegarLoopAbstraction_SUBSET.txt");
	titi = new Automaton("./samples/BuchiCegarLoopAbstraction_SUPERSET.txt", toto);
	std::cout << "BuchiCegarLoopAbstraction" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");

	titi = new Automaton("./samples/BuchiCegarLoopAbstraction_SUPERSET.txt");
	toto = new Automaton("./samples/BuchiCegarLoopAbstraction_SUBSET.txt", titi);
	std::cout << "BuchiCegarLoopAbstraction" << std::endl;
	inclusion(titi, toto);
	delete toto;
	delete titi;


	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/example_SUBSET.txt");
	titi = new Automaton("./samples/example_SUPERSET.txt", toto);
	std::cout << "example" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");

	titi = new Automaton("./samples/example_SUPERSET.txt");
	toto = new Automaton("./samples/example_SUBSET.txt", titi);
	std::cout << "example" << std::endl;
	inclusion(titi, toto);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/fischerV3_SUBSET.txt");
	titi = new Automaton("./samples/fischerV3_SUPERSET.txt", toto);
	std::cout << "fischerV3" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");

	titi = new Automaton("./samples/fischerV3_SUPERSET.txt");
	toto = new Automaton("./samples/fischerV3_SUBSET.txt", titi);
	std::cout << "fischerV3" << std::endl;
	inclusion(titi, toto);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

//	toto = new Automaton("./samples/Odd_and_even_work_as_expected_SUBSET.txt");
//	titi = new Automaton("./samples/Odd_and_even_work_as_expected_SUPERSET.txt", toto);
//	std::cout << "Odd_and_even_work_as_expected" << std::endl;
//	inclusion(toto, titi);
//	delete toto;
//	delete titi;

	printf("--------------------------------------------\n");

//	titi = new Automaton("./samples/Odd_and_even_work_as_expected_SUPERSET.txt");
//	toto = new Automaton("./samples/Odd_and_even_work_as_expected_SUBSET.txt", titi);
//	std::cout << "Odd_and_even_work_as_expected" << std::endl;
//	inclusion(titi, toto);
//	delete toto;
//	delete titi;


	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

//	toto = new Automaton("./samples/peterson_SUBSET.txt");
//	titi = new Automaton("./samples/peterson_SUPERSET.txt", toto);
//	std::cout << "peterson" << std::endl;
//	inclusion(toto, titi);
//	delete toto;
//	delete titi;

	printf("--------------------------------------------\n");

//	titi = new Automaton("./samples/peterson_SUPERSET.txt");
//	toto = new Automaton("./samples/peterson_SUBSET.txt", titi);
//	std::cout << "peterson" << std::endl;
//	inclusion(titi, toto);
//	delete toto;
//	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/slides_example_SUBSET.txt");
	titi = new Automaton("./samples/slides_example_SUPERSET.txt", toto);
	std::cout << "slides_example" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");

	titi = new Automaton("./samples/slides_example_SUPERSET.txt");
	toto = new Automaton("./samples/slides_example_SUBSET.txt", titi);
	std::cout << "slides_example" << std::endl;
	inclusion(titi, toto);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/Specal_factors_are_unique_SUBSET.txt");
	titi = new Automaton("./samples/Specal_factors_are_unique_SUPERSET.txt", toto);
	std::cout << "Specal_factors_are_unique" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");

	titi = new Automaton("./samples/Specal_factors_are_unique_SUPERSET.txt");
	toto = new Automaton("./samples/Specal_factors_are_unique_SUBSET.txt", titi);
	std::cout << "Specal_factors_are_unique" << std::endl;
	inclusion(titi, toto);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");

	toto = new Automaton("./samples/The_lazy_Ostrowski_representation_is_unique_SUBSET.txt");
	titi = new Automaton("./samples/The_lazy_Ostrowski_representation_is_unique_SUPERSET.txt", toto);
	std::cout << "The_lazy_Ostrowski_representation_is_unique" << std::endl;
	inclusion(toto, titi);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");

	titi = new Automaton("./samples/The_lazy_Ostrowski_representation_is_unique_SUPERSET.txt");
	toto = new Automaton("./samples/The_lazy_Ostrowski_representation_is_unique_SUBSET.txt", titi);
	std::cout << "The_lazy_Ostrowski_representation_is_unique" << std::endl;
	inclusion(titi, toto);
	delete toto;
	delete titi;

	printf("--------------------------------------------\n");
	printf("--------------------------------------------\n");
*/





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
