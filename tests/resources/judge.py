#!/usr/bin/python3

import sys
import time
import math
import traceback


def _escape(obj):
    obj = str(obj)
    buf = []
    transform = {' ': '\\ ', '\n': '\\n', '\\': '\\\\'}
    for c in obj:
        buf.append(transform.get(c, c))
    return "".join(buf)


def _join(*args):
    return " ".join(_escape(arg) for arg in args)


def _split(line):
    tokens = []
    buf = []
    INIT = 0
    ORDINAL = 1
    ESCAPE = 2
    state = INIT
    transform = {'\\': '\\', 'n': '\n', ' ': ' '}
    for c in line:
        if state in {INIT, ORDINAL}:
            if c == '\\':
                state = ESCAPE
            elif c in {' ', '\n'}:
                tokens.append("".join(buf))
                buf = []
            else:
                buf.append(c)
        else:
            assert state == ESCAPE, "Logic error"
            assert c in transform, "Unknown character \"{}\"".format(c)
            buf.append(transform[c])
            state = ORDINAL
    assert state != ESCAPE, "Last character may not be escape"
    if len(buf):
        tokens.append("".join(buf))
    return tokens


class ResourceLimits(object):

    __slots__ = ('time_limit_millis', 'real_time_limit_millis')

    def __init__(self, time_limit_millis, real_time_limit_millis):
        self.time_limit_millis = time_limit_millis
        self.real_time_limit_millis = real_time_limit_millis

    def __str__(self):
        return '{} {}'.format(self.time_limit_millis, self.real_time_limit_millis)

    def __repr__(self):
        return 'ResourceLimits(time_limit_millis={}, real_time_limit_millis={})'.format(
            self.time_limit_millis, self.real_time_limit_millis)


class Result(object):

    __slots__ = ('status', 'data')

    def __init__(self, status, data):
        self.status = status
        self.data = data

    @staticmethod
    def from_line(line):
        return Result(*_split(line))

    def __repr__(self):
        return "Result(status='{}', data='{}')".format(
            self.status, self.data)


def log(*args, **kwargs):
    kwargs["file"] = sys.stderr
    ftime, itime = math.modf(time.time())
    _1 = time.strftime("%Y-%b-%d %T", time.localtime(itime))
    _2 = str(ftime)[2:8]
    tb = traceback.extract_stack()[-2]
    line = " -> ".join(map(str.strip, traceback.format_list([tb])[0].strip().split('\n')))
    print("[{}.{}] INFO [{}] -".format(_1, _2, line), *args, **kwargs)


def terminate(id):
    log("Terminating", id)
    print("TERMINATE", id)
    sys.stdout.flush()


def _begin_1(id):
    log("Begin", id)
    print("BEGIN", id)
    sys.stdout.flush()


def _begin_2(id, tokenizer_argument):
    log("Begin", [id, tokenizer_argument])
    print("BEGIN", id, _escape(tokenizer_argument))
    sys.stdout.flush()


def _begin_3(id, tokenizer_argument, resource_limits):
    log("Begin", id, [tokenizer_argument, resource_limits])
    assert isinstance(resource_limits, ResourceLimits),\
        "resource_limits should be the instance of ResourceLimits"
    print("BEGIN", id, _escape(tokenizer_argument), _escape(resource_limits))


def begin(id, *args):
    return [_begin_1, _begin_2, _begin_3][len(args)](id, *args)


def send(id, msg):
    log("Send", [id, msg])
    print("SEND", id, _escape(msg))
    sys.stdout.flush()


def end(id, discardRemaining=None):
    print("END", id, end='')
    if discardRemaining is not None:
        log("End", id, discardRemaining)
        print('', discardRemaining and 1 or 0)
    else:
        log("End", id)
        print()
    sys.stdout.flush()
    result = Result.from_line(sys.stdin.readline())
    log("Result.status =", result.status)
    return result
