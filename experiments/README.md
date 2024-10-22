## Artifact

The attached dockerfile builds quak in docker and sets up it to run the
experiments presented in the paper ,,Quak: Quantitative Automata Kit''.

To build the artifact, _go to the parent directory_ (the top-level directory of
Quak), and run

```
docker build . -t quak -f experiments/Dockerfile
```

Building the artifact takes approximately 2 minutes on a laptop with 4 cores @ 2.8GHz.
Note that the set of random automata is generated anew everytime the docker
image is build, which may slightly influence the results of the experiments.

You can run all the experiments with the following command:

```
docker run --rm -ti -v "$(pwd)/results":/opt/quak/experiments/results quak ./run-all.sh
```

Or you can run the experiments one by one by issuing the following commands:

```
# run the inclusion experiments (Figure 2 and Figure 3)
docker run --rm -ti -v "$(pwd)/results":/opt/quak/experiments/results quak ./run-inclusion.sh

# run the constant-check experiments (Figure 4)
docker run --rm -ti -v "$(pwd)/results":/opt/quak/experiments/results quak ./run-constant.sh

# run the drone simulation (Figure 5, table)
docker run --rm -ti -v "$(pwd)/results":/opt/quak/experiments/results quak ./run-drone.sh
```

Running the inclusion experiments takes many hours on a multi-core system. If
you want to run the experiments only on a subset of automata, you can specify
the option `--num=X`, where `X` is the number of automata. Then, the
experiments will run only on the first `X` generated automata, which means
`X*X` configurations are going to be executed.
```
docker run --rm -ti -v "$(pwd)/results":/opt/quak/experiments/results quak ./run-inclusion.sh --num=3

# you can use `--num` also with `run-all.sh`
docker run --rm -ti -v "$(pwd)/results":/opt/quak/experiments/results quak ./run-all.sh --num=3
```

Running with `--num=15` should take about 1 hour with 4 cores @ 2.8GHz.


### Random automata

Random automata are generated always when the docker image is built,
so in order to re-generate the automata, you can re-build the docker image.
Alternatively, you can start a docker container and re-build the automata
using `gen-aut.sh` script in the `experiments/` directory.
Note that the re-generated automata will be available only in this container.

## Description of the binaries

Note that not all the binaries are used by the artifact built by Dockerfile.

- `inclusion` takes two automata (in .txt) on input and the value function (see
  help), computes the inclusion and measures the time
- `measure-inclusion-rand` takes several parameters (see -help) and generates a
  bunch of random automata and measures the inclusion on them
