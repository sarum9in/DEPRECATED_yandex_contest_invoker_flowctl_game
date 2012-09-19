#!/usr/bin/python3

import sys

def log(*args, **kwargs):
    kwargs["file"] = sys.stderr
    print(*args, **kwargs)

if __name__ == '__main__':
    log(sys.argv)
