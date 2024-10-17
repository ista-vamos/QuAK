#!/usr/bin/env python3

import sys
from statistics import mean

#  -- Running experiments for sd=1 --
### Drone with random controller
#Avg energy consumption per step:  9.945165742222157
#
### Drone follow smoothed traj
#Avg energy consumption per step:  5.4015550845228155
#Monitor score random: 9.32537
#Monitor score smoothed: 0.202734
# -- Running experiments for sd=5 --
### Drone with random controller
#Avg energy consumption per step:  12.3940337300456
#
### Drone follow smoothed traj
#Avg energy consumption per step:  2.253123074574844
#Monitor score random: 15.9977
#Monitor score smoothed: 0.838714

def get_sd(line):
    sdeq = line.split()[4]
    return int(sdeq.split('=')[1])


results = {}
with open(sys.argv[1], 'r') as f:
    for line in f:
        if 'experiments for sd' in line:
            sd = get_sd(line)
        elif 'Drone with random' in line:
            controller = 'random'
        elif 'Drone follow smoothed' in line:
            controller = 'smooth'
        elif 'Avg energy consumption' in line:
            results.setdefault(sd, {}).setdefault(controller,
                                                  {}).setdefault('ec', []).append(float(line.split()[5]))
        elif 'Monitor score random' in line:
            results.setdefault(sd, {}).setdefault('random',
                                                  {}).setdefault('mon', []).append(float(line.split()[3]))
        elif 'Monitor score smoothed' in line:
            results.setdefault(sd, {}).setdefault('smooth',
                                                  {}).setdefault('mon', []).append(float(line.split()[3]))


print('          Original        Smooth')
print('         EC    Score     EC    Score')
print('--------------------------------------')
for sd, values in results.items():
    R = values['random']
    S = values['smooth']
    print(f'sd={sd} | {mean(R["ec"]) :5.2f} | {mean(R["mon"]):5.2f} | {mean(S["ec"]):5.2f} | {mean(S["mon"]):5.2f}')
