#!/usr/bin/env python
# coding: utf-8

# In[1]:


import sys
import csv
from matplotlib import rcParams
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from statistics import mean
import pandas as pd
from os.path import basename, dirname, join
from os import listdir


# In[2]:


TIMEOUT=100000000
MAXTIME=102000000
OUTDIR="."

rcParams.update({'font.size': 12})


# In[3]:


def value_fun_name(nm):
    if nm == "Infim":
        return "Inf"
    return nm

def read_csv(path):
    print("Reading", path)
    D = pd.read_csv(path, sep=",", on_bad_lines='warn')
    D.columns = ["A1", "A2", "value_fun", "status", "booleanized", "verdict",
                 "A1-states", "A1-edges", "A2-states", "A2-edges",
                 "cputime", "returncode"]
    D["A1A2"] = D["A1"] + D["A2"]
    D = D.replace({"None": None})
    #D["cputime"].fillna(value=105000000, inplace=True)
    D.loc[D["status"] == "TIMEOUT", "cputime"] = MAXTIME
    D["cputime"] = D["cputime"].astype(float) / 1000

    D["A1-states"] = D["A1-states"].astype(float)
    D["A2-states"] = D["A2-states"].astype(float)

    #assert ("-4sym" in path) or ("-2sym" in path)
    D["alphabet"] = 4 if "-4sym" in path else 2

    D["opt-scc"] = False if 'no-scc' in path else True

    #D["cputime"] = data_nost["cputime"].replace("None", None).astype(float)

    return D


# Fix MAXTIME and TIMEOUT

TIMEOUT=100000
MAXTIME=102000



data_frames = []
for path in sys.argv[1:]:
    D = read_csv(path)
    data_frames.append(D)
data = pd.concat(data_frames, ignore_index=True) #.dropna()


######################################################################
######################################################################
## Sup automata, optimizations, qq-plot
######################################################################
######################################################################
value_fun = "Sup"
print("-- Generating qq-plot for ", value_fun)
D = data[(data["value_fun"] == value_fun) & (data["booleanized"] == False) & (data["status"] == "DONE")]

D_scc = D[(D["opt-scc"])]
D_no = D[(~D["opt-scc"])]

D = D_scc.merge(D_no, on="A1A2", how="left")


assert(len(D_no) > 0)
assert(len(D_scc) > 0)
#D["verdict"] = D["verdict"].astype(bool)

fig, ax = plt.subplots(1, 1, figsize=(4, 4))


def plot_sorted(D, lab):
    D = D.sort_values(by="cputime", ignore_index=True)
    D["n"] = D.index
    vals = D["cputime"].to_list()
    vals.sort()
    xs = list(range(0, len(vals)))
    return plt.plot(xs, vals, label=lab)

D_no = pd.DataFrame()
D_no["cputime"] = D["cputime_y"] / 1000

D_scc = pd.DataFrame()
D_scc["cputime"] = D["cputime_x"] / 1000
plots = [plot_sorted(D_no, "no opt."), plot_sorted(D_scc, "scc-search")]
plt.legend()

ax.set(xlabel='# instances', ylabel='CPU time [s]')
fig.savefig(f"{OUTDIR}/fig3-qqplot.pdf", bbox_inches='tight', dpi=600)


# In[10]:


TIMEOUT=100
MAXTIME=101


# In[11]:


def gen_scatter(D1, D2, xlab, ylab):

   #D1 = D1[["A1A2", "cputime", "verdict"]]
   #D2 = D2[["A1A2", "cputime", "verdict"]]

    D = D1.merge(D2, on="A1A2", how="inner")
    assert len(D) <= len(D1) and len(D) <= len(D2)

    tmp = D["cputime_x"] <= D["cputime_y"]
    print(f"Number of values where {xlab} was faster than {ylab}")
    print(tmp.value_counts(normalize=True))
    print("------")

    fig, ax = plt.subplots(1, 1, figsize=(4, 4))
    #ax.set(yscale="symlog", xscale="symlog")
    #ax.set_aspect('equal', 'box')

    sel = D[(D["verdict_x"] == True) | (D["verdict_y"] == True)]
    S = pd.DataFrame()
    S["cputime_x"] = sel["cputime_x"].astype(float).div(1000)
    S["cputime_y"] = sel["cputime_y"].astype(float).div(1000)
    plt.scatter(S["cputime_x"], S["cputime_y"],
                marker='x', color="green", alpha=0.6)

    sel = D[(D["verdict_x"] == False) | (D["verdict_y"] == False)]
    S = pd.DataFrame()
    S["cputime_x"] = sel["cputime_x"].astype(float).div(1000)
    S["cputime_y"] = sel["cputime_y"].astype(float).div(1000)
    plt.scatter(S["cputime_x"], S["cputime_y"],
                marker='o', color="orange", alpha=0.3)


    plt.plot([0, MAXTIME], [0, MAXTIME], 'k-', alpha=0.75)
    plt.plot([0, MAXTIME], [TIMEOUT, TIMEOUT], '--', alpha=0.5, color="cornflowerblue")
    plt.plot([TIMEOUT, TIMEOUT], [0, TIMEOUT], '--', alpha=0.5, color="cornflowerblue")
    ax.set(xlabel=xlab, ylabel=ylab)
    fig.savefig(f"{OUTDIR}/fig3-scatter.pdf", bbox_inches='tight', dpi=600)


# In[12]:


#####################################################################

######################################################################
######################################################################
## Sup automata, optimizations, scatter plot
######################################################################
######################################################################
value_fun = "Sup"
print("-- Generating scatter for optimizations of ", value_fun)
D = data[(data["value_fun"] == value_fun) & (data["booleanized"] == False) ]



# In[13]:


D_scc = D[(D["opt-scc"])]
D_no = D[(~D["opt-scc"])]

assert(len(D_no) > 0)
assert(len(D_scc) > 0)
#D["verdict"] = D["verdict"].astype(bool)


# In[14]:


gen_scatter(D_no, D_scc, "no optimization", "scc-search")


# In[ ]:




