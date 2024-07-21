set(SRCDIR ../src)

function(add_self_exe NAME)
	add_executable(${NAME} test-self-inclusion-rand.cpp)
	target_include_directories(${NAME} PRIVATE ${SRCDIR})
	target_link_libraries(${NAME} PRIVATE quak)
	add_test(${NAME} ${NAME})
endfunction()


function(add_exe NAME)
	add_executable(${NAME} test-inclusion-rand.cpp)
	target_include_directories(${NAME} PRIVATE ${SRCDIR})
	target_link_libraries(${NAME} PRIVATE quak)
	add_test(${NAME} ${NAME})
endfunction()



############################################################
### Inf
############################################################

set(TEST_NAME test-self-inclusion-rand-inf-1state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=1
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-inf-2state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=2
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-inf-4state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=4
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=4000
				)


set(TEST_NAME test-self-inclusion-rand-inf-10state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=10
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10
				)

set(TEST_NAME test-self-inclusion-rand-inf-8state-4alph)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=8
				EDGES_NUM=0
				ALPHABET_SIZE=4
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10
				)

set(TEST_NAME test-inclusion-rand-inf-1state)
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


set(TEST_NAME test-inclusion-rand-inf-2state)
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


set(TEST_NAME test-inclusion-rand-inf-4state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=4
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=100
				)


############################################################
### Sup
############################################################


set(TEST_NAME test-self-inclusion-rand-sup-1state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=1
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-sup-2state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=2
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-sup-4state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=4
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=4000
				)

set(TEST_NAME test-self-inclusion-rand-sup-10state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=10
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10
				)

set(TEST_NAME test-self-inclusion-rand-sup-8state-4alph)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=8
				EDGES_NUM=0
				ALPHABET_SIZE=4
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10
				)

set(TEST_NAME test-inclusion-rand-sup-1state)
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

set(TEST_NAME test-inclusion-rand-sup-2state)
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

set(TEST_NAME test-inclusion-rand-sup-4state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=4
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=100
				)


set(TEST_NAME test-inclusion-rand-sup-8state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=8
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=50
				)



############################################################
### LimSup
############################################################


set(TEST_NAME test-self-inclusion-rand-limsup-1state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=LimSup
				STATES_NUM=1
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-limsup-2state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=LimSup
				STATES_NUM=2
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-self-inclusion-rand-limsup-4state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=LimSup
				STATES_NUM=4
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=4000
				)

set(TEST_NAME test-self-inclusion-rand-limsup-10state)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=LimSup
				STATES_NUM=10
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10
				)

set(TEST_NAME test-self-inclusion-rand-limsup-8state-4alph)
add_self_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=LimSup
				STATES_NUM=8
				EDGES_NUM=0
				ALPHABET_SIZE=4
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10
				)

set(TEST_NAME test-inclusion-rand-limsup-1state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=LimSup
				STATES_NUM=1
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-inclusion-rand-limsup-2state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=LimSup
				STATES_NUM=2
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=10000
				)

set(TEST_NAME test-inclusion-rand-limsup-4state)
add_exe(${TEST_NAME})
target_compile_definitions(${TEST_NAME} PUBLIC
				VALUE_FUNCTION=LimSup
				STATES_NUM=4
				EDGES_NUM=0
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				REPETITIONS=1000
				)
