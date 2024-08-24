#!/bin/bash

set -e

DIR=$(dirname $0)/

# we assume the docker build, adjust if necessary
BUILD=$(dirname $0)/../build
BUILD_NO_SCC=$(dirname $0)/../build-no-scc

mkdir -p results

for VALUE_FUN in Sup LimSup; do
for S in 2-sym; do
	python3 "$DIR/run-inclusion.py" --dir $DIR/automata-$S --out results/$VALUE_FUN-$S --value-fun $VALUE_FUN --bindir="$BUILD/experiments" $@
	python3 "$DIR/run-inclusion.py" --dir $DIR/automata-$S --out results/$VALUE_FUN-$S --value-fun $VALUE_FUN --bindir="$BUILD_NO_SCC/experiments" $@
done
done
