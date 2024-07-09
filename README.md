# QuAK: Quantitative Automata Kit

Description: TBD

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

TBD
