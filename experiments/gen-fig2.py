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
OUTDIR=join(dirname(__file__), 'results')

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

    D["opt-scc"] = False if '-no-scc-' in path else True

    #D["cputime"] = data_nost["cputime"].replace("None", None).astype(float)

    return D


data_frames = []
for path in sys.argv[1:]:
    D = read_csv(path)
    data_frames.append(D)
data = pd.concat(data_frames, ignore_index=True) #.dropna()

# Fix MAXTIME and TIMEOUT
TIMEOUT=100000
MAXTIME=102000


value_fun = "Sup"
print("-- Generating scatter for ", value_fun)
D = D[D["value_fun"] == value_fun]
D = D[D["opt-scc"]]

print("Rows: ", len(D))
print("----")
print(D["A1-states"].value_counts())
print("----")
print(D["A2-states"].value_counts())
print("=====")


D1 = D[D["booleanized"] == False]
D2 = D[D["booleanized"] == True]

sel_data = D1.merge(D2, on="A1A2", how="inner", suffixes=["_antichains", "_bool"])

print(sel_data)


assert len(sel_data) <= len(D1) and len(sel_data) <= len(D2), (len(sel_data), len(D1), len(D2))

fig, ax = plt.subplots(1, 1, figsize=(4, 4))
ax.set(yscale="symlog",xscale="symlog")
ax.set_aspect('equal', 'box')
#ax[1].set(yscale="symlog")
#ax.set(yscale="symlog",xscale="symlog")
# -------------------------
xcol = "cputime_antichains"
ycol = "cputime_bool"

tmp = sel_data["cputime_antichains"] <= sel_data["cputime_bool"]
print("Number of values where antichains was faster")
print(tmp.value_counts(normalize=True))
print("------")

D = sel_data[(sel_data["verdict_antichains"] == False) | (sel_data["verdict_bool"] == False)]
print(len(D))
plt.scatter(D["cputime_antichains"], D["cputime_bool"],
            marker='o', color="orange", alpha=0.3)

D = sel_data[(sel_data["verdict_antichains"] == True) | (sel_data["verdict_bool"] == True)]
print(len(D))

plt.scatter(D["cputime_antichains"], D["cputime_bool"],
            marker='x', color="green", alpha=0.6)


plt.plot([0, MAXTIME], [0, MAXTIME], 'k-', alpha=0.75)
plt.plot([0, MAXTIME], [TIMEOUT, TIMEOUT], '--', alpha=0.5, color="cornflowerblue")
plt.plot([TIMEOUT, TIMEOUT], [0, TIMEOUT], '--', alpha=0.5, color="cornflowerblue")
ax.set(xlabel='Antichains [ms]', ylabel='Standard [ms]',
       title=f'{value_fun_name(value_fun)} automata')
fig.tight_layout()
fig.savefig(f"{OUTDIR}/fig2-sup.pdf", bbox_inches='tight', dpi=600)

##
# ----------------------------------------------------------------------------------------------
##


value_fun = "LimSup"
print("-- Generating scatter for ", value_fun)
D = data[data["value_fun"] == value_fun]
D = D[D["opt-scc"]]

print("Rows: ", len(D))
print("----")
print(D["A1-states"].value_counts())
print("----")
print(D["A2-states"].value_counts())
print("=====")


print(D["booleanized"].value_counts())
print("Verdict distr:")
print(D["verdict"].value_counts())
print("xxxxxxxxxxxxxxxxx")

D1 = D[D["booleanized"] == False]
D2 = D[D["booleanized"] == True]
print("----")
sel_data = D1.merge(D2, on="A1A2", how="inner", suffixes=["_antichains", "_bool"])
assert len(sel_data) <= len(D1) and len(sel_data) <= len(D2)


tmp = sel_data["cputime_antichains"] <= sel_data["cputime_bool"]
print("Number of values where antichains was faster")
print(tmp.value_counts(normalize=True))
print("------")

fig, ax = plt.subplots(1, 1, figsize=(4, 4))
ax.set(yscale="symlog", xscale="symlog")
ax.set_aspect('equal', 'box')
#ax[1].set(yscale="symlog")
#ax.set(yscale="symlog",xscale="symlog")
# -------------------------
xcol = "cputime_antichains"
ycol = "cputime_bool"

tmp = sel_data["cputime_antichains"] <= sel_data["cputime_bool"]
print("Number of values where antichains was faster")
print(tmp.value_counts(normalize=True))
print("------")

D = sel_data[(sel_data["verdict_antichains"] == False) | (sel_data["verdict_bool"] == False)]
#print(len(D))
plt.scatter(D["cputime_antichains"], D["cputime_bool"],
            marker='o', color="orange", alpha=0.3)

D = sel_data[(sel_data["verdict_antichains"] == True) | (sel_data["verdict_bool"] == True)]
#print(len(D))

plt.scatter(D["cputime_antichains"], D["cputime_bool"],
            marker='x', color="green", alpha=0.6)


plt.plot([0, MAXTIME], [0, MAXTIME], 'k-', alpha=0.75)
plt.plot([0, MAXTIME], [TIMEOUT, TIMEOUT], '--', alpha=0.5, color="cornflowerblue")
plt.plot([TIMEOUT, TIMEOUT], [0, TIMEOUT], '--', alpha=0.5, color="cornflowerblue")
ax.set(xlabel='Antichains [ms]', ylabel='Standard [ms]',
       title=f'{value_fun_name(value_fun)} automata')
fig.tight_layout()
fig.savefig(f"{OUTDIR}/fig2-limsup.pdf", bbox_inches='tight', dpi=600)


