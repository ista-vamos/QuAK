#!/usr/bin/env python3

from tempfile import mkdtemp
from subprocess import Popen, PIPE, DEVNULL, run as runcmd, TimeoutExpired
from os.path import dirname, realpath, basename, abspath, join, isfile, isdir
from os import listdir, access, X_OK, environ as ENV, symlink, makedirs
from sys import argv, stderr
from multiprocessing import Pool, Lock
import argparse


bindir = f"{dirname(realpath(__file__))}/"
inclusion_binary = join(bindir, "inclusion")

ABORT_ON_ERROR = True
TIMEOUT = 10

def errlog(*args):
    with open(join(dirname(__file__), "log.txt"), "a") as logf:
        for a in args:
            print(a, file=logf)

def run_one(arg):
    A1, A2, value_fun = arg

    # run our monitor
    s1, i1 = run_inclusion(A1, A2, value_fun)

    s2, i2 = run_inclusion(A1, A2, value_fun, booleanize=True)

    if s1 == "DONE" and s2 == "DONE" and i1 != i2:
        with lock:
            print("\033[1;31m-- Different result on ", A1, A2, "\033[0m", file=stderr)
            if ABORT_ON_ERROR:
                exit(1)

def run_inclusion(A1, A2, value_fun, booleanize=False):

   #with lock:
   #    print("\033[1;32m-- Running on ", A1, A2, "\033[0m", file=stderr)
    cmd = [inclusion_binary, A1, A2, value_fun]
    if booleanize:
        cmd.append("booleanize")

    #print(cmd)
    status = "FAIL"
    p = Popen(cmd, stderr=PIPE, stdout=PIPE)#, cwd=arg.dir)
    try:
        out, err = p.communicate(timeout=TIMEOUT)
    except TimeoutExpired:
        status = "TIMEOUT"
        p.kill()
        out, err = p.communicate()
        assert p.returncode != 0, p
    # assert out is not None, cmd
    assert err is not None, cmd

    data = dict()
    if p.returncode in (0, 1):
        status = "DONE"

    for line in out.splitlines():
        line = line.strip()
        if line.startswith(b"Is included"):
            data['included'] = int(line.split()[2]) == 1
        elif line.startswith(b"Cputime"):
            data['cputime'] = int(line.split()[1])
        elif line.startswith(b"A1 states"):
            nums = line.split()[2].split(b",")
            data['A1-states'], data['A1-edges'] = int(nums[0]),int(nums[1])
        elif line.startswith(b"A2 states"):
            nums = line.split()[2].split(b",")
            data['A2-states'], data['A2-edges'] = int(nums[0]),int(nums[1])

    # pandas takes Inf as infinity, so rename it
    if value_fun == "Inf":
        value_fun = "Infim"

    with lock:
        print(A1, A2, value_fun, status, booleanize,
              data.get('included'),
              data.get('A1-states'), data.get('A1-edges'),
              data.get('A2-states'), data.get('A2-edges'),
              data.get('cputime'),
              p.returncode)

    return status, data.get('included')


def get_params(automata_dir, value_fun):
    for f1 in listdir(automata_dir):
        for f2 in listdir(automata_dir):
            if f1.endswith(".txt") and f2.endswith(".txt"):
                #for value_fun in ("Sup", "Inf"):
                yield f"{automata_dir}/{f1}", f"{automata_dir}/{f2}", value_fun

def init_lock(l):
    global lock
    lock = l




def run_all(args):
    #print(f"\033[1;34mRunning trace_len={trace_len}, bits={bits} [using {args.j} workers]\033[0m", file=stderr)

    lock = Lock()

    with Pool(processes=args.j, initializer=init_lock, initargs=(lock,)) as pool:
        result = pool.map(run_one, get_params(args.dir, args.value_fun))

parser = argparse.ArgumentParser()
parser.add_argument("-j", metavar="PROC_NUM", action='store', type=int)
parser.add_argument("--dir", help="Take automata from this dir.", action='store', required=True)
parser.add_argument("--value-fun", help="Value function: Sup, Inf, ...", action='store', required=True)
parser.add_argument("--timeout", help="The timeout for one run (wall time)", action='store', type=int)
args = parser.parse_args()

args.dir = abspath(args.dir)
if args.timeout is not None:
    TIMEOUT = args.timeout

run_all(args)
