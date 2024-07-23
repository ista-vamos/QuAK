import sys
from re import compile

from gen_smooth import get_symbol, BOUNDS


def read_events(out):
    R = compile('[(](.*)/(.*)[)]')

    for line in sys.stdin:
        try:
            #  event15  POINTER_MOTION          +20.384s	  1.00/  0.00 ( +1.00/ +0.00)
            M = R.search(line)
            x, y = M[1], M[2]
            x = int(float(x))
            y = int(float(y))
            if x > BOUNDS[1]: x = BOUNDS[1]
            if x < BOUNDS[0]: x = BOUNDS[0]
            if y > BOUNDS[1]: y = BOUNDS[1]
            if y < BOUNDS[0]: y = BOUNDS[0]

            print(get_symbol(x, y), file=out)
            out.flush()
        except Exception as e:
            print("FAILED for:", file=sys.stderr)
            print(line, file=sys.stderr)
            raise e

try:
    with open(sys.argv[1], 'w') as out:
        read_events(out)
except (BrokenPipeError, KeyboardInterrupt):
    exit(0)
