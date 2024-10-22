## Artifact

The attached dockerfile builds quak in docker and sets up it to run the
experiments presented in the paper ,,Quak: Quantitative Automata Kit''.

To build the artifact, _go to the parent directory_ (the top-level directory of
Quak), and run

```
docker build . -t quak -f experiments/Dockerfile
```

Warning: if you have already built QuAK with the Dockerfile from the parent directory,
this command will overwrite the image. To avoid this, change `-t quak` to `-t <new_name>`
where `<new_name>` is a name of the container with experiments. Do not forget to use this new name
when running the experiments through the contanier later.

Building the artifact takes approximately 2 minutes on a laptop with 4 cores @ 2.8GHz.
Note that the set of random automata is generated anew everytime the docker
image is build, which may slightly influence the results of the experiments.

You can run all the experiments with the following command:

```
docker run --rm -ti -v "$(pwd)/results":/opt/quak/experiments/results quak ./run-all.sh
```

All results of the experiments are stored into the directory `results/`.
You can also run the experiments one by one by issuing the following commands:

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

## The structure of the artifact

- `CMakeLists.txt`, `Dockerfile`: configuration for `CMake` and `docker` or `podman`
- `README.md`: this README
- `run-all.sh`: run all experiments, calls `run-inclusion.sh`, `run-constant.sh`, `run-drone.sh`

### Generating automata

- `gen-aut.sh`: the main script to generate random automata, calls `rand_automaton.py`
- `rand_automaton.py`: the script to generate a random automaton

### Inclusion experiments

- `run-inclusion.sh`: main script to run the inclusion experiments, calls `run-inclusion.py`
- `run-inclusion.py`: python script that actually runs the inclusion experiments
- `inclusion`,`inclusion.cpp`: the binary `inclusion`, generated from `inclusion.cpp`,
   takes two automata (stored in files) on input and the value function (see help),
   and it computes the inclusion and measures the time.
- `gen-fig2.py`, `gen-fig3.py`: generate the plots (as PDF files) from figures 2 and 3 from the ISoLA paper.
   These scripts are assumed to be run from inside the docker, otherwise you need to adjust the paths
   inside those scripts. The plots are stored into the directory `results/`.

#### Not used in the artifact
- `measure-inclusion`, `measure-inclusion.cpp` takes several parameters (see -help) and generates a
  bunch of random automata and measures the inclusion on them

### Constant-check experiments

- `run-constant.sh`: the main script to run constant-check experiments, calls `run-constant.py`.
- `run-constant.py`: the script that actually runs the constant-check experiments.
- `constant`, `constant.cpp`: the binary constant takes on input an automaton file and a value function,
                     and computes if the automaton is constant (and measures the time).
                     `constant.cpp` is the source file for `constant`.
- `gen-fig4.py`: generate the plot in Figure 4 from the ISoLA paper. The script is assumed to be run from
                 inside the docker, otherwise you need to adjust the paths inside those scripts.
                 The plots are stored into the directory `results/`.

### Drone simulation/smoothness monitoring
- `run-drone.sh`: the main script to run the experiments with monitoring smoothness of a drone controller,
                  calls `drone_sim.py`.
- `drone_sim.py`: run the drone simulation.
- `gen_smooth.py`: generate the automaton for monitoring smoothness of drone motion, used to generate `drone-monitor.txt`.
- `drone-monitor.txt`: the automaton for monitoring smoothness of drone motion.
- `drone-traj.py`, `drone-tab.py`: scripts to generate Figure 5 from the ISoLA paper. Note that the trajectory plot
                 is going to be the one from the last executed simulation.
                  

#### Not used in the artifact
- `parse-pointer-motion.py`: a script that can be used to generate a trace from a pointer motion
  that can be fed into the smoothness monitor instead of a trace from the drone simulation.
  The monitor that quantifies the smoothness of the pointer motion. Might not be up to date.
