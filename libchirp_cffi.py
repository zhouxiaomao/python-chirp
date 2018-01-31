"""cffi for libchirp binding."""
import sys
import os
from os import path
import platform
from cffi import FFI

here = os.environ.get("LICHIRP_HERE") or path.abspath(path.dirname(__file__))

libs = []
libdirs = []
incdirs = [here]

if sys.platform == "win32":
    if platform.architecture()[0] == "64bit":
        bits = "64"
    else:
        bits = "32"
    uvdir = "libuv-2015-%s-release" % bits
    ssldir = "openssl-2015-%s-release" % bits
    libdirs.append(path.join(here, "..", "libuv-build", uvdir, "lib"))
    libdirs.append(path.join(here, "..", "openssl-build", ssldir, "lib"))
    incdirs.append(path.join(here, "..", "libuv-build", uvdir, "include"))
    incdirs.append(path.join(here, "..", "openssl-build", ssldir, "include"))
    libs.extend([
        "libuv",
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
else:
    libs.extend([
        "uv",
        "m",
        "pthread",
        "ssl",
        "crypto",
    ])
    if sys.platform != "darwin":
        libs.append("rt")
    else:
        incdirs.append("/usr/local/opt/openssl/include")
        libdirs.append("/usr/local/opt/openssl/lib")

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
    library_dirs=libdirs,
    include_dirs=incdirs,
)
ffibuilder.cdef(_header)

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
