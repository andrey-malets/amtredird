#!/usr/bin/env python

import argparse
import msgpack
import socket
import sys


class Cmd:
    LIST  = 0
    START = 1
    STOP  = 2


class Result:
    OK             = 0
    NO_SUCH_CLIENT = 1
    FAILED         = 2
    NOT_UNDERSTOOD = 3


def talk(cmd, client):
    PROTO_VERSION = 1
    DEFAULT_ADDR = "/var/run/amtredird.sock"

    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect(DEFAULT_ADDR)
    sock.sendall(msgpack.packb([PROTO_VERSION, [cmd, client]]))

    reply = sock.recv(1024)
    [version, code] = msgpack.unpackb(reply)
    assert version == PROTO_VERSION
    return code


def start(client): return talk(Cmd.START, client)
def stop(client): return talk(Cmd.STOP, client)


def main(raw_args):
    cmds = {'start': start, 'stop': stop}
    parser = argparse.ArgumentParser(
        description='Client for simple AMT redirection daemon')
    parser.add_argument('COMMAND', choices=cmds.keys(),
                        help='Command to execute')
    parser.add_argument('CLIENT', help='Client hostname to operate on')

    args = parser.parse_args(raw_args)
    try:
        return cmds[args.COMMAND](args.CLIENT)
    except Exception as e:
        print >>sys.stderr, e
        return 10


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
