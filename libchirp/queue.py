"""Implements the :py:class:`queue.Queue`-based interface."""

from queue import Queue

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


@ffi.def_extern()
def _queue_recv_cb(chirp_t, msg_t):
    """libchirp.c calls this when a message has arrived."""
    chirp = ffi.from_handle(chirp_t.user_data)
    msg = Message(msg_t)
    chirp._register_msg(msg)
    msg._chirp = chirp
    chirp.put(msg)


class Chirp(ChirpBase, Queue):
    """Implements the :py:class:`queue.Queue`-based interface.

    Concurrency is achieved by sending multiple messages and waiting for
    results later. Use :py:attr:`libchirp.queue.Message.identity` as key to a
    dict, to match-up requests and answers.

    See :ref:`exceptions`.

    :param libchirp.Loop loop: libuv event-loop
    :param libchirp.Config config: chirp config
    """

    def __init__(self, loop, config):
        Queue.__init__(self)
        ChirpBase.__init__(self, loop, config, lib._queue_recv_cb)
