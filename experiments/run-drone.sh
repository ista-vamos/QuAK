#!/bin/bash

set -e

DIR=$(dirname $0)/

# we assume the docker build, adjust if necessary
BUILD=$(dirname $0)/../build
BUILD=$(dirname $0)/../

mkdir -p results
echo  "" > "results/drone.txt"

for i in `seq 1 3`; do
echo "================================================================" | tee -a "results/drone.txt"
echo " trial $i" | tee -a "results/drone.txt"
echo "================================================================" | tee -a "results/drone.txt"
for SD in 1 5; do
	echo ""
	echo " -- Running experiments for sd=$SD --"
	echo ""
	python3 "$DIR/drone-sim.py" --sd=$SD $@ | tee -a "results/drone.txt"

	echo "-----"
	$BUILD/quak $DIR/drone-monitor.txt monitor Avg forces.txt > monitor-sd$SD.log
	$BUILD/quak $DIR/drone-monitor.txt monitor Avg forces-smooth.txt > monitor-sd$SD-smooth.log
	SCORE=$(tail -n 2 monitor-sd$SD.log | head -n 1 | cut -d ' ' -f 3)
	echo "Monitor score random: $SCORE" | tee -a "results/drone.txt"
	SCORE=$(tail -n 2 monitor-sd$SD-smooth.log | head -n 1  | cut -d ' ' -f 3)
	echo "Monitor score smoothed: $SCORE" | tee -a "results/drone.txt"
done
done