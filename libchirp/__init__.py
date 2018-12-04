"""Main module of libchirp, containing common and low level bindings."""
import atexit
from concurrent.futures import Future
from concurrent.futures import TimeoutError as CFTimeoutError
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

__all__ = ('Config', 'Loop')


class ChirpFuture(Future):
    """Implements a future that can wait for a chirp request."""

    def __init__(self, auto_release=True):
        self._auto_release = auto_release
        Future.__init__(self)

    def result(self, timeout=None):
        """Return the answer to the request.

        See :py:meth:`concurrent.futures.Future.result`. In contrast to the
        standard Future this method will be release the message-slot if
        :py:attr:`libchirp.Config.AUTO_RELEASE` is True.

        .. code-block:: python

            res = fut.result()
            # Do something
            res.release()

        :rtype: libchirp.MessageBase
        """
        try:
            res = Future.result(self, timeout)
            if self._auto_release:
                res.release()
            return res
        finally:
            self._chirp = None

    def send_result(self, timeout=None):
        """Wait for the request being sent.

        :param float timeout: Timeout in seconds
        """
        return self._send_fut.result(timeout)


class Config(object):
    """Chirp configuration.

    The underlaying C type is annotated in parens. CFFI will raise errors if
    the values overflow (OverflowError) or don't convert (TypeError).

    You can create the certificate using the makepki Makefile_ on github. If
    you want to create it manually the chain has to contain:

    * The certification authority's public key

    * The client public key (signed by CA)

    * The client private key

    Any client-key signed by the CA will be able to connect.

    .. _Makefile: https://github.com/concretecloud/chirp/tree/master/mk/makepki
    """

    _ips     = ('BIND_V4', 'BIND_V6')
    _bools   = ('SYNCHRONOUS', 'DISABLE_SIGNALS', 'DISABLE_ENCRYPTION')
    _strings = ('CERT_CHAIN_PEM', 'DH_PARAMS_PEM')

    def __init__(self):
        self._sealed = False
        self._AUTO_RELEASE = True
        conf_t = _new_nozero("ch_config_t*")
        self._conf_t = conf_t
        lib.ch_chirp_config_init(conf_t)

    def _setattr_ffi(self, name, value):
        """Set attributes to the ffi object.

        Most attributes are directly set, strings and bools are converted.
        """
        if self._sealed:
            raise RuntimeError("Config is used an therefore read-only.")
        conf = self._conf_t
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

    def _getattr_ffi(self, name):
        """Get attributes from the ffi object.

        Most attributes are directly get, strings and bools are converted.
        """
        conf = self._conf_t
        if name in Config._ips:
            return ip_address(bytes(getattr(conf, name))).compressed
        elif name in Config._bools:
            return bool(getattr(conf, name)[0])
        elif name in Config._strings:
            return ffi.string(getattr(conf, name)).decode("UTF-8")
        else:
            return getattr(conf, name)

    @property
    def SYNCHRONOUS(self):
        """Get if chirp requests and waits for acknowledge messages.

        Default True. Makes chirp connection-synchronous.  See
        :ref:`modes-of-operation`. Python boolean expected.

        :rtype: bool
        """
        return self._getattr_ffi('SYNCHRONOUS')

    @SYNCHRONOUS.setter
    def SYNCHRONOUS(self, value):
        """Set if chirp requests and waits for acknowledge messages."""
        self._setattr_ffi('SYNCHRONOUS', value)

    @property
    def AUTO_RELEASE(self):
        """Get if chirp releases messages.

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
        return self._AUTO_RELEASE

    @AUTO_RELEASE.setter
    def AUTO_RELEASE(self, value):
        """Set chirp releasing messages."""
        if self._sealed:
            raise RuntimeError("Config is used an therefore read-only.")
        self._AUTO_RELEASE = value

    @property
    def BACKLOG(self):
        """Get the TCP-listen socket backlog. (uint8_t).

        From man 2 listen:

            The backlog argument defines the maximum length to which the queue
            of pending connections for sockfd may grow.  If a connection
            request arrives when the queue is full, the client may receive an
            error with an indication of ECONNREFUSED or, if the underlying
            protocol supports retransmission, the request may be ignored so
            that a later reattempt at connection succeeds.

        :rtype: int
        """
        return self._getattr_ffi('BACKLOG')

    @BACKLOG.setter
    def BACKLOG(self, value):
        """Set the TCP-listen socket backlog."""
        self._setattr_ffi('BACKLOG', value)

    @property
    def BIND_V4(self):
        """Override IPv4 bind address.

        String representation expected, parsed by
        :py:class:`ipaddress.ip_address`.

        :rtype: str
        """
        return self._getattr_ffi('BIND_V4')

    @BIND_V4.setter
    def BIND_V4(self, value):
        """Override IPv4 bind address."""
        self._setattr_ffi('BIND_V4', value)

    @property
    def BIND_V6(self):
        """Override IPv6 bind address.

        String representation expected, parsed by
        :py:class:`ipaddress.ip_address`.

        :rtype: str
        """
        return self._getattr_ffi('BIND_V6')

    @BIND_V6.setter
    def BIND_V6(self, value):
        """Override IPv6 bind address."""
        self._setattr_ffi('BIND_V6', value)

    @property
    def BUFFER_SIZE(self):
        """Get the size of the buffer used for a connection.

        Defaults to 0, which means use the size requested by libuv. Should not
        be set below 1024. (uint32_t)

        :rtype: int
        """
        return self._getattr_ffi('BUFFER_SIZE')

    @BUFFER_SIZE.setter
    def BUFFER_SIZE(self, value):
        """Set the size of the buffer used for a connection."""
        self._setattr_ffi('BUFFER_SIZE', value)

    @property
    def CERT_CHAIN_PEM(self):
        """Get the path to the verification certificate. Python string.

        :rtype: str
        """
        return self._getattr_ffi('CERT_CHAIN_PEM')

    @CERT_CHAIN_PEM.setter
    def CERT_CHAIN_PEM(self, value):
        """Set the path to the verification certificate."""
        self._setattr_ffi('CERT_CHAIN_PEM', value)

    @property
    def DH_PARAMS_PEM(self):
        """Get the path to the file containing DH parameters. Python string.

        :rtype: str
        """
        return self._getattr_ffi('DH_PARAMS_PEM')

    @DH_PARAMS_PEM.setter
    def DH_PARAMS_PEM(self, value):
        """Set the path to the file containing DH parameters."""
        self._setattr_ffi('DH_PARAMS_PEM', value)

    @property
    def DISABLE_ENCRYPTION(self):
        """Get if encryption is disabled.

        Only use if you know what you are doing.  Connections to "127.0.0.1"
        and "::1" aren't encrypted anyways. Python boolean expected. Defaults
        to False.

        :rtype: bool
        """
        return self._getattr_ffi('DISABLE_ENCRYPTION')

    @DISABLE_ENCRYPTION.setter
    def DISABLE_ENCRYPTION(self, value):
        """Set encryption is disabled."""
        self._setattr_ffi('DISABLE_ENCRYPTION', value)

    @property
    def DISABLE_SIGNALS(self):
        """Get if signals are disabled.

        By default chirp closes on SIGINT (Ctrl-C) and SIGTERM. Python boolean
        expected. Defaults to False.

        :rtype: bool
        """
        return self._getattr_ffi('DISABLE_SIGNALS')

    @DISABLE_SIGNALS.setter
    def DISABLE_SIGNALS(self, value):
        """Set signals are disabled."""
        self._setattr_ffi('DISABLE_SIGNALS', value)

    @property
    def IDENTITY(self):
        """Override the chirp-nodes identity (this chirp instance).

        By default chirp will generate a IDENTITY. Python bytes of length 16.
        Everything else will not be accepted by CFFI.

        :rtype: bytes
        """
        return self._getattr_ffi('IDENTITY')

    @IDENTITY.setter
    def IDENTITY(self, value):
        """Override the chirp-nodes identity (this chirp instance)."""
        self._setattr_ffi('IDENTITY', value)

    @property
    def MAX_MSG_SIZE(self):
        """Get the max message size accepted by chirp. (uint32_t).

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

        :rtype: int
        """
        return self._getattr_ffi('MAX_MSG_SIZE')

    @MAX_MSG_SIZE.setter
    def MAX_MSG_SIZE(self, value):
        """Set the max message size accepted by chirp."""
        self._setattr_ffi('MAX_MSG_SIZE', value)

    @property
    def MAX_SLOTS(self):
        """Get the count of message-slots used.

        Allowed values are values between 1 and 32.  The default is 0: Use 16
        slots of `SYNCHRONOUS` = `False` and 1 slot if `SYNCHRONOUS` = `True`.
        (uint8_t)

        :rtype: int
        """
        return self._getattr_ffi('MAX_SLOTS')

    @MAX_SLOTS.setter
    def MAX_SLOTS(self, value):
        """Set the count of message-slots used."""
        self._setattr_ffi('MAX_SLOTS', value)

    @property
    def PORT(self):
        """Get the Port for listening to connections. (uint16_t).

        :rtype: int
        """
        return self._getattr_ffi('PORT')

    @PORT.setter
    def PORT(self, value):
        """Set the Port for listening to connections."""
        self._setattr_ffi('PORT', value)

    @property
    def REUSE_TIME(self):
        """Get the time until a connection gets garbage collected.

        Until then the connection will be reused. Actual reuse time will be
        max(REUSE_TIME, TIMEOUT * 3). (float)

        :rtype: float
        """
        return self._getattr_ffi('REUSE_TIME')

    @REUSE_TIME.setter
    def REUSE_TIME(self, value):
        """Set the time until a connection gets garbage collected."""
        self._setattr_ffi('REUSE_TIME', value)

    @property
    def TIMEOUT(self):
        """Get send- and connect-timeout scaling in seconds.

        Send-timeout will be TIMEOUT seconds. Connect-timeout will be
        min(TIMEOUT * 2, 60) seconds. (float)

        :rtype: float
        """
        return self._getattr_ffi('TIMEOUT')

    @TIMEOUT.setter
    def TIMEOUT(self, value):
        """Set send- and connect-timeout scaling in seconds."""
        self._setattr_ffi('TIMEOUT', value)


@ffi.def_extern()
def _release_cb(chirp_t, identity_t, serial):
    """libchirp.c calls this when a message is released."""
    chirp = ffi.from_handle(chirp_t.user_data)
    identity = ffi.buffer(identity_t, lib.CH_ID_SIZE)[:]
    chirp._release_msg(identity, serial)


class MessageBase(object):
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
        '_chirp',
    )

    def __init__(self, cmsg=None):
        self._msg_t = cmsg
        self._copy_from_c()
        self._fut = None
        self._chirp = None
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

        :rtype: bytes
        """
        return self._remote_identity

    @property
    def has_slot(self):
        """Return if the message has a slot.

        If :py:attr:`libchirp.Config.AUTO_RELEASE` is False, you have to call
        :py:meth:`release_slot`

        :rtype: bool
        """
        msg = self._ensure_message()
        return lib.ch_msg_has_slot(msg) != 0


class MessageThread(MessageBase):
    """Chirp message. To answer to message just replace the data and send it.

    .. note::

       The underlaying C type is annotated in parens. The properties of the
       message use asserts to check if the value has the correct type, length,
       range. You can disable these with python -O.
    """

    def release_slot(self):
        """Release the internal message-slot. This method returns a Future.

        Will also acknowledge the message if the remote requested a
        acknowledge-message.

        The result of the future will be set to (identity, serial) once the
        message is released. If the message had no slot, the result will be set
        to None.

        Releasing a message from a different thread is thread-safe. Releasing
        the same message from different threads twice will lead to undefined
        behavior. Releasing, waiting for the result, switching the thread
        synchronized (via queue for example), releasing is fine, tough.

        :rtype: Future
        """
        chirp = self._chirp
        doit = False
        if chirp:
            with chirp._lock:
                if self.has_slot:
                    msg_t = self._msg_t
                    self._msg_t = None
                    fut = chirp._release_msgs[(self.identity, self.serial)][0]
                    doit = True
        if doit:
            if self._fut:
                raise RuntimeError(
                    "Message still sending, please wait for the send() result"
                )
            lib.ch_chirp_release_msg_slot_ts(
                chirp._chirp_t, msg_t, lib._release_cb
            )
            return fut
        fut = Future()
        fut.set_result(None)
        return fut

    release = release_slot


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


