#!/bin/bash

set -e

DIR=$(dirname $0)/

# we assume the docker build, adjust if necessary
BUILD=$(dirname $0)/../build
BUILD=$(dirname $0)/../

mkdir -p results

for SD in 1 5; do
	echo ""
	echo " -- Running experiments for sd=$SD --"
	python3 "$DIR/drone-sim.py" --sd=$SD $@ | tee -a "results/drone.txt"
	$BUILD/quak $DIR/drone-monitor.txt monitor Avg forces.txt > monitor-sd$SD.log
	$BUILD/quak $DIR/drone-monitor.txt monitor Avg forces-smooth.txt > monitor-sd$SD-smooth.log
 	tail -n 2 monitor-sd$SD.log | head -n 1
 	tail -n 2 monitor-sd$SD-smooth.log | head -n 1
done
