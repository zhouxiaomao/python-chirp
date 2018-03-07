"""Main module of libchirp, containing common and low level bindings."""
import atexit
from concurrent.futures import Future
from ipaddress import ip_address, IPv6Address
import logging
import sys
import socket
import threading
import ssl  # noqa let python setup ssl

from _libchirp_cffi import ffi, lib  # noqa

assert lib.ch_libchirp_init() == lib.CH_SUCCESS
atexit.register(lambda: lib.ch_libchirp_cleanup())
# Since the init functions of libchirp will zero the memory, we need an
# allocator that doesn't zero the memory.
_new_nozero = ffi.new_allocator(should_clear_after_alloc=False)
_l = logging.getLogger("libchirp")

__all__ = ('Config', 'Message')


class Config(object):
    """Chirp configuration.

    The underlaying C type is annotated in parens. CFFI will raise errors if
    the values overflow (OverflowError) or don't convert (TypeError).

    .. py:attribute:: REUSE_TIME

       Time until a connection gets garbage collected. Until then the
       connection will be reused. (float)

    .. py:attribute:: TIMEOUT

       IO related timeout in seconds: Sending messages, connecting to remotes.
       (float)

    .. py:attribute:: PORT

       Port for listening to connections. (uint16_t)

    .. py:attribute:: BACKLOG

       TCP-listen socket backlog. (uint8_t)

       From man 2 listen:

           The backlog argument defines the maximum length to which the queue
           of pending connections for sockfd may grow.  If a connection request
           arrives when the queue is full, the client may receive an error with
           an indication of ECONNREFUSED or, if the underlying protocol
           supports retransmission, the request may be ignored so that a later
           reattempt at connection succeeds.

    .. py:attribute:: MAX_SLOTS

       Count of message-slots used. Allowed values are values between 1 and 32.
       The default is 0: Use 16 slots of ACKNOWLEDGE=0 and 1 slot if
       ACKNOWLEDGE=1. (uint8_t)

    .. py:attribute:: ACKNOWLEDGE

       Acknowledge messages. Default True. Makes chirp connection-synchronous.
       See :ref:`modes-of-operation`. Python boolean expected.

    .. py:attribute:: DISABLE_SIGNALS

       By default chirp closes on SIGINT (Ctrl-C) and SIGTERM. Python boolean
       expected. Defaults to False.

    .. py:attribute:: BUFFER_SIZE

       Size of the buffer used for a connection. Defaults to 0, which means
       use the size requested by libuv. Should not be set below 1024.
       (uint32_t)

    .. py:attribute:: MAX_MSG_SIZE

       Max message size accepted by chirp. (uint32_t)

       If you are concerned about memory usage set config.MAX_SLOTS=1 and
       config.MAX_MSG_SIZE to something small, depending on your use-case. If
       you do this, a connection will use about:

       conn_buffers_size = config.BUFFER_SIZE +
          min(config.BUFFER_SIZE, CH_ENC_BUFFER_SIZE) +
          sizeof(ch_connection_t) +
          sizeof(ch_message_t) +
          (memory allocated by TLS implementation)

       conn_size = conn_buffers_size + config.MAX_MSG_SIZE

       With the default config and SSL conn_buffers_size should be about
       64k + 16k + 2k + 32k -> 114k. Derived from documentation, no
       measurement done.

    .. py:attribute:: BIND_V6

       Override IPv6 bind address. String representation expected, parsed by
       :py:class:`ipaddress.ip_address`.

    .. py:attribute:: BIND_V4

       Override IPv4 bind address. String representation expected, parsed by
       :py:class:`ipaddress.ip_address`.

    .. py:attribute:: IDENTITY

       Override the chirp-nodes IDENTITY (this chirp instance). By default
       chirp will generate a IDENTITY. Python bytes of length 16. Everything
       else will not be accepted by CFFI.

    .. py:attribute:: CERT_CHAIN_PEM

       Path to the verification certificate. Python string.

    .. py:attribute:: DH_PARAMS_PEM

       Path to the file containing DH parameters. Python string.

    .. py:attribute:: DISABLE_ENCRYPTION

       Disables encryption. Only use if you know what you are doing.
       Connections to "127.0.0.1" and "::1" aren't encrypted anyways. Python
       boolean expected. Defaults to False.

    .. py:attribute:: AUTO_RELEASE

       By default chirp will release the message-slot automatically when the
       handler-callback returns. Python boolean.

       Not used in queue-operation: always release the message.

       In synchronous-mode the remote will only send the next message when the
       current message has been released.

       In asynchronous-mode when all slots are used up and the TCP-buffers
       are filled up, the remote will eventually not be able to send more
       messages. After TIMEOUT seconds messages start to time out.

       synchronous-mode/asynchronous-mode are independent from async-, queue-
       and pool-operation. The modes refer to a single connection, while the
       operation refers to the interface in python.

       See :ref:`modes-of-operation`
    """

    _ips     = ('BIND_V4', 'BIND_V6')
    _bools   = ('ACKNOWLEDGE', 'DISABLE_SIGNALS', 'DISABLE_ENCRYPTION')
    _strings = ('CERT_CHAIN_PEM', 'DH_PARAMS_PEM')

    def __init__(self):
        dself = self.__dict__
        dself['AUTO_RELEASE'] = True
        conf_t = _new_nozero("ch_config_t*")
        dself['_conf_t'] = conf_t
        lib.ch_chirp_config_init(conf_t)

    def __setattr__(self, name, value):
        """Set attributes to the ffi object.

        Most attributes are directly set, strings and bools are converted.
        """
        conf = self.__dict__['_conf_t']
        if name in Config._ips:
            setattr(conf, name, ip_address(value).packed)
        elif name in Config._bools:
            setattr(conf, name, value.to_bytes(1, sys.byteorder))
        elif name in Config._strings:
            string = ffi.new("char[]", value.encode("UTF-8"))
            # Strings must be kept alive
            self.__dict__['_%s' % name] = string
            setattr(conf, name, string)
        else:
            setattr(conf, name, value)

    def __getattr__(self, name):
        """Get attributes from the ffi object.

        Most attributes are directly get, strings and bools are converted.
        """
        conf = self.__dict__['_conf_t']
        if name in Config._ips:
            return ip_address(bytes(getattr(conf, name))).compressed
        elif name in Config._bools:
            return bool(getattr(conf, name)[0])
        elif name in Config._strings:
            return ffi.string(getattr(conf, name)).decode("UTF-8")
        else:
            return getattr(conf, name)


class Message(object):
    """Chirp message. To answer to message just replace the data and send it.

    .. note::

       The underlaying C type is annotated in parens. The properties of the
       message use asserts to check if the value has the correct type, length,
       range. You can disable these with python -O.
    """

    __slots__ = (
        '_msg_t',
        '_kheader',
        '_kdata',
        '_identity',
        '_serial',
        '_header',
        '_data',
        '_address',
        '_port',
        '_remote_identity',
        '_fut',
    )

    def __init__(self, cmsg=None):
        self._msg_t = cmsg
        self._copy_from_c()
        self._fut = None
        if cmsg:
            lib.ch_msg_free_data(self._msg_t)

    def _ensure_message(self):
        """Ensure that a message exists."""
        msg = self._msg_t
        if not msg:
            msg = _new_nozero("ch_message_t*")
            lib.ch_msg_init(msg)
            self._msg_t = msg
        return msg

    def _copy_from_c(self):
        """Copy messsage from C structure."""
        msg = self._ensure_message()
        self._identity = ffi.buffer(msg.identity)[:]
        self._serial = msg.serial
        self._header = ffi.buffer(msg.header, msg.header_len)[:]
        self._data = ffi.buffer(msg.data, msg.data_len)[:]
        if msg.ip_protocol == socket.AF_INET6:
            abuf = ffi.buffer(msg.address, lib.CH_IP_ADDR_SIZE)[:]
        else:
            abuf = ffi.buffer(msg.address, lib.CH_IP4_ADDR_SIZE)[:]
        self._address = ip_address(abuf)
        self._port = msg.port
        self._remote_identity = ffi.buffer(msg.remote_identity)[:]

    def _copy_to_c(self):
        """Copy messsage to C structure."""
        msg = self._ensure_message()
        msg.identity = self._identity
        header_len = len(self.header)
        msg.header_len = header_len
        if header_len:
            header = ffi.from_buffer(self._header)
            msg.header = header
            # Buffers must be kept alive
            self._kheader = header
        else:
            msg.header = ffi.NULL
        data_len = len(self.data)
        msg.data_len = data_len
        if data_len:
            data = ffi.from_buffer(self._data)
            msg.data = data
            # Buffers must be kept alive
            self._kdata = data
        else:
            msg.data = ffi.NULL
        addr = self._address
        if isinstance(addr, IPv6Address):
            msg.ip_protocol = socket.AF_INET6
            msg.address = addr.packed
        else:
            msg.ip_protocol = socket.AF_INET
            msg.address = addr.packed
        msg.port = self._port

    @property
    def identity(self):
        """Get identify the message and answers to it. (uint8_t[16]).

        The identity can be used to find answers to a message, since replying
        to the message won't change the identity.

        If you need to uniquely identify the message, use the identity/serial
        pair, since the serial will change when replying to messages.
        (read-only)

        :rtype: bytes
        """
        return self._identity

    @property
    def serial(self):
        """Get the serial number of the message. (uint32_t).

        Increases monotonic. Be aware of overflows, if want to use it for
        ordering use the delta: serialA - serialB. (read-only)

        :rtype: int
        """
        return self._serial

    @property
    def header(self):
        """Get the header used by upper-layer protocols.

        Users should not use it, except if you know what you are doing.

        :rtype: bytes
        """
        return self._header

    @header.setter
    def header(self, value):
        """Set the header used by upper-layer protocols.

        :param bytes value: The value
        """
        assert isinstance(value, bytes)
        self._header = value

    @property
    def data(self):
        """Get the data of the message.

        :rtype: bytes
        """
        return self._data

    @data.setter
    def data(self, value):
        """Set the data of the message.

        :param bytes value: The value
        """
        assert isinstance(value, bytes)
        self._data = value

    @property
    def address(self):
        """Get address.

        If the message was received: The address of the remote the message was
        received from.

        If the message will be sent: The address to send the message to.

        This allows to reply to messages just by replacing :py:meth:`data`.

        :return: String representation generated by
                 py:class:`ipaddress.ip_address`.
        :rtype: string
        """
        return self._address.compressed

    @address.setter
    def address(self, value):
        """Set address.

        :param str value: String representation expected, parsed by
                            :py:class:`ipaddress.ip_address`.
        """
        self._address = ip_address(value)

    @property
    def port(self):
        """Get port. (uint16_t).

        If the message was received: The port of the remote the message was
        received from.

        If the message will be sent: The port to send the message to.

        This allows to reply to messages just by replacing :py:meth:`data`.

        :rtype: int
        """
        return self._port

    @port.setter
    def port(self, value):
        """Set port.

        :param int value: The value
        """
        assert value >= 0 and value <= 2**16
        self._port = value

    @property
    def remote_identity(self):
        """Detect the remote instance. (uint8_t[16]).

        By default a node's identity will change on each start of chirp. If
        multiple peers share state, a change in the remote_identity should
        trigger a reset of the state. Simply use the remote_identity as key in
        a dictionary of shared state. (read-only)
        """
        return self._remote_identity

    @property
    def has_slot(self):
        """Return if the message has a slot.

        If :py:attr:`libchirp.Config.AUTO_RELEASE` is False, you have to call
        :py:meth:`release_slot`
        """
        msg = self._ensure_message()
        return lib.ch_msg_has_slot(msg) == 1

    def release_slot(self):
        """Release the internal message-slot.

        Will also acknowledge the message, if
        :py:attr:`libchirp.Config.ACKNOWLEDGE` is True.

        You do not have to call this if :py:attr:`libchirp.Config.AUTO_RELEASE`
        is True. The message will automatically released when the
        message-handler (your code) returns.
        """
        if self.has_slot:
            # TODO test
            lib.ch_chirp_release_message(self._msg_t)
            self._msg_t = None


@ffi.def_extern()
def _loop_async_cb(async_t):
    """Libuv calls this in the thread context of the event-loop.

    Used to execute code in the event-loops thread.
    """
    self = ffi.from_handle(async_t.data)
    with self._lock:
        soon_list = self._soon_list
        self._soon_list = []
    for func, args, kwargs in soon_list:
        func(*args, **kwargs)


@ffi.def_extern()
def _loop_close_cb(handle_t):
    """Libuv calls this when the async_t handle is closed."""
    self = ffi.from_handle(handle_t.data)
    with self._lock:
        loop = self._loop_t
    lib.uv_stop(loop)


class Loop(object):
    """Initialize and run a libuv event-loop."""

    def __init__(self):
        self._stopped = False
        self._started = False
        self._soon_list    = []
        self._refcnt       = 1
        self._lock         = threading.Lock()
        self._data         = ffi.new_handle(self)
        self._loop_t       = _new_nozero("uv_loop_t*")
        self._async_t      = _new_nozero("uv_async_t*")
        self._async_t.data = self._data
        lib.uv_loop_init(self._loop_t)
        lib.uv_async_init(self._loop_t, self._async_t, lib._loop_async_cb)

    def _target(self):
        """Run the event-loop."""
        with self._lock:
            loop_t = self._loop_t
        _l.debug("libuv event-loop started")
        if lib.uv_run(loop_t, lib.UV_RUN_DEFAULT) != 0:
            _l.warning("Cannot close all uv-handles/requests.")
            if lib.uv_run(loop_t, lib.UV_RUN_NOWAIT) != 0:
                # No we have a serious problem
                _l.error("Cannot close all uv-handles/requests.")

    def call_soon(self, func, *args, **kwargs):
        """Call function in event-loop thread.

        The function will be executed asynchronous. If you need a result pass a
        py:class:`concurrent.futures.Future` to the function.

        For example:

        .. code:: python

           loop.call_soon(print, "hello")
        """
        with self._lock:
            self._soon_list.append((func, args, kwargs))
            async_t = self._async_t
        lib.uv_async_send(async_t)

    def run(self):
        """Run the event loop."""
        with self._lock:
            do_it = not self._started
            stopped = self._stopped
        if do_it:
            self._started = True
            self._thread = threading.Thread(target=self._target)
            self._thread.start()
        elif stopped:
            # We are silent about multiple run/stop, but if user expects it is
            # possible to restart, we raise an error.
            raise RuntimeError("Cannot restart loop")

    @property
    def running(self):
        """Return True if event-loop is running."""
        with self._lock:
            return self._started and not self._stopped

    def stop(self):
        """Stop the event-loop.

        For convenience stop() will wait for the last chirp instance to stop.
        """
        with self._lock:
            do_it = False
            if self._started and not self._stopped:
                self._stopped = True
                do_it = True
        if do_it:
            self._refdec()

    def _refdec(self):
        """Decrement refcount. If 0 the loop will be stopped."""
        with self._lock:
            self._refcnt -= 1
            do_it = self._refcnt == 0
        if do_it:
            self._do_stop()

    def _refinc(self):
        """Increment refcount."""
        with self._lock:
            self._refcnt += 1

    def _do_stop(self):
        """Stop the event-loop."""
        def stop_libuv(self):
            with self._lock:
                async_t = self._async_t
            lib.uv_close(
                ffi.cast("uv_handle_t*", async_t),
                lib._loop_close_cb
            )
        self.call_soon(stop_libuv, self)
        self._thread.join()
        _l.debug("libuv event-loop stopped")
        if lib.ch_loop_close(self._loop_t) != lib.CH_SUCCESS:
            raise RuntimeError("Closing libuv event-loop failed")
        # Break loops
        # I first implemented everything nicely with weakrefs and __del__
        # only to learn that python sometimes calls __del__ so late that
        # _do_stop() fails. So the user has to call stop.
        with self._lock:
            self._thread = None
            self._data   = None


