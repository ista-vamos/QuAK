#!/usr/bin/env python
# coding: utf-8

# In[25]:


import sys
import csv
from matplotlib import rcParams
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from statistics import mean
import pandas as pd
import seaborn as sns
from os.path import basename, dirname, join
from os import listdir
from math import sqrt


# In[113]:


orig = pd.read_csv("traj.txt", sep=",")
orig.columns = ["x", "y"]

N = 500
orig = orig.head(N)
print(orig)


# In[124]:


# Inspired by code from https://medium.com/@jaems33/understanding-robot-motion-path-smoothing-5970c8363bc4
def smooth(path, weight_data=0.5, weight_smooth=0.1, tolerance=0.001):
    new = path[:]
    change = tolerance

    while change >= tolerance:
        change = 0.0
        for i in range(1, len(new) - 1):
                old_x = path[i][0]
                old_y = path[i][1]

                new_x, x_prev, x_next = new[i][0], new[i - 1][0], new[i + 1][0]
                new_y, y_prev, y_next = new[i][1], new[i - 1][1], new[i + 1][1]
                new_x_saved = new_x
                new_y_saved = new_y

                new_x += weight_data * (old_x - new_x) + weight_smooth * (x_next + x_prev - (2 * new_x))
                new_y += weight_data * (old_y - new_y) + weight_smooth * (y_next + y_prev - (2 * new_y))

                new[i] = (new_x, new_y)

                change += abs(new_x - new_x_saved)
                change += abs(new_y - new_y_saved)
        #print('change:', change)
    return new


# In[137]:


fig, ax = plt.subplots(1, 1, figsize=(4, 4))
ax.plot(orig["x"], orig["y"], color="blue", alpha=0.4)
S = smooth(list(orig.itertuples(index=False)), weight_data=0.01, weight_smooth=0.95)
ax.plot([p[0] for p in S], [p[1] for p in S], color="red", linewidth=2)


fig.savefig(f"../plots/drone.pdf", bbox_inches='tight', dpi=300)


# In[ ]:




