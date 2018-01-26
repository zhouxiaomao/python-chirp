"""cffi for libchirp binding."""
import sys
from cffi import FFI

libs = [
    "uv",
]

cflags = []
ldflags = []

if sys.platform == "win32":
    libs.extend([
        "advapi32",
        "iphlpapi",
        "psapi",
        "shell32",
        "user32",
        "userenv",
        "ws2_32",
        "kernel32",
        "libeay32",
        "ssleay32",
        "gdi32",
        "crypt32",
    ])
    ldflags.extend(["/LIBPATH:openssl\\lib"])
else:
    libs.extend([
        "m",
        "pthread",
        "ssl",
        "crypto",
    ])
    if sys.platform != "darwin":
        libs.append("rt")
    else:
        cflags.append("-I/usr/local/opt/openssl/include")
        ldflags.append("-L/usr/local/opt/openssl/lib")

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

/* libchirp init */
void
ch_en_set_manual_tls_init(void);

ch_error_t
ch_libchirp_cleanup(void);

ch_error_t
ch_libchirp_init(void);
"""

ffibuilder.set_source(
    "_libchirp_cffi",
    _source,
    libraries=libs,
    extra_compile_args=cflags,
    extra_link_args=ldflags,
)
ffibuilder.cdef(_header)

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
