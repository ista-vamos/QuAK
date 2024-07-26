import argparse
from random import randrange, uniform

parser = argparse.ArgumentParser()
parser.add_argument("--alphabet", action='store', required=True)
parser.add_argument("--states-num", action='store', type=int, required=True)
parser.add_argument("--edges-num", action='store', type=int, default=0)
parser.add_argument("--max-weight", action='store', type=float, required=True)
parser.add_argument("--min-weight", action='store', type=float, required=True)
parser.add_argument("--constant", action='store_true')
args = parser.parse_args()

args.alphabet = args.alphabet.split(",")

states_num = args.states_num
minw, maxw = args.max_weight, args.min_weight
alphabet = args.alphabet

edges_num = 0

if args.constant:
    constant_weight = uniform(minw, maxw)

def get_weight(minw, maxw):
    if args.constant:
        return constant_weight
    return uniform(minw, maxw)

for state in range(0, states_num):
    for symbol in alphabet:
        target = randrange(0, states_num)
        weight = get_weight(minw, maxw)
        print(f"{symbol}: {weight:0.3f}, q{state} -> q{target}")
        edges_num += 1

while edges_num < args.edges_num:
    source = randrange(0, states_num)
    target = randrange(0, states_num)
    weight = get_weight(minw, maxw)
    symbol = alphabet[randrange(0, len(alphabet))]
    print(f"{symbol}: {weight:0.3f}, q{state} -> q{target}")
    edges_num += 1




