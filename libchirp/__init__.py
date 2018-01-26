"""Main module of libchirp, containing low level bindings."""
import sys
import atexit

from _libchirp_cffi import ffi, lib  # noqa

if sys.platform != "win32":
    lib.ch_en_set_manual_tls_init()

assert lib.ch_libchirp_init() == lib.CH_SUCCESS
atexit.register(lambda: lib.ch_libchirp_cleanup())
