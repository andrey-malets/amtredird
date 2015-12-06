#!/usr/bin/env python

import argparse
import msgpack
import socket
import sys


PROTO_VERSION = 1


class Cmd:
    LIST  = 0
    START = 1
    STOP  = 2


class Result:
    OK             = 0
    NO_SUCH_CLIENT = 1
    FAILED         = 2
    NOT_UNDERSTOOD = 3


def make_request(cmd, client):
    request = [PROTO_VERSION, cmd]
    if cmd in [Cmd.START, Cmd.STOP]:
        if client is None:
            raise ValueError("client is required")
        request.append(client)
    else:
        assert client is None
    return request


def talk(*args):
    DEFAULT_ADDR = "/var/run/amtredird.sock"

    request = make_request(*args)

    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect(DEFAULT_ADDR)
    sock.sendall(msgpack.packb(request))
    raw_reply = sock.recv(1024, socket.MSG_WAITALL)
    reply = msgpack.unpackb(raw_reply)

    assert len(reply) > 1 and reply[0] == PROTO_VERSION
    code, args = reply[1], reply[2:]
    return (code, args)


def list(client): return talk(Cmd.LIST, client)
def start(client): return talk(Cmd.START, client)
def stop(client): return talk(Cmd.STOP, client)


def main(raw_args):
    cmds = {'list': list, 'start': start, 'stop': stop}
    parser = argparse.ArgumentParser(
        description='Client for simple AMT redirection daemon')
    parser.add_argument('COMMAND', choices=cmds.keys(),
                        help='Command to execute')
    parser.add_argument('-c', metavar='CLIENT',
                        help='Client hostname to operate on')

    args = parser.parse_args(raw_args)
    try:
        code, args = cmds[args.COMMAND](args.c)
        if len(args) > 0:
            for value in args:
                print value
        return code
    except Exception as e:
        print >>sys.stderr, e
        return 10


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
