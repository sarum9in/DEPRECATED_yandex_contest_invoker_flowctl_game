#!/usr/bin/python3

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
