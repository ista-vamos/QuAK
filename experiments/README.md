## Artifact

The attached dockerfile builds quak in docker and sets up it to run the experiments
presented in the paper ,,Quak: Quantitative Automata Kit''.

To build the artifact, _go to the parent directory_ (the top-level directory of Quak),
and run

```
docker build . -t quak -f experiments/Dockerfile
```

Note that the set of random automata is generated anew everytime the docker
image is build, which may _slightly_ influence the results of the experiments.


## Description of the binaries

Note that not all the binaries are used by the artifact built by Dockerfile.

- `inclusion` takes two automata (in .txt) on input and the value function (see help), computes the inclusion and measures the time
- `measure-inclusion-rand` takes several parameters (see -help) and generates a bunch of random automata and measures the inclusion on them
