"""cffi for libchirp binding."""
import sys
import os
from os import path
import platform
from cffi import FFI

here = os.environ.get("LICHIRP_HERE") or path.abspath(path.dirname(__file__))
static = os.environ.get("LIBCHIRP_STATIC") == "True"

link = []
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
        "m",
        "pthread",
    ])
    if static:
        link.extend(
            path.join(here, lib) for lib in
            ["libuv.a", "libcrypto.a", "libssl.a"]
        )
    else:
        libs.extend([
            "uv",
            "ssl",
            "crypto",
        ])
    if sys.platform != "darwin":
        libs.append("rt")
    else:
        if not static:
            incdirs.append("/usr/local/opt/openssl/include")
            libdirs.append("/usr/local/opt/openssl/lib")

ffibuilder = FFI()

with open("libchirp.c") as f:
    _source = f.read()

_header = """
typedef char ch_buf;

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

#define CH_IP_ADDR_SIZE 16
#define CH_IP4_ADDR_SIZE 4
#define CH_ID_SIZE 16

// Forward decls

struct ch_chirp_s;
typedef struct ch_chirp_s ch_chirp_t;
struct ch_config_s;
typedef struct ch_config_s ch_config_t;
struct ch_message_s;
typedef struct ch_message_s ch_message_t;

// Callbacks

typedef void (*ch_done_cb_t)(ch_chirp_t* chirp);
typedef void (*ch_log_cb_t)(char msg[], char error);
typedef void (*ch_send_cb_t)(
        ch_chirp_t* chirp, ch_message_t* msg, ch_error_t status);
typedef void (*ch_recv_cb_t)(ch_chirp_t* chirp, ch_message_t* msg);
typedef void (*ch_start_cb_t)(ch_chirp_t* chirp);

// Config

struct ch_config_s {
    float    REUSE_TIME;
    float    TIMEOUT;
    uint16_t PORT;
    uint8_t  BACKLOG;
    uint8_t  MAX_HANDLERS;
    char     ACKNOWLEDGE;
    char     DISABLE_SIGNALS;
    uint32_t BUFFER_SIZE;
    uint32_t MAX_MSG_SIZE;
    uint8_t  BIND_V6[CH_IP_ADDR_SIZE];
    uint8_t  BIND_V4[CH_IP4_ADDR_SIZE];
    uint8_t  IDENTITY[CH_ID_SIZE]; // 16
    char*    CERT_CHAIN_PEM;
    char*    DH_PARAMS_PEM;
    char     DISABLE_ENCRYPTION;
};

void
ch_chirp_config_init(ch_config_t* config);

// Message

struct ch_message_s {
    // Network data, has to be sent in network order
    uint8_t  identity[CH_ID_SIZE];
    uint32_t serial;
    uint8_t  type;
    uint16_t header_len;
    uint32_t data_len;
    // These fields follow the message in this order (see *_len above)
    ch_buf* header;
    ch_buf* data;
    // Local       only data
    uint8_t       ip_protocol;
    uint8_t       address[CH_IP_ADDR_SIZE]; // 16
    int32_t       port;
    uint8_t       remote_identity[CH_ID_SIZE];
    void*         user_data;
    uint8_t       _flags;
    ch_send_cb_t  _send_cb;
    uint8_t       _handler;
    void*         _pool;
    ch_message_t* _next;
};

void
ch_msg_free_data(ch_message_t* message);

ch_error_t
ch_msg_init(ch_message_t* message);

int
ch_msg_has_recv_handler(ch_message_t* message);

void
ch_chirp_release_message(ch_message_t* msg);
"""

ffibuilder.set_source(
    "_libchirp_cffi",
    _source,
    libraries=libs,
    library_dirs=libdirs,
    include_dirs=incdirs,
    extra_link_args=link,
)
ffibuilder.cdef(_header)

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
