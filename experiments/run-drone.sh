#!/bin/bash

set -e

DIR=$(dirname $0)/

# we assume the docker build, adjust if necessary
BUILD=$(dirname $0)/../build

mkdir -p results
echo  "" > "results/drone.txt"

for i in `seq 1 3`; do
echo "================================================================" | tee -a "results/drone.txt"
echo " trial $i" | tee -a "results/drone.txt"
echo "================================================================" | tee -a "results/drone.txt"
for SD in 1 5; do
	echo ""
	echo " -- Running experiments for sd=$SD --" | tee -a "results/drone.txt"
	echo ""
	python3 "$DIR/drone_sim.py" --sd=$SD | tee -a "results/drone.txt"

	echo "-----"
	$BUILD/quak $DIR/drone-monitor.txt monitor Avg forces.txt > monitor-sd$SD.log
	$BUILD/quak $DIR/drone-monitor.txt monitor Avg forces-smooth.txt > monitor-sd$SD-smooth.log
	SCORE=$(tail -n 2 monitor-sd$SD.log | head -n 1 | cut -d ' ' -f 3)
	echo "Monitor score random: $SCORE" | tee -a "results/drone.txt"
	SCORE=$(tail -n 2 monitor-sd$SD-smooth.log | head -n 1  | cut -d ' ' -f 3)
	echo "Monitor score smoothed: $SCORE" | tee -a "results/drone.txt"
	echo "-----"
done
done

python3 $DIR/drone-traj.py $DIR/traj.txt
python3 $DIR/drone-tab.py results/drone.txt > results/fig5-tab.txt
