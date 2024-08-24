#!/usr/bin/env python3

from tempfile import mkdtemp
from subprocess import Popen, PIPE, DEVNULL, run as runcmd, TimeoutExpired
from os.path import dirname, realpath, basename, abspath, join, isfile, isdir
from os import listdir, access, X_OK, environ as ENV, symlink, makedirs
from sys import argv, stderr
from multiprocessing import Pool, Lock
import argparse


ABORT_ON_ERROR = True

def errlog(*args):
    with open(join(dirname(__file__), "log.txt"), "a") as logf:
        for a in args:
            print(a, file=logf)

def run_one(arg):
    A1, A2, value_fun, args = arg

    r1 = run_inclusion(A1, A2, value_fun, args)
    s1, i1 = r1[3], r1[5]

    r2 = run_inclusion(A1, A2, value_fun, args, booleanize=True)
    s2, i2 = r2[3], r2[5]

    if s1 == "DONE" and s2 == "DONE" and i1 != i2:
        print("\033[1;31m-- Different result on ", A1, A2, "\033[0m", file=stderr)
        if ABORT_ON_ERROR:
            exit(1)
    return r1, r2

def run_inclusion(A1, A2, value_fun, args, booleanize=False):

    inclusion_binary = join(args.bindir, "inclusion")
    cmd = [inclusion_binary, A1, A2, value_fun]
    if booleanize:
        cmd.append("booleanize")

    #print(cmd)
    status = "FAIL"
    p = Popen(cmd, stderr=PIPE, stdout=PIPE)#, cwd=arg.dir)
    try:
        out, err = p.communicate(timeout=args.timeout)
    except TimeoutExpired:
        status = "TIMEOUT"
        p.kill()
        out, err = p.communicate()
        assert p.returncode != 0, p
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

    return (A1, A2, value_fun, status, booleanize,
          data.get('included'),
          data.get('A1-states'), data.get('A1-edges'),
          data.get('A2-states'), data.get('A2-edges'),
          data.get('cputime'),
          p.returncode)

def list_automata(automata_dir, num):
    n = 0
    for a in listdir(automata_dir):
        if not a.endswith(".txt"):
            continue
        if num is not None and n >= num:
            break

        yield a
        n += 1


def get_params(args):
    automata_dir, value_fun = args.dir, args.value_fun

    for f1 in list_automata(automata_dir, args.num):
        for f2 in list_automata(automata_dir, args.num):
            yield f"{automata_dir}/{f1}", f"{automata_dir}/{f2}", value_fun, args


def run_all(args):
    N = sum(1 for _ in list_automata(args.dir, args.num))**2
    n = 0
    with open(args.out, "w") as out,\
         Pool(processes=args.j) as pool:
        for r1, r2 in pool.imap_unordered(run_one, get_params(args)):
            print(",".join(map(str, r1)), file=out)
            print(",".join(map(str, r2)), file=out)
            n += 1
            print(f"Executed {n} configs ({100*(n/N) : 5.2f}%).", end="\r")
        print("\nAll done!")


parser = argparse.ArgumentParser()
parser.add_argument("-j", metavar="PROC_NUM", action='store', type=int)
parser.add_argument("--dir", help="Take automata from this dir.", action='store', required=True)
parser.add_argument("--bindir", help="Use binaries from this dir.", action='store',
                    default=f"{dirname(realpath(__file__))}")
parser.add_argument("--out", help="Output file", action='store', required=True)
parser.add_argument("--value-fun", help="Value function: Sup, Inf, ...", action='store', required=True)
parser.add_argument("--timeout", help="The timeout for one run (wall time)", action='store', type=int)
parser.add_argument("--num", help="Number of automata to use", action='store', type=int)
args = parser.parse_args()

args.dir = abspath(args.dir)

run_all(args)
