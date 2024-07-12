add_library(quak STATIC
	    Automaton.cpp
	    Edge.cpp
	    Map.cpp
	    Set.cpp
	    State.cpp
	    Symbol.cpp
	    Weight.cpp
	    Word.cpp
	    Parser.cpp
	    Monitor.cpp
	    RandomAutomaton.cpp
	    FORKLIFT/inclusion.cpp
	    FORKLIFT/FixpointLoop.cpp
	    FORKLIFT/FixpointStem.cpp
	    FORKLIFT/ContextOf.cpp
	    FORKLIFT/PostContextVariable.cpp
	    FORKLIFT/PostTargetVariable.cpp
	    FORKLIFT/StateRelation.cpp
	    FORKLIFT/TargetOf.cpp
)

add_library(quak-private STATIC utils.cpp)

add_library(quakso SHARED sharedlib.cpp)
target_link_libraries(quakso PUBLIC quak)
set_target_properties(quakso PROPERTIES OUTPUT_NAME quak)

