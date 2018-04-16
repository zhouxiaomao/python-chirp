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
    if not chirp._check_request(msg):
        if chirp._disable_queue:
            msg.release()
        else:
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
        self._disable_queue = False
        Queue.__init__(self)
        ChirpBase.__init__(self, loop, config, lib._queue_recv_cb)

    @property
    def disable_queue(self):
        """Get if the queue is disabled.

        If the queue is disabled no messages will be delivered to the queue and
        the messages will always be released.
        """
        return self._disable_queue

    @disable_queue.setter
    def disable_queue(self, value):
        """Set if the queue is disabled."""
        self._disable_queue = value

    def get(self, *args, **kwargs):
        """Remove, release and return an message from the queue.

        If optional args block is true and timeout is None (the default), block
        if necessary until an item is available. If timeout is a positive
        number, it blocks at most timeout seconds and raises the Empty
        exception if no item was available within that time. Otherwise (block
        is false), return an item if one is immediately available, else raise
        the Empty exception (timeout is ignored in that case).
        """
        msg = Queue.get(self, *args, **kwargs)
        if msg and self._auto_release:
            msg.release()
        return msg

    def get_nowait(self):
        """Equivalent to get(False)."""
        msg = Queue.get(self, False)
        if msg and self._auto_release:
            msg.release()
        return msg
