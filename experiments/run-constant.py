#!/usr/bin/env python3

from tempfile import mkdtemp
from subprocess import Popen, PIPE, DEVNULL, run as runcmd, TimeoutExpired
from os.path import dirname, realpath, basename, abspath, join, isfile, isdir
from os import listdir, access, X_OK, environ as ENV, symlink, makedirs
from sys import argv, stderr
from multiprocessing import Pool, Lock
import argparse

lock = Lock()

bindir = f"{dirname(realpath(__file__))}/"
binary = join(bindir, "constant")

ABORT_ON_ERROR = True
TIMEOUT = 10

def errlog(*args):
    with open(join(dirname(__file__), "log.txt"), "a") as logf:
        for a in args:
            print(a, file=logf)

def run_one(arg):
    A, value_fun = arg

    run_constant(A, value_fun)

def run_constant(A, value_fun):

   #with lock:
   #    print("\033[1;32m-- Running on ", A1, A2, "\033[0m", file=stderr)
    cmd = [binary, A, value_fun]

    #print(cmd)
    status = "FAIL"
    p = Popen(cmd, stderr=PIPE, stdout=PIPE)#, cwd=arg.dir)
    try:
        out, err = p.communicate(timeout=TIMEOUT)
    except TimeoutExpired:
        status = "TIMEOUT"
        p.kill()
        out, err = p.communicate()
    #assert p.returncode == 0, p
    # assert out is not None, cmd
    assert err is not None, cmd

    data = dict()
    if p.returncode in (0, 1):
        try:
            status = "DONE"
            for line in out.splitlines():
                line = line.strip()
                if line.startswith(b"Is constant"):
                    data['constant'] = int(line.split()[2]) == 1
                elif line.startswith(b"Cputime"):
                    data['cputime'] = int(line.split()[1])
                elif line.startswith(b"states/"):
                    nums = line.split()[1].split(b",")
                    data['states'], data['edges'] = int(nums[0]),int(nums[1])
        except Exception as e:
            print("ERROR parsing line:", file=stderr)
            print(line, file=stderr)
            raise e

    with lock:
        print(A, value_fun, status,
              data.get('constant'),
              data.get('states'), data.get('edges'),
              data.get('cputime'),
              p.returncode)

    return status, data.get('constant')


def get_params(automata_dir, value_fun):
    for f1 in listdir(automata_dir):
        if f1.endswith(".txt"):
            #for value_fun in ("Sup", "Inf"):
            yield f"{automata_dir}/{f1}", value_fun

def run_all(args):
    #print(f"\033[1;34mRunning trace_len={trace_len}, bits={bits} [using {args.j} workers]\033[0m", file=stderr)

    with Pool(processes=args.j) as pool:
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
