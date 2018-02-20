"""Main module of libchirp, containing common and low level bindings."""
import atexit
from ipaddress import ip_address, IPv6Address
import sys
import socket

from _libchirp_cffi import ffi, lib  # noqa

lib.ch_en_set_manual_tls_init()  # del_if_chirp_is_built_with_static_openssl

assert lib.ch_libchirp_init() == lib.CH_SUCCESS
atexit.register(lambda: lib.ch_libchirp_cleanup())
# Since the init functions of libchirp will zero the memory, we need an
# allocator that doesn't zero the memory.
new_nozero = ffi.new_allocator(should_clear_after_alloc=False)


class Config(object):
    """Chirp configuration.

    The underlaying C type is annotated in parens. CFFI will raise errors if
    the values overflow (OverflowError) or don't convert (TypeError).

    .. py:attribute:: REUSE_TIME

       Time until a connection gets garbage collected. Until then the
       connection will be reused. (float)

    .. py:attribute:: TIMEOUT

       IO related timeout: Sending messages, connecting to remotes. (float)

    .. py:attribute:: PORT

       Port for listening to connections. (uint16_t)

    .. py:attribute:: BACKLOG

       TCP-listen socket backlog. (uint8_t)

    .. py:attribute:: MAX_HANDLERS

       Count of handlers used. Allowed values are values between 1 and 32.
       The default is 0: Use 16 handlers of ACKNOWLEDGE=0 and 1 handler if
       ACKNOWLEDGE=1. (uint8_t)

    .. py:attribute:: ACKNOWLEDGE

       Acknowledge messages. Default True. Makes chirp connection-synchronous.
       See :ref:`modes-of-operation`. Python boolean expected.

    .. py:attribute:: DISABLE_SIGNALS

       By default chirp closes on SIGINT (Ctrl-C) and SIGTERM. Python boolean
       expected.

    .. py:attribute:: BUFFER_SIZE

       Size of the buffer used for a connection. Defaults to 0, which means
       use the size requested by libuv. Should not be set below 1024.
       (uint32_t)

    .. py:attribute:: MAX_MSG_SIZE

       Max message size accepted by chirp. (uint32_t)

       If you are concerned about memory usage set config.MAX_HANDLERS=1 and
       config.MAX_MSG_SIZE to something small, depending on your use-case. If
       you do this, a connection will use about:

       conn_buffers_size = config.BUFFER_SIZE +
          min(config.BUFFER_SIZE, CH_ENC_BUFFER_SIZE) +
          sizeof(ch_connection_t) +
          sizeof(ch_message_t) +
          $(memory allocated by TLS implementation)

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
       all chars are 0, which means chirp will generate a IDENTITY. Python
       bytes of length 16. Everything else will not be accepted by CFFI.

    .. py:attribute:: CERT_CHAIN_PEM

       Path to the verification certificate. Python string.

    .. py:attribute:: DH_PARAMS_PEM

       Path to the file containing DH parameters. Python string.

    .. py:attribute:: DISABLE_ENCRYPTION

       Disables encryption. Only use if you know what you are doing.
       Connections to "127.0.0.1" and "::1" aren't encrypted anyways. Python
       boolean expected.

    .. py:attribute:: AUTO_RELEASE

       By default chirp will release the message automatically when the
       handler-callback returns. Python boolean.

       In synchronous mode the remote will only send the next message if the
       current message has been released.

       In asynchronous mode when all handlers are used up and the TCP-buffers
       are filled up, the remote will eventually not be able to send more
       messages. After TIMEOUT seconds messages start to time out.

       See :ref:`modes-of-operation`
    """

    _ips     = ('BIND_V4', 'BIND_V6')
    _bools   = ('ACKNOWLEDGE', 'DISABLE_SIGNALS', 'DISABLE_ENCRYPTION')
    _strings = ('CERT_CHAIN_PEM', 'DH_PARAMS_PEM')

    def __init__(self):
        dself = self.__dict__
        dself['AUTO_RELEASE'] = True
        conf = new_nozero("ch_config_t *")
        dself['_conf'] = conf
        lib.ch_chirp_config_init(conf)

    def __setattr__(self, name, value):
        """Set attributes to the ffi object.

        Most attributes are directly set, strings and bools are converted.
        """
        conf = self.__dict__['_conf']
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
        conf = self.__dict__['_conf']
        if name in Config._ips:
            return ip_address(bytes(getattr(conf, name))).compressed
        elif name in Config._bools:
            return bool(getattr(conf, name)[0])
        elif name in Config._strings:
            return ffi.string(getattr(conf, name)).decode("UTF-8")
        else:
            return getattr(conf, name)


class Message(object):
    """Chirp message. TODO document.

    .. note::

       The properties of the message use asserts to check if the value has the
       correct type, length, range. You can disable these with python -O.
    """

    __slots__ = (
        '_msg',
        '_kheader',
        '_kdata',
        '_identity',
        '_serial',
        '_header',
        '_data',
        '_address',
        '_port',
        '_remote_identity',
    )

    def __init__(self, cmsg=None):
        self._msg = cmsg
        self._copy_from_c()
        if cmsg:
            lib.ch_msg_free_data(self._msg)

    def _ensure_message(self):
        """Ensure that a message exists."""
        msg = self._msg
        if not msg:
            msg = new_nozero("ch_message_t *")
            lib.ch_msg_init(msg)
            self._msg = msg
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
        """Get identity."""
        return self._identity

    @property
    def serial(self):
        """Get serial."""
        return self._serial

    @property
    def header(self):
        """Get header."""
        return self._header

    @header.setter
    def header(self, value):
        """Set header."""
        assert isinstance(value, bytes)
        self._header = value

    @property
    def data(self):
        """Get data."""
        return self._data

    @data.setter
    def data(self, value):
        """Set data."""
        assert isinstance(value, bytes)
        self._data = value

    @property
    def address(self):
        """Get address."""
        return self._address.compressed

    @address.setter
    def address(self, value):
        """Set address."""
        self._address = ip_address(value)

    @property
    def port(self):
        """Get port."""
        return self._port

    @port.setter
    def port(self, value):
        """Set port."""
        assert value >= 0 and value <= 2**16
        self._port = value

    @property
    def has_recv_handler(self):
        """TODO document."""
        msg = self._ensure_message()
        return lib.ch_msg_has_recv_handler(msg) == 1

    @property
    def remote_identity(self):
        """Get remote_identity."""
        return self._remote_identity

    def release(self):
        """TODO document."""
        if self.has_recv_handler:
            # TODO test
            lib.ch_chirp_release_message(self._msg)
            self._msg = None
