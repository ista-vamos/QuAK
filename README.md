# QuAK: Quantitative Automata Kit

QuAK is an open source C++ library that helps automate the analysis of quantitative automata.
Currently, QuAK supports the classes of quantitative automata with the following value functions:
$\mathsf{Inf}, \mathsf{Sup}, \mathsf{LimInf}, \mathsf{LimSup}, \mathsf{LimInfAvg}, \mathsf{LimSupAvg}$.

Let $\mathsf{Val}$ be one of the value functions above. 
Given two $\mathsf{Val}$ automata $\mathcal{A}$ and $\mathcal{B}$ with a rational number $v \in \mathbb{Q}$, QuAK is able to solve the following problems (whenever known to be computable):
1. Check if $\mathcal{A}$ is non-empty with respect to $v$.
2. Check if $\mathcal{A}$ is universal with respect to $v$.
3. Check if $\mathcal{A}$ is included in $\mathcal{B}$.
4. Check if $\mathcal{A}$ defines a constant function.
5. Check if $\mathcal{A}$ defines a safety property.
6. Check if $\mathcal{A}$ defines a liveness property.
7. Compute the top value $\top$ of $\mathcal{A}$.
8. Compute the bottom value $\bot$ of $\mathcal{A}$.
9. Compute the safety closure of $\mathcal{A}$.
10. Compute the safety-liveness decomposition of $\mathcal{A}$.
11. Construct and execute a monitor for $\mathcal{A}$.

## Building
### Building through Dockerfile

You can build a container with QuAK using Docker or Podman and the provided Dockerfile
in the top-level project directory (there is also another Dockerfile in the directory `experiments/`,
which does more things that just building QuAK.). From the top-level directory, run:
```
docker build . -t quak
```

Note: the container built by the other Dockerfile (`experiments/Dockerfile`) is
also named "quak" (if you precisely follow the instructions from
`experiments/README.md`). The command above will then overwrite the image. To
avoid this, change `-t quak` to `-t <new_name>` where `<new_name>` is a name of
the container different from the name of the container with the experiments. Do
not forget to use this new name when running the contanier later.

Once you have the docker image built, you can start a terminal inside the docker image as follows:
```
docker run --rm -ti quak
```


### Building from sources

QuAK has no external dependencies. The only requirements are to have a C++ compiler that supports the C++17 standard or newer,
and make. Recommended is to have also CMake, which is used by default to configure the project.

#### Using CMake

The easiest way to build QuAK is to use CMake + make. On Ubuntu, you can
install CMake and Make with the following command:

```
apt-get install make cmake
# install also C++ compiler if you do not have one
# apt-get install g++
```

Then you can compile QuAK:

```
cmake . -DCMAKE_BUILD_TYPE=Release
make -j4
```

For debug builds, use `Debug` instead of `Release`. You can tweak compile time
options that enable the optimization of algorithms: use
`-DENABLE_SCC_SEARCH_OPT=OFF` to turn off an SCC-based optimization of deciding
language inclusion (and other problems where the inclusion algorithm is used as
a subroutine).

To compile the code with link-time (i.e., inter-procedural) optimizations,
use the option `-DENABLE_IPO=ON`.

Once compiled, you can run tests with calling `make test`.


##### Building with VAMOS integration

