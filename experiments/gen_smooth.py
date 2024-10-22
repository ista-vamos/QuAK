# generate the automaton to evaluate the smoothness of drone motion

from math import sqrt

def get_d(symbol):
    idx = symbol.find("|")
    return int(symbol[:idx]), int(symbol[idx+1:])


def get_symbol(dx, dy):
    assert isinstance(dx, int)
    assert isinstance(dy, int)

    sdx = str(dx)
    if dx >= 0:
        sdx = f"+{sdx}"
    sdy = str(dy)
    if dy >= 0:
        sdy = f"+{sdy}"

    r = f"{sdx}|{sdy}"
    assert (dx, dy) == get_d(r), (dx, dy, r)
    return r

BOUNDS = (-10, 10)
ALPHABET = [get_symbol(dx, dy) for dx in range(BOUNDS[0], BOUNDS[1]+1) for dy in range(BOUNDS[0], BOUNDS[1]+1)]

def get_weight(src, a):
    xn, yn = get_d(a)
    xo, yo = get_d(src)

    d1 = xo - xn
    d2 = yo - yn
    return sqrt(d1*d1 + d2*d2)


def gen_transitions(src):
    for a in ALPHABET:
        print(f"{a}, {get_weight(src, a) : .3f}: {src} -> {a}")

if __name__ == "__main__":
    for state in ALPHABET:
        gen_transitions(state)
