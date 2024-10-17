#!/usr/bin/env python
# coding: utf-8

# In[10]:


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
from math import floor


TIMEOUT=100000
MAXTIME=101000

rcParams.update({'font.size': 12})


TO = TIMEOUT * 1.2
def read_csv(path):
    print("Reading", path)
    D = pd.read_csv(path, sep=" ", on_bad_lines='warn')
    D.columns = ["A", "value_fun", "status", "verdict",
                 "states", "edges", "cputime", "returncode"]

    D = D.replace({"None": None})

    D["states"] = D["states"].astype(float)
    D["edges"] = D["edges"].astype(float)
    D["size"] = D["states"] + D["edges"]


    D["cputime"] = D["cputime"].astype(float) / 1000
    global TO
    TO *= 1.7
    D["cputime"].fillna(value=TO, inplace=True)

    D["opt-ctx"] = True if '-ctx-' in path else False
    D["opt-scc"] = False if 'no-scc' in path else True

    D["value_fun"] = D["value_fun"].replace("Infim", "Inf")

    return D


data_frames = []
for path in sys.argv[:1]:
    D = read_csv(path)
    data_frames.append(D)
data = pd.concat(data_frames, ignore_index=True)


# In[14]:


def gen_plot(ax, D, xcol, ycol, hue, style=None, hue_order=None):
    ax.set(yscale="symlog")
    sns.scatterplot(data=D,
                    x=xcol, y=ycol, hue=hue, style=style,
                    ax=ax, palette="Dark2", alpha=1,
                    hue_order=hue_order
                    )
    xlim = ax.get_xlim()
    return ax.plot([0, xlim[1]], [TIMEOUT*1.1, TIMEOUT*1.1], '--', alpha=0.4, color="black")


# In[16]:


#funs = ["Inf", "Sup", "LimInf", "LimSup", "LimInfAvg", "LimSupAvg"]
funs = ["LimInfAvg", "LimSupAvg"]

D = data[(~data["opt-ctx"]) & data["opt-scc"]]
D = D[D["value_fun"].isin(funs)]
D = D[~D["size"].isna()]

print(f"===== {funs} =====")

fig, ax = plt.subplots(1, 1, figsize=(10, 4))
plot = gen_plot(ax, D, "edges", "cputime", "value_fun",
                hue_order=funs)

ax.set(xlabel='# edges', ylabel='CPU time [ms]')

ax.legend(title="Value function")
ax.set_yticks([10**x for x in range(1, 6)])



fig.savefig(f"../plots/constant-function.pdf", bbox_inches='tight', dpi=300)


# In[ ]:




