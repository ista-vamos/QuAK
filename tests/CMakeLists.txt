function(add_exe NAME)
	add_executable(${NAME} test-self-inclusion-rand.cpp)
	target_include_directories(${NAME} PRIVATE ${SRCDIR})
	target_link_libraries(${NAME} PRIVATE quak)
	add_test(${NAME} ${NAME})
endfunction()



set(TEST_NAME test-self-inclusion-rand-inf-1state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=1
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-sup-1state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=1
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-inf-2state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=2
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-sup-2state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=2
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-inf-4state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=4
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=5000
				)

set(TEST_NAME test-self-inclusion-rand-sup-4state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=4
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=4000
				)

set(TEST_NAME test-self-inclusion-rand-inf-10state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=10
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=1000
				)

set(TEST_NAME test-self-inclusion-rand-sup-10state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=10
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=1000
				)

set(TEST_NAME test-self-inclusion-rand-inf-10state-5alph)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=10
				EDGES_NUM=0
				ALPHABET_SIZE=5
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10
				)

set(TEST_NAME test-self-inclusion-rand-sup-10state-5alph)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=10
				EDGES_NUM=0
				ALPHABET_SIZE=5
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=100
				)
