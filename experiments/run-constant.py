#!/usr/bin/env python3

from tempfile import mkdtemp
from subprocess import Popen, PIPE, DEVNULL, run as runcmd, TimeoutExpired
from os.path import dirname, realpath, basename, abspath, join, isfile, isdir
from os import listdir, access, X_OK, environ as ENV, symlink, makedirs
from sys import argv, stderr
from multiprocessing import Pool
import argparse



ABORT_ON_ERROR = True
TIMEOUT = 10

def errlog(*args):
    with open(join(dirname(__file__), "log.txt"), "a") as logf:
        for a in args:
            print(a, file=logf)

def run_one(arg):
    A, value_fun, args = arg

    return run_constant(A, value_fun, args)

def run_constant(A, value_fun, args):
    binary = join(args.bindir, "constant")
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
        assert p.returncode != 0
    #assert p.returncode == 0, p
    # assert out is not None, cmd
    assert err is not None, cmd

    data = dict()
    if p.returncode == 0:
        status = "DONE"
    try:
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

    if value_fun == "Inf":
        value_fun = "Infim"

    return(A, value_fun, status,
           data.get('constant'),
           data.get('states'), data.get('edges'),
           data.get('cputime'),
           p.returncode)

def automata_num(automata_dir):
    return sum(1 for f in listdir(automata_dir) if f.endswith('.txt'))

def get_params(args):
    n = 0
    automata_dir = args.dir
    max_num = args.num
    for f1 in listdir(automata_dir):
        if f1.endswith(".txt"):
            #for value_fun in ("Sup", "Inf"):
            yield f"{automata_dir}/{f1}", args.value_fun, args

            n += 1
            if max_num is not None and n >= max_num:
                break



def run_all(args):
    #print(f"\033[1;34mRunning trace_len={trace_len}, bits={bits} [using {args.j} workers]\033[0m", file=stderr)

    N = (args.num or automata_num(args.dir))
    n = 0
    with Pool(processes=args.j) as pool, open(args.out or '/dev/stdout', "w") as out:
        result = pool.imap_unordered(run_one, get_params(args))
        for r in result:
            print(' '.join(map(str, r)), file=out)
            n += 1
            if args.out:
                print(f"Executed {n} configs ({100*(n/N) : 5.2f}%).", end="\r")
        print("\nAll done!")

parser = argparse.ArgumentParser()
parser.add_argument("-j", metavar="PROC_NUM", action='store', type=int)
parser.add_argument("--dir", help="Take automata from this dir.", action='store', required=True)
parser.add_argument("--value-fun", help="Value function: Sup, Inf, ...", action='store', required=True)
parser.add_argument("--num", help="Number of automata to run on", action='store', default=None)
parser.add_argument("--timeout", help="The timeout for one run (wall time)", action='store', type=int)
parser.add_argument("--out", help="Output file (stdout is used if not given)", action='store', default=None)
parser.add_argument("--bindir", help="Use binaries from this dir.", action='store',
                    default=f"{dirname(realpath(__file__))}")

args = parser.parse_args()

if args.num is not None:
    args.num = int(args.num)

args.dir = abspath(args.dir)
if args.timeout is not None:
    TIMEOUT = args.timeout

run_all(args)
