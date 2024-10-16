#!/bin/bash

set -e

DIR=$(dirname $0)/

# we assume the docker build, adjust if necessary
BUILD=$(dirname $0)/../build

mkdir -p results

for VALUE_FUN in LimInfAvg LimSupAvg; do
for S in 2-sym; do
	echo ""
	echo " -- Running experiments for $VALUE_FUN with $S alphabet --"

	python3 "$DIR/run-constant.py" --dir $DIR/automata-$S\
		--out "results/$VALUE_FUN-$S-constant.csv"\
		--value-fun $VALUE_FUN\
		--bindir="$BUILD/experiments"\
		$@ 

	echo "## results stored into 'results/$VALUE_FUN-$S-constant.csv'"
done
done