_last_error = threading.local()


@ffi.def_extern()
def _chirp_log_cb(msg, error):
    """libchirp.c calls this to log messages."""
    emsg = ffi.string(msg).decode("UTF-8")
    if error[0]:
        _last_error.data = emsg
        _l.error(emsg)
    else:
        _l.debug(emsg)


@ffi.def_extern()
def _chirp_done_cb(chirp_t):
    """libchirp.c calls this when the chirp-instance is done."""
    ffi.from_handle(chirp_t.user_data)._done.set_result(0)


@ffi.def_extern()
def _send_cb(chirp_t, msg_t, status):
    """libchirp.c calls this when a message is sent."""
    chirp = ffi.from_handle(chirp_t.user_data)
    msg = ffi.from_handle(msg_t.user_data)
    if status == lib.CH_SUCCESS:
        msg._fut.set_result(0)
    else:
        msg._fut.set_exception(
            chirp_error_to_exception(status, _last_error.data)
        )
    with chirp._lock:
        del chirp._await_msgs[msg]
    msg._fut = None


def chirp_error_to_exception(error, msg):
    """Convert libchirp error-codes to exceptions."""
    if error == lib.CH_VALUE_ERROR:
        excp = ValueError(msg or "CH_VALUE_ERROR")
    elif error == lib.CH_UV_ERROR:
        excp = RuntimeError(msg or "CH_UV_ERROR")
    elif error == lib.CH_INIT_FAIL:
        excp = RuntimeError(msg or "CH_INIT_FAIL")
    elif error == lib.CH_TLS_ERROR:
        excp = RuntimeError(msg or "CH_TLS_ERROR")
    elif error == lib.CH_EADDRINUSE:
        excp = OSError(msg or "CH_EADDRINUSE")
    elif error == lib.CH_FATAL:
        excp = RuntimeError(msg or "CH_FATAL")
    elif error == lib.CH_PROTOCOL_ERROR:
        excp = RuntimeError(msg or "CH_PROTOCOL_ERROR")
    elif error == lib.CH_CANNOT_CONNECT:
        excp = ConnectionError(msg or "CH_CANNOT_CONNECT")
    elif error == lib.CH_WRITE_ERROR:
        excp = ConnectionError(msg or "CH_WRITE_ERROR")
    elif error == lib.CH_TIMEOUT:
        excp = TimeoutError(msg or "CH_TIMEOUT")
    elif error == lib.CH_ENOMEM:
        excp = MemoryError()
    else:
        excp = Exception(msg or "Unknown error: %d" % error)
    excp.ecode = error
    return excp