class Loop(object):
    """Initialize and run a libuv event-loop.

    By default the loop is run.

    :param bool run_loop: Run the loop (True)
    """

    def __init__(self, run_loop=True):
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
        if run_loop:
            self.run()

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
            # There will be another iteration into the event-loop, we don't
            # need to wait for a callback
            lib.uv_close(
                ffi.cast("uv_handle_t*", async_t),
                ffi.NULL
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


def make_timer(fut):
    """Create a uv_timer to cleanup requests on timeout.

    :param ChirpFuture fut: Future to cancel after timeout
    """
    chirp = fut._chirp
    timer_t = _new_nozero("uv_timer_t*")
    fut._timer_t = timer_t
    lib.uv_timer_init(chirp._loop._loop_t, timer_t)
    # Wrap the future in a cffi handler to pass to timer_t.data
    handle = ffi.new_handle(fut)
    timer_t.data = handle
    # Keep the handler data
    fut._handle = handle
    lib.uv_timer_start(
        timer_t,
        lib._request_timeout_cb,
        int(chirp._config.TIMEOUT * 1000),
        0
    )


@ffi.def_extern()
def _timer_close_cb(timer_t):
    """Free data assosited with the request timeout-timer.

    Free the handle pointing from timer_t.data to the future and free
    timer_t itself.
    """
    fut = ffi.from_handle(timer_t.data)
    chirp = fut._chirp
    if chirp:
        with chirp._lock:
            if fut._id in chirp._requests:
                # Resolve the circular-reference, so the future will be cleared
                # by reference-counting.
                del chirp._requests[fut._id]
    fut._handle = None
    fut._timer_t = None


@ffi.def_extern()
def _request_timeout_cb(timer_t):
    """When the request was not answered in time set an exception and cleanup.

    The close-handler will release the C-memory.
    """
    fut = ffi.from_handle(timer_t.data)
    fut.set_exception(TimeoutError("Chirp request timed out"))
    lib.uv_close(
        ffi.cast("uv_handle_t*", fut._timer_t),
        lib._timer_close_cb
    )


@ffi.def_extern()
def _chirp_log_cb(msg, error):
    """libchirp.c calls this to log messages."""
    emsg = ffi.string(msg).decode("UTF-8")
    if error[0]:
        _last_error.data = emsg
        _l.debug(emsg)
    else:
        _l.info(emsg)


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
        msg._fut.set_result(msg)
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

    * :py:class:`libchirp.asyncio.Chirp`: Runs chirp in a :py:mod:`asyncio`
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

    :param Loop     loop: libuv event-loop
    :param Config config: chirp config
    :param          recv: Recv callback
    """

    def __init__(self, loop, config, recv=None):
        assert isinstance(loop, Loop)
        assert isinstance(config, Config)
        config.__dict__['_sealed'] = True
        self._await_msgs   = dict()
        self._release_msgs = dict()
        self._requests     = dict()
        self._timeouts     = dict()
        self._done         = Future()
        self._lock         = threading.Lock()
        self._loop         = loop
        self._config       = config
        self._auto_release = config.AUTO_RELEASE
        self._stopped      = False
        if not recv:
            self._recv     = ffi.NULL
        else:
            self._recv     = recv
        self._chirp_t      = _new_nozero("ch_chirp_t*")
        fut = Future()
        loop.call_soon(ChirpBase._chirp_init, self, fut)
        res = fut.result()
        if res != 0:
            if not isinstance(res, MemoryError):
                self._done.result()
            raise res
        loop._refinc()

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
            self._recv,
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

    def _register_msg(self, msg):
        """Register a message in the release dict."""
        fut = Future()
        with self._lock:
            self._release_msgs[(msg.identity, msg.serial)] = (fut, msg)

    def _release_msg(self, identity, serial):
        """Call future of a released message."""
        key = (identity, serial)
        with self._lock:
            fut = self._release_msgs[key][0]
            del self._release_msgs[key]
        fut.set_result(key)

    @property
    def loop(self):
        """Get the :py:class:`libchirp.Loop` used by the chirp instance.

        :rtype: Loop
        """
        return self._loop

    def stop(self):
        """Stop the chirp-instance."""
        with self._lock:
            if self._stopped:
                return
            rel_msgs = dict(self._release_msgs)
        try:
            for fut, _ in rel_msgs.values():
                fut.result(timeout=self._config.TIMEOUT)
        except CFTimeoutError:
            for _, msg in rel_msgs.values():
                msg.release_slot()
            try:
                for fut, _ in rel_msgs.values():
                    fut.result(timeout=self._config.TIMEOUT)
            except CFTimeoutError:
                pass
            # Although we have cleaned-up for the user, we raise an exception,
            # because this is an usage-error.
            raise RuntimeError(
                "Timeout waiting for released messages, "
                "maybe a message was not released."
            )
        finally:
            lib.ch_chirp_close_ts(self._chirp_t)
            self._done.result()
            self._loop._refdec()
            with self._lock:
                # Break loops
                self._stopped = True
                self._release_msgs = None
                self._data = None
                self._loop = None

    def send(self, msg):
        """Send a message. This method returns a Future.

        The result will contain the message that has been sent.

        In synchronous-mode the future finishes once the remote has released
        the message. In asynchronous-mode the future finishes once the message
        has been passed to the operating-system.

        Calling result() can raise the exceptions: :py:class:`ConnectionError`,
        :py:class:`TimeoutError`, :py:class:`RuntimeError`,
        :py:class:`ValueError`, :py:class:`MemoryError`.  The exception
        contains the last error message if any generated by chirp. Also
        :py:class:`Exception` for unknown errors. See :ref:`exceptions`.

        See also :ref:`concurrency`.

        Sending different messages from different threads is thread-safe.
        Sending the same message twice from different threads will lead to
        undefined behavior. Sending, waiting for the result, switching the
        thread synchronized (via queue for example), sending is fine, tough.

        :param MessageThread msg: The message to send.
        :rtype: concurrent.futures.Future
        """
        assert isinstance(msg, MessageThread)
        fut = Future()
        with self._lock:
            if msg._fut:
                raise RuntimeError(
                    "Message still sending, please wait for the send() result"
                )
            msg._ensure_message()
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

    def request(self, msg, auto_release=True):
        """Send a message and wait for an answer.

        This method returns a :py:class:`libchirp.ChirpFuture`.

        The result() of the will contain the answer to the message. If you
        don't intend to consume the result use :py:meth:`send` instead.

        By default the message slot used by the response will released.
        If `auto_release` is False, you have to release the response-message.

        Exceptions, threading and concurrency aspects are the same as
        :py:meth:`send`. Issue: If an answer to a request arrives after the
        timeout it will be delivered at normal message.

        To wait for the request being sent use
        :py:meth:`libchirp.ChirpFuture.send_result`.

        .. code-block:: python

            req = chirp.request(msg)
            req.send_result()
            answer = req.result()

        :param MessageThread msg: The message to send.
        :param bool auto_release: Release the response (default True)
        :rtype: concurrent.futures.Future
        """
        send_fut = self.send(msg)
        fut = ChirpFuture(auto_release)
        fut._send_fut = send_fut
        fut._chirp = self
        id_ = msg.identity
        fut._id = id_
        with self._lock:
            self._requests[id_] = fut

        self.loop.call_soon(make_timer, fut)
        return fut

    def _check_request(self, msg):
        """Check if message is an response to a request."""
        with self._lock:
            fut = self._requests.get(msg.identity)
        if fut:
            # If the request if found, we stop the timeout-timer and cleanup
            lib.uv_timer_stop(fut._timer_t)
            lib.uv_close(
                ffi.cast("uv_handle_t*", fut._timer_t),
                lib._timer_close_cb
            )
            fut.set_result(msg)
            return True
        return False

    def identity(self):
        return ffi.buffer(
            lib.ch_chirp_get_identity(self._chirp_t).data
        )[:]
