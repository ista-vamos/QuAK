#!/bin/bash

set -e

DIR=$(dirname $0)/

mkdir -p results

date

./run-inclusion.sh $@
./run-constant.sh $@
./run-drone.sh $@

date