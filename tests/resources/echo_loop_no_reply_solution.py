#!/usr/bin/python3

import sys


if __name__ == '__main__':
    while True:
        inp = input()
        if inp == 'EXIT':
            sys.exit()
        print(inp)
        sys.stdout.flush()
