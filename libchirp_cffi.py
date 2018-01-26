"""cffi for libchirp binding."""
from cffi import FFI

ffibuilder = FFI()

with open("libchirp.c") as f:
    _source = f.read()

_header = """
typedef enum {
    CH_SUCCESS        = 0,
    CH_VALUE_ERROR    = 1,
    CH_UV_ERROR       = 2,
    CH_PROTOCOL_ERROR = 3,
    CH_EADDRINUSE     = 4,
    CH_FATAL          = 5,
    CH_TLS_ERROR      = 6,
    CH_UNINIT         = 7,
    CH_IN_PRORESS     = 8,
    CH_TIMEOUT        = 9,
    CH_ENOMEM         = 10,
    CH_SHUTDOWN       = 11,
    CH_CANNOT_CONNECT = 12,
    CH_QUEUED         = 13,
    CH_USED           = 14,
    CH_MORE           = 15,
    CH_BUSY           = 16,
    CH_EMPTY          = 17,
    CH_WRITE_ERROR    = 18,
    CH_INIT_FAIL      = 19,
} ch_error_t;

ch_error_t
ch_libchirp_init(void);
"""

ffibuilder.set_source("_libchirp_cffi", _source)
ffibuilder.cdef(_header)

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
