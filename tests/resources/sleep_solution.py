#!/usr/bin/python3

import sys
import time


if __name__ == '__main__':
    inp = ''
    while inp != 'EXIT':
        inp = input()
        time.sleep(float(inp))
        print(inp)
        sys.stdout.flush()