First build [VAMOS](https://github.com/ista-vamos/vamos).
Then run cmake with these parameters:

```
cmake . -Dvamos_DIR=/path/to/vamos/directory
make -j4
```

#### Building with the legacy Makefile

If you have troubles building QuAK with CMake, you can try using building it
only with Make. To do this, run:

```
make -f Makefile.legacy
```

This makefile is likely out of date and does not support building tests,
neither integration with VAMOS. It is a subject of removal in the future.

## Input Format

QuAK reads and constructs automata from text files.
Each automata is represented as a list of transitions of the following format:
```
a : v, q -> p
```
which encodes a transition from state $q$ to state $p$ with letter $a$ and weight $v$.
Weight $v$ is either a C float number, or an unsigned hexadecimal integer that represents
the bits of a C float number.

The initial state of the input automaton is the source state of the first transition in its text file.

**Important:** QuAK requires that its input automata are complete (a.k.a. total),
i.e., for every state $q$ and every letter $a$, there is at least one outgoing
transition from $q$ with letter $a$.

## Using QuAK (as a library)

To use the library in your program, use the following directive:
```cpp
#include "FORKLIFT/inclusion.h"
#include "Automaton.h"
#include "Monitor.h"
```

For a sample program that puts together the demonstrations below, see *examples/sampleProgram.cpp*.

Below, we consider two $\mathsf{Val}$ automata $\mathcal{A}$ and $\mathcal{B}$ with a rational number $v \in \mathbb{Q}$.

### Constructing Automata
To construct an automaton from a file, use the following template:
```cpp
Automaton* A = new Automaton("A.txt");
```
The value function is unspecified during construction, but it needs to be passed as a parameter to the functions below.

<!-- To use the copy constructor (that also trims and completes the input), use the following:
```cpp
Automaton* B = new Automaton(A, valueFunction);
```
Here, the value function is needed because the weights of the transitions involving the new sink state depends on the value function. -->

### Non-emptiness Check
To check the non-emptiness of $\mathcal{A}$ with respect to $v$, use the following:
```cpp
bool flag = A->isNonEmpty(Val, v);
```

### Universality Check
To check the universality of $\mathcal{A}$ with respect to $v$, use the following:
```cpp
bool flag = A->isUniversal(Val, v);
```

### Inclusion Check
To check the inclusion of $\mathcal{A}$ in $\mathcal{B}$, use the following:
```cpp
bool flag = A->isIncludedIn(B, Val, booleanized);
```
where *booleanized* determines which inclusion algorithms is called.
If *booleanized* is false, then our quantitative extension of the antichain algorithm is used.
If *booleanized* is true, then the standard inclusion algorithm (repeatedly booleanizing the quantitative automaton and calling the boolean antichain algorithm) is used.
By default, *booleanized* is set to false.

### Equivalence Check
To check the equivalence of $\mathcal{A}$ and $\mathcal{B}$, use the following:
```cpp
bool flag = A->isEquivalentTo(B, Val, booleanized);
```
where *booleanized* is the same a for the inclusion check.


### Constant-function Check
To check if $\mathcal{A}$ defines a constant function, use the following:
```cpp
bool flag = A->isConstant(Val);
```

### Safety Check
To check if $\mathcal{A}$ defines a safety property, use the following:
```cpp
bool flag = A->isSafe(Val);
```

### Liveness Check
To check if $\mathcal{A}$ defines a liveness property, use the following:
```cpp
bool flag = A->isLive(Val);
```

### Top-value Computation
To compute the top value of $\mathcal{A}$, use the following:
```cpp
weight_t top = A->getTopValue(Val);
```

### Bottom-value Computation
To compute the bottom value of $\mathcal{A}$, use the following:
```cpp
weight_t bot = A->getBottomValue(Val);
```

### Safety Closure Construction
To construct the safety closure of $\mathcal{A}$, use the following:
```cpp
Automaton* safe_A = safetyClosure(A, Val);
```

### Safety-Liveness Decomposition
For the safety component of the decomposition, use the safety closure construction above.
To construct the liveness component of the decomposition of $\mathcal{A}$, use the following:
```cpp
Automaton* live_A = livenessComponent_deterministic(A, Val);
```

### Witnesses

All the above-mentioned operations can return a witness for its results: an ultimately periodic word
that witnesses the returned value. This is done via the optional argument `witness`:

```
UltimatelyPeriodicWord *witness;
bool flag = A->isNonEmpty(Val, v, &witness);
// ... process witness
delete witness;

weight_t bot = A->getBottomValue(Val, &witness);
// ... process witness
delete witness;

UltimatelyPeriodicWord *witness1, *witness2;
bool flag = A->isEquivalentTo(B, val, booleanized, &witness1, &witness2);
// ... process witnesses
delete witness1;
delete witness2;
```

Note that you must delete the witness manually once you are done with it.
The witness format is `prefix(cycle)`, e.g., `aa(ab)` is the word `aaababab...`.

### Monitor Construction and Execution
QuAK can contsruct monitors from deterministic automata by either reading them from a file or copying an automaton object:
```cpp
Monitor* M = new Monitor("A.txt", Val);
```
```cpp
Monitor* M = new Monitor(A, Val);
```
where $\mathsf{Val}$ is $\mathsf{Avg}$. 

The monitor updates its state by reading a letter of type *std::string* and returning the current value:
```cpp
weight_t t = M->next(letter);
```
For example, a monitor can process a word file as follows: 
```cpp
std::ifstream stream("samples/wordfile.txt");
std::string symbol;
while (stream) {
    stream >> symbol;
    std::cout << symbol << " -> " << M->next(symbol) << "\n" << std::flush;
}
```
<!-- 
At any point, the monitor can provide the highest and lowest values achievable from the current state of its input automaton:
```cpp
weight_t h = M->getHighest(); 
weight_t l = M->getLowest();
``` -->

## Using QuAK (as a tool)

To use the tool directly, simply compile and follow the instructions below.

```
Usage: ./quak [-cputime] [-v] [-d] [-print-witness] automaton-file [ACTION ACTION ...]
Where ACTIONs are the following, with VALF = <Inf | Sup | LimInf | LimSup | LimSupAvg | LimInfAvg>:
  stats
  dump 
  empty VALF <weight>
  non-empty VALF <weight>
  universal VALF <weight>
  constant VALF
  safe VALF
  live VALF
  isIncluded VALF automaton2-file
  isIncludedBool VALF automaton2-file
  isEquivalent VALF automaton2-file
  monitor <Inf | Sup | Avg> word-file
  witness-file file-name
```

The commands *stats* prints the size of the automaton and *dump* prints the automaton. Command *witness-file* instructs the preceding command to write a witness (if any) to the given file.
The remaining commands implement the decision procedures and monitoring algorithms as expected.
For monitoring, the word files must contain one symbol per line.
Use the option *-cputime* to print the running time, *-v* to print the input size, and *-d* to print the automaton.
Option *-print-witness* will make each operation to print the witness (if any).
Some examples are given below.

```
$ ./quak -cputime -d  A.txt safe LimInfAvg

Cputime of building the automaton: 3 ms
automaton (A.txt):
	alphabet (2):
		0 -> a
		1 -> b
	weights (5):
		0 -> -9.545000
		1 -> -5.077000
		2 -> 0.634000
		3 -> 1.100000
		4 -> 6.122000
		MIN = -9.545000
		MAX = 6.122000
	states (2):
		0 -> q0, scc: 0
		1 -> q1, scc: -1
		INITIAL = q0
	SCCs (1):
		q0
	edges (5):
		a : -9.545, q0 -> q0
		b : 1.1, q0 -> q0
		a : 0.634, q1 -> q1
		a : 6.122, q1 -> q0
		b : -5.077, q1 -> q0

----------
isSafe(LimInfAvg) = 0
Cputime: 4 ms
----------
```

```
$ ./quak A.txt constant Inf witness-file w.txt 
----------
isConstant(Inf) = 0
----------

$ cat w.txt
a(a)
```

```
$ ./quak -print-witness A.txt safe LimInfAvg witness-file w1.txt constant Sup witness-file w2.txt
----------
isSafe(LimInfAvg) = 0
Witness: (a)
----------
isConstant(Sup) = 0
Witness: (a)
----------

$ cat w1.txt
(a)
$ cat w2.txt
(a)
```

