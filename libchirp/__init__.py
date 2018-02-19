"""Main module of libchirp, containing common and low level bindings."""
import atexit
from ipaddress import ip_address
import sys

from _libchirp_cffi import ffi, lib  # noqa

lib.ch_en_set_manual_tls_init()  # del_if_chirp_is_built_with_static_openssl

assert lib.ch_libchirp_init() == lib.CH_SUCCESS
atexit.register(lambda: lib.ch_libchirp_cleanup())


class Config(object):
    """Chirp configuration. TODO document."""

    _ips     = ('BIND_V4', 'BIND_V6')
    _bools   = ('ACKNOWLEDGE', 'DISABLE_SIGNALS', 'DISABLE_ENCRYPTION')
    _strings = ('CERT_CHAIN_PEM', 'DH_PARAMS_PEM')

    def __init__(self):
        conf = ffi.new("ch_config_t *")
        self.__dict__['_conf'] = conf
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
