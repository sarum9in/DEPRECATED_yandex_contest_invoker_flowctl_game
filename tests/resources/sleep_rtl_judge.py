#!/usr/bin/python3

from judge import *


if __name__ == '__main__':
    log(sys.argv)
    solutions = int(sys.argv[1])
    for i in range(solutions):
        begin(i, '\n', ResourceLimits(real_time_limit_millis=500))
        send(i, '0.1\n')
        result = end(i)
        assert result.status == 'OK'
        assert result.data == '0.1\n'
        begin(i, '\n', ResourceLimits(real_time_limit_millis=500))
        send(i, '1\n')
        assert end(i).status == 'REAL_TIME_LIMIT_EXCEEDED'
        begin(i, '\n')
        send(i, '0\n')
        assert end(i).status == 'EOF_ERROR'
