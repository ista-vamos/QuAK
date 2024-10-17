#!/usr/bin/env python
# coding: utf-8

import sys
import matplotlib.pyplot as plt
import pandas as pd
from math import sqrt

from drone_sim import smooth

orig = pd.read_csv(sys.argv[1], sep=",")
orig.columns = ["x", "y"]

N = 500
orig = orig.head(N)


fig, ax = plt.subplots(1, 1, figsize=(4, 4))
ax.plot(orig["x"], orig["y"], color="blue", alpha=0.4)
S = smooth(list(orig.itertuples(index=False)), weight_data=0.01, weight_smooth=0.95)
ax.plot([p[0] for p in S], [p[1] for p in S], color="red", linewidth=2)


fig.savefig(f"results/fig5-trajectory.pdf", bbox_inches='tight', dpi=1200)

