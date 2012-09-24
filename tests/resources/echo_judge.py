#!/usr/bin/python3

from judge import *


if __name__ == '__main__':
    log(sys.argv)
    solutions = int(sys.argv[1])
    for i in range(solutions):
        begin(i, '\n')
        send(i, 'hello,')
        send(i, ' ')
        send(i, 'world!\n')
        assert end(i).status == 'OK'
