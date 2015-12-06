import config
import json
import os
import socket
import sys

from flask import abort, Flask, request
from functools import wraps

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir))
import client

application = Flask('amtredird')
application.debug = True


class authorized(object):

    def __init__(self, acl):
        self.hosts = dict((socket.gethostbyname(host), True) for host in acl)

    def __call__(self, f):
        @wraps(f)
        def wrapper(*args, **kwargs):
            if request.remote_addr in self.hosts:
                return f(*args, **kwargs)
            else:
                abort(403)
        return wrapper


def reply(result):
    return "{}\n".format(json.dumps(result, indent=2))


@application.route('{}/list'.format(config.WEB_PATH))
@authorized(config.ACL)
def list():
    try:
        return reply(client.list(None))
    except Exception as e:
        return reply({'error': str(e)})


@application.route('{}/<cmdname>'.format(config.WEB_PATH), methods=["POST"])
@authorized(config.ACL)
def act(cmdname):
    cmd = {'start': client.start, 'stop': client.stop}.get(cmdname)
    if cmd is not None:
        return reply(dict((value, cmd(value))
                          for _, value in request.form.iteritems()))
    else:
        abort(404)