class ChirpBase(object):
    """Chirp base for async-, queue- and pool-operation.

    * :py:class:`libchirp.async.Chirp`: Runs chirp in a :py:mod:`asyncio`
      environment. Messages are received via a handler and sending is
      await-able

    * :py:class:`libchirp.queue.Chirp`: Implements a :py:class:`queue.Queue`.
      Concurrency is achieved by sending multiple messages and waiting for
      results later. Use :py:attr:`libchirp.Message.identity` as key to a dict,
      to match-up requests and answers.

    * :py:class:`libchirp.pool.Chirp`: Implements a
      :py:class:`concurrent.futures.ThreadPoolExecutor`. Used when you have to
      interface with blocking code. Please only use if really needed.

    Creating a chirp instance can raise the exceptions:
    :py:class:`OSError`, :py:class:`TimeoutError`, :py:class:`RuntimeError`,
    :py:class:`ValueError`, :py:class:`MemoryError`.  The exception contains
    the last error message if any generated by chirp. Also
    :py:class:`Exception` for unknown errors. See :ref:`exceptions`.

    :param loop     Loop: libuv event-loop
    :param config Config: chirp config
    """

    def _chirp_init(self, fut):
        with self._lock:
            chirp  = self._chirp_t
            config = self._config._conf_t
            loop   = self._loop._loop_t
        _last_error.data = ""
        res = lib.ch_chirp_init(
            chirp,
            config,
            loop,
            ffi.NULL,
            ffi.NULL,
            lib._chirp_done_cb,
            lib._chirp_log_cb
        )
        data = ffi.new_handle(self)
        with self._lock:
            self._data     = data
            chirp.user_data = data
        if res == 0:
            fut.set_result(0)
        else:
            fut.set_result(chirp_error_to_exception(
                res, _last_error.data
            ))

    def __init__(self, loop, config):
        assert isinstance(loop, Loop)
        assert isinstance(config, Config)
        self._await_msgs = dict()
        self._done       = Future()
        self._lock       = threading.Lock()
        self._loop       = loop
        self._config     = config
        self._chirp_t    = _new_nozero("ch_chirp_t*")
        fut = Future()
        loop.call_soon(ChirpBase._chirp_init, self, fut)
        res = fut.result()
        if res != 0:
            if not isinstance(res, MemoryError):
                self._done.result()
            raise res
        loop._refinc()

    def stop(self):
        """Stop the chirp-instance."""
        lib.ch_chirp_close_ts(self._chirp_t)
        self._done.result()
        self._loop._refdec()
        with self._lock:
            # Break loops
            self._data = None
            self._loop = None

    def send(self, msg):
        """Send a message.

        Returns a Future which you can await if you use async-operation or you
        call result() on it for queue- and pool-operation.

        In synchronous-mode the future finishes once the remote has released
        the message. In asynchronous-mode the future finishes once the message
        has been passed to the operating-system.

        Awaiting/Calling result() can raise the exceptions:
        :py:class:`ConnectionError`, :py:class:`TimeoutError`,
        :py:class:`RuntimeError`, :py:class:`ValueError`,
        :py:class:`MemoryError`.  The exception contains the last error message
        if any generated by chirp. Also :py:class:`Exception` for unknown
        errors. See :ref:`exceptions`.

        In a low-latency environment up to 64 for futures can be kept open at
        time for optimal performance. Beyond 64, managing the open requests in
        chirp and maybe also the operating-system seems to kick in. If you need
        to improve the performance please experiment in your particular
        environment.

        :param msg Message: The message to send.
        :rtype: concurrent.futures.Future
        """
        assert isinstance(msg, Message)
        fut = Future()
        if msg._fut:
            raise RuntimeError("Message already sending")
        with self._lock:
            msg._fut = fut
            msg_t = msg._msg_t
            handle = ffi.new_handle(msg)
            msg_t.user_data = handle
            # msg/handle must be kept alive
            self._await_msgs[msg] = handle
        _last_error.data = ""
        msg._copy_to_c()
        lib.ch_chirp_send_ts(self._chirp_t, msg_t, lib._send_cb)
        return fut
