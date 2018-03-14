"""Implements the :py:class:`queue.Queue`-based interface."""

from libchirp import ChirpBase, Message, Config, Loop
from concurrent.futures import ThreadPoolExecutor

from _libchirp_cffi import ffi, lib  # noqa


__all__ = ('Chirp', 'Config', 'Message', 'Loop')


def _loop_hander(chirp, msg):
    """Call the user-hander and releases the message if AUTO_RELEASE=1."""
    chirp.handler(msg)
    if chirp._auto_release:
        msg.release()


@ffi.def_extern()
def _pool_recv_cb(chirp_t, msg_t):
    """libchirp.c calls this when a message has arrived."""
    chirp = ffi.from_handle(chirp_t.user_data)
    msg = Message(msg_t)
    chirp._register_msg(msg)
    msg._chirp = chirp
    chirp.submit(_loop_hander, chirp, msg)


class Chirp(ChirpBase, ThreadPoolExecutor):
    """Implements a :py:class:`concurrent.futures.ThreadPoolExecutor`.

    Used when you have to interface with blocking code. Please only use if
    really needed. The handler is called on different threads, do not share
    anything or only thread-safe things.

    Subclass :py:class:`libchirp.pool.Chirp` and implement :py:meth:`handler`.

    If :py:attr:`libchirp.Config.AUTO_RELEASE` = `True` the message will be
    release automatically once the handler returns.

    Concurrency is achieved by sending multiple messages and waiting for
    results later. Use :py:attr:`libchirp.Message.identity` as key to a dict,
    to match-up requests and answers.

    :param libchirp.Loop loop: libuv event-loop
    :param libchirp.Config config: chirp config
    """

    def __init__(self, loop, config, max_workers=None, thread_name_prefix=''):
        ThreadPoolExecutor.__init__(self, max_workers, thread_name_prefix)
        ChirpBase.__init__(self, loop, config, lib._pool_recv_cb)

    def handler(self, msg):  # noqa
        """Called when a message arrives.

        :param Message msg: The message
        """
