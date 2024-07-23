from gen_smooth import ALPHABET, get_weight
from sys import argv
from random import randrange

last = "+0|+0"
for n in range(0, int(argv[1])):
    idx = randrange(0, len(ALPHABET))
    mins = {a: get_weight(last, a) for a in ALPHABET}
    a = min(mins, key=mins.get)
    if randrange(0, 2) == 1:
        a = ALPHABET[idx]
    print(a)
