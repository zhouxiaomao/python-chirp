"""Implements the ThreadPool-based interface."""

from concurrent.futures import ThreadPoolExecutor

from libchirp import ChirpBase, Config, Loop, MessageThread

from _libchirp_cffi import ffi, lib  # noqa


__all__ = ('Chirp', 'Config', 'Message', 'Loop')


class Message(MessageThread):  # noqa
    """Chirp message. To answer to message just replace the data and send it.

    .. note::

       The underlaying C type is annotated in parens. The properties of the
       message use asserts to check if the value has the correct type, length,
       range. You can disable these with python -O.
    """

    pass


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
    results later. Use :py:attr:`libchirp.MessageBase.identity` as key to a
    dict, to match-up requests and answers.

    `**kwargs` are passed to ThreadPoolExecutor.

    See :ref:`exceptions`.

    :param libchirp.Loop loop: libuv event-loop
    :param libchirp.Config config: chirp config
    """

    def __init__(self, loop, config, **kwargs):
        ThreadPoolExecutor.__init__(self, **kwargs)
        ChirpBase.__init__(self, loop, config, lib._pool_recv_cb)

    def handler(self, msg):  # noqa
        """Called when a message arrives.

        :param libchirp.pool.Message msg: The message
        """
        pass
