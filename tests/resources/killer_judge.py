#!/usr/bin/python3

# for python2 compatibility
from __future__ import print_function

import sys
import time
import math


def log(*args, **kwargs):
    kwargs["file"] = sys.stderr
    ftime, itime = math.modf(time.time())
    _1 = time.strftime("%Y-%b-%d %T", time.localtime(itime))
    _2 = str(ftime)[2:8]
    print("[{}.{}] INFO [JUDGE] -".format(_1, _2), *args, **kwargs)


def terminate(id):
    log("Terminating {}".format(id))
    print("TERMINATE {}".format(id))


if __name__ == '__main__':
    log(sys.argv)
    solutions = int(sys.argv[1])
    for i in range(solutions):
        terminate(i)
