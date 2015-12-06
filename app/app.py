import config
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


@application.route('{}/list'.format(config.WEB_PATH))
@authorized(config.ACL)
def list():
    try:
        return "{}\n".format(client.list(None))
    except Exception as e:
        return str(e)


@application.route('{}/<cmdname>'.format(config.WEB_PATH), methods=["POST"])
@authorized(config.ACL)
def act(cmdname):
    cmd = {'start': client.start, 'stop': client.stop}.get(cmdname)
    if cmd is not None:
        return "{}\n".format(
            [cmd(value) for _, value in request.form.iteritems()])
    else:
        abort(404)
