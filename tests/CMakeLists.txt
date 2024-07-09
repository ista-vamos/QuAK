function(add_exe NAME)
	add_executable(${NAME} test-self-inclusion-rand.cpp)
	target_include_directories(${NAME} PRIVATE ${SRCDIR})
	target_link_libraries(${NAME} PRIVATE quak)
	add_test(${NAME} ${NAME})
endfunction()

add_exe(test-self-inclusion-rand-inf-1)
target_compile_definitions(test-self-inclusion-rand-inf-1 PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=1
				EDGES_NUM=4
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				)


add_exe(test-self-inclusion-rand-inf)
target_compile_definitions(test-self-inclusion-rand-inf PUBLIC
				VALUE_FUNCTION=Inf
				STATES_NUM=2
				EDGES_NUM=4
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				)

add_exe(test-self-inclusion-rand-sup)
target_compile_definitions(test-self-inclusion-rand-sup PUBLIC
				VALUE_FUNCTION=Sup
				STATES_NUM=2
				EDGES_NUM=4
				ALPHABET_SIZE=2
				MIN_WEIGHT=-10.0
				MAX_WEIGHT=10.0
				)

