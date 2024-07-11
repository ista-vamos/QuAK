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
11. Construct and execute a monitor for extremal values of $\mathcal{A}$.


## Building from sources

### Using CMake

The easiest way to build QuAK is to use CMake + make:

On Ubuntu, you can install CMake and Make with the following command:
```
apt-get install make cmake
```

Then you can compile QuAK:

```
cmake . -DCMAKE_BUILD_TYPE=Release
make -j4
```

For debug builds, use `Debug` instead of `Release`. You can tweak compile time
options that enable optimizations of algorithms, namely use
`-DENABLE_SCC_SEARCH_OPT=OFF` to turn off an SCC-based optimization of deciding
language inclusion and use `-DENABLE_CONTEXT_REDUNDANCY_OPT=OFF` to turn off
redundant contexts pruning in the same algorithm.

To compile the code with link-time (i.e., inter-procedural) optimizations,
use the option `-DENABLE_IPO=ON`.

Once compiled, you can run tests with calling `make test`.


#### Building with VAMOS integration

First build [VAMOS](https://github.com/ista-vamos/vamos).
Then run cmake with these parameters:

```
cmake . -Dvamos_DIR=/path/to/vamos/directory
make -j4
```

### Using old makefile

To build the project with the old makefile, run

```
make -f Makefile.legacy
```

This makefile does not support integration with VAMOS.

## Using QuAK

To use the library in your program, use the following directive:
```cpp
TODO
```

For a sample program that puts together the demonstrations below, see *sampleProgram.cpp*.

Below, we consider two $\mathsf{Val}$ automata $\mathcal{A}$ and $\mathcal{B}$ with a rational number $v \in \mathbb{Q}$.

### Input Format

QuAK reads and constructs automata from text files.
Each automata is represented as a list of transitions of the following format:
```
a : v, q -> p
```
which encodes a transition from state $q$ to state $p$ with letter $a$ and weight $v$.

The initial state of the input automaton is the source state of the first transition in its text file.

**Important:** QuAK requires that its input automata are total (a.k.a. complete), i.e., for every state $q$ and every letter $a$, there is at least one outgoing transition from $q$ with letter $a$.

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
weight_t top = A->getBotValue(Val);
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

### Monitor Construction and Execution
QuAK can contsruct monitors from deterministic automata by either reading them from a file or copying an automaton object:
```cpp
Monitor* M = new Monitor("A.txt", Val);
```
```cpp
Monitor* M = new Monitor(A, Val);
```

The monitor updates its state by reading a letter:
```cpp
TODO
```
<!-- ```cpp
M->read(letter);
``` -->

At any point, the monitor can provide the highest and lowest values achievable from the current state of its input automaton:
```cpp
weight_t h = M->getHighest(); 
weight_t l = M->getLowest();
```
