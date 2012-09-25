#!/usr/bin/python3

from judge import *


if __name__ == '__main__':
    log(sys.argv)
    solutions = int(sys.argv[1])
    for i in range(solutions):
        begin(i, '\n')
        send(i, 'Hello,')
        send(i, ' ')
        send(i, 'world!\n')
        result = end(i)
        assert result.status == 'OK'
        assert result.data == 'Hello, world!\n'
        begin(i, '\n')
        assert end(i).status == 'EOF_ERROR'
