"""Implements the :py:mod:`asyncio`-based interface."""

import asyncio

from libchirp import ChirpBase, Config, Loop, MessageThread

from _libchirp_cffi import ffi, lib  # noqa

__all__ = ('Chirp', 'Config', 'Message', 'Loop')


class Message(MessageThread):
    """Chirp message. To answer to message just replace the data and send it.

    .. note::

       The underlaying C type is annotated in parens. The properties of the
       message use asserts to check if the value has the correct type, length,
       range. You can disable these with python -O.
    """

    def release_slot(self):
        """Release the internal message-slot. This method is await-able.

        Will also acknowledge the message if the remote requested a
        acknowledge-message.

        The result of the future will be set to (identity, serial) once the
        message is released. If the message had no slot, the result will be set
        to None.

        May only be used from asyncio-event-loop-thread.

        :rtype: asyncio.Future
        """
        return asyncio.wrap_future(MessageThread.release_slot(self))

    release = release_slot


async def _async_handler(chirp, msg):
    """Call the user-hander and releases the message if AUTO_RELEASE=1."""
    await chirp.handler(msg)
    if chirp._auto_release:
        msg.release()


@ffi.def_extern()
def _async_recv_cb(chirp_t, msg_t):
    """libchirp.c calls this when a message has arrived."""
    chirp = ffi.from_handle(chirp_t.user_data)
    msg = Message(msg_t)
    chirp._register_msg(msg)
    msg._chirp = chirp
    asyncio.run_coroutine_threadsafe(
        _async_handler(chirp, msg), chirp._asyncio_loop
    )


class Chirp(ChirpBase):
    """Runs chirp in a :py:mod:`asyncio` environment.

    Messages are received via a handler and sending is await-able.

    Subclass :py:class:`libchirp.asyncio.Chirp` and implement
    :py:meth:`handler`.

    Concurrency is achieved by sending multiple messages and waiting for
    results later. Use :py:attr:`libchirp.queue.Message.identity` as key to a
    dict, to match-up requests and answers.

    See :ref:`exceptions`.

    :param libchirp.Loop loop: libuv event-loop
    :param libchirp.Config config: chirp config
    :param asyncio.AbstractEventLoop asyncio_loop: asyncio event loop
    """

    def __init__(self, loop, config, asyncio_loop):
        assert isinstance(asyncio_loop, asyncio.AbstractEventLoop)
        self._asyncio_loop = asyncio_loop
        ChirpBase.__init__(self, loop, config, lib._async_recv_cb)

    def send(self, msg):
        """Send a message. This method is await-able.

        Returns a Future which you can await. The result will contain the
        message that has been sent.

        In synchronous-mode the future finishes once the remote has released
        the message. In asynchronous-mode the future finishes once the message
        has been passed to the operating-system.

        Awaiting can raise the exceptions: :py:class:`ConnectionError`,
        :py:class:`TimeoutError`, :py:class:`RuntimeError`,
        :py:class:`ValueError`, :py:class:`MemoryError`.  The exception
        contains the last error message if any generated by chirp. Also
        :py:class:`Exception` for unknown errors. See :ref:`exceptions`.

        See also :ref:`concurrency`.

        May only be used from asyncio-event-loop-thread.

        :param libchirp.asyncio.Message msg: The message to send.
        :rtype: asyncio.Future
        """
        return asyncio.wrap_future(ChirpBase.send(self, msg))

    async def handler(self, msg):  # noqa
        """Called when a message arrives.

        Please implement this method.

        If you don't implement this chirp will release the message-slots
        regardless of the AUTO_RELEASE setting.

        :param libchirp.asyncio.Message msg: The message
        """
        if not self._auto_release:
            msg.release()
