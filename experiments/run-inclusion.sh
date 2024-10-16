#!/bin/bash

set -e

DIR=$(dirname $0)/

# we assume the docker build, adjust if necessary
BUILD=$(dirname $0)/../build
BUILD_NO_SCC=$(dirname $0)/../build-no-scc

mkdir -p results

for VALUE_FUN in Sup LimSup; do
for S in 2-sym; do
	echo " -- Running experiments for $VALUE_FUN with $S alphabet --"
	python3 "$DIR/run-inclusion.py" --dir $DIR/automata-$S --out "results/$VALUE_FUN-$S.csv" --value-fun $VALUE_FUN --bindir="$BUILD/experiments" $@
	echo "## results stored into 'results/$VALUE_FUN-$S.csv'"

	echo " -- Running experiments for $VALUE_FUN with $S alphabet, no SCC optimization --"
	python3 "$DIR/run-inclusion.py" --dir $DIR/automata-$S --out "results/$VALUE_FUN-$S-no-scc.csv" --value-fun $VALUE_FUN --bindir="$BUILD_NO_SCC/experiments" $@
	echo "## results stored into 'results/$VALUE_FUN-$S-no-scc.csv'"
done
done
