# QuAK: Quantitative Automata Kit

QuAK is an open source C++ library that helps automate the analysis of quantitative automata.
Currently, QuAK supports the classes of quantitative automata with the following value functions:
$\mathsf{Inf}, \mathsf{Sup}, \mathsf{LimInf}, \mathsf{LimSup}, \mathsf{LimInfAvg}, \mathsf{LimSupAvg}$.

Given two quantitative automata $\mathcal{A}$ and $\mathcal{B}$ with a rational number $v \in \mathbb{Q}$, QuAK is able to solve the following problems (whenever known to be computable):
1. Check if $\mathcal{A}$ is non-empty with respect to $v$.
2. Check if $\mathcal{A}$ is universal with respect to $v$.
3. Check if $\mathcal{A}$ is included in $\mathcal{B}$.
4. Check if $\mathcal{A}$ defines a constant function.
5. Check if $\mathcal{A}$ defines a safety property.
6. Check if $\mathcal{A}$ defines a liveness property.
7. Compute the top value $\top$ of $\mathcal{A}$.
8. Compute the bottom value $\mathcal{B}ot$ of $\mathcal{A}$.
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

For debug builds, use `Debug` instead of `Release`.


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

For a sample program that includes everything below, see *TODO*

### Input Format

QuAK takes reads automata from text files.
Each automata is represented as a list of transitions of the following format:
```
a : v, q -> p
```
which encodes a transition from $q$ to $p$ with letter $a$ and weight $v$.

The initial state of the input automaton is the source state of the first transition in its text file.

**Important:** QuAK requires that its input automata are total (a.k.a. complete), i.e., for every state $q$ and every letter $a$, there is at least one outgoing transition from $q$ with letter $a$.

### Constructing Automata
TODO

### Non-emptiness Check
TODO

### Universality Check
TODO

### Inclusion Check
TODO

### Constant-function Check
TODO

### Safety Check
TODO

### Liveness Check
TODO

### Top-value Computation
TODO

### Bottom-value Computation
TODO

### Safety Closure Construction
TODO

### Safety-Liveness Decomposition
TODO

### Monitor Construction and Execution
TODO