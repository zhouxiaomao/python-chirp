"""Main module of libchirp, containing low level bindings."""
import atexit

from _libchirp_cffi import ffi, lib  # noqa

lib.ch_en_set_manual_tls_init()  # del_if_chirp_is_built_with_static_openssl

assert lib.ch_libchirp_init() == lib.CH_SUCCESS
atexit.register(lambda: lib.ch_libchirp_cleanup())
