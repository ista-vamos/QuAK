#!/bin/bash

# we assume the docker build, adjust if necessary
BUILD=$(dirname $0)/../build
BUILD_NO_SCC=$(dirname $0)/../build-no-scc

mkdir -p results

for VALUE_FUN in Sup LimSup; do
for S in 2-sym; do
	python3 $BUILD/experiments/run-inclusion.py --dir experiments/automata-$S --out results/$VALUE_FUN-$S --value-fun $VALUE_FUN $@
done
done
