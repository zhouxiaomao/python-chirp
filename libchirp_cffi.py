"""cffi for libchirp binding."""
from cffi import FFI
import os
from os import path
import platform
import sys

here = os.environ.get("LICHIRP_HERE") or path.abspath(path.dirname(__file__))
static = os.environ.get("LIBCHIRP_STATIC") == "True"

comp = ["-DCH_DISABLE_SIGNALS"]
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
    comp.extend(["-std=gnu99"])
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
    CH_SUCCESS         = 0,
    CH_VALUE_ERROR     = 1,
    CH_UV_ERROR        = 2,
    CH_PROTOCOL_ERROR  = 3,
    CH_EADDRINUSE      = 4,
    CH_FATAL           = 5,
    CH_TLS_ERROR       = 6,
    CH_NOT_INITIALIZED = 7,
    CH_IN_PRORESS      = 8,
    CH_TIMEOUT         = 9,
    CH_ENOMEM          = 10,
    CH_SHUTDOWN        = 11,
    CH_CANNOT_CONNECT  = 12,
    CH_QUEUED          = 13,
    CH_USED            = 14,
    CH_MORE            = 15,
    CH_BUSY            = 16,
    CH_EMPTY           = 17,
    CH_WRITE_ERROR     = 18,
    CH_INIT_FAIL       = 19,
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

struct uv_timer_s;
typedef struct uv_timer_s uv_timer_t;
struct uv_loop_s;
typedef struct uv_loop_s uv_loop_t;
struct uv_handle_s;
typedef struct uv_handle_s uv_handle_t;
struct uv_async_s;
typedef struct uv_async_s uv_async_t;
struct ch_chirp_s;
typedef struct ch_chirp_s ch_chirp_t;
struct ch_config_s;
typedef struct ch_config_s ch_config_t;
struct ch_message_s;
typedef struct ch_message_s ch_message_t;

// Callbacks

typedef void (*uv_timer_cb)(uv_timer_t* handle);
typedef void (*ch_done_cb_t)(ch_chirp_t* chirp);
typedef void (*ch_log_cb_t)(char msg[], char error);
typedef void (*ch_send_cb_t)(
        ch_chirp_t* chirp, ch_message_t* msg, ch_error_t status);
typedef void (*ch_recv_cb_t)(ch_chirp_t* chirp, ch_message_t* msg);
typedef void (*ch_start_cb_t)(ch_chirp_t* chirp);
typedef void (*ch_release_cb_t)(
        ch_chirp_t* chirp, uint8_t identity[CH_ID_SIZE], uint32_t serial);

typedef void (*uv_close_cb)(uv_handle_t* handle);
typedef void (*uv_async_cb)(uv_async_t* handle);

extern "Python" void _timer_close_cb(uv_handle_t* handle);
extern "Python" void _request_timeout_cb(uv_timer_t*);
extern "Python" void _loop_async_cb(uv_async_t*);
extern "Python" void _chirp_log_cb(char msg[], char error);
extern "Python" void _chirp_done_cb(ch_chirp_t* chirp);
extern "Python" void _send_cb(
        ch_chirp_t* chirp, ch_message_t* msg, ch_error_t status);
extern "Python" void _queue_recv_cb(ch_chirp_t* chirp, ch_message_t* msg);
extern "Python" void _pool_recv_cb(ch_chirp_t* chirp, ch_message_t* msg);
extern "Python" void _async_recv_cb(ch_chirp_t* chirp, ch_message_t* msg);
extern "Python" void _release_cb(
        ch_chirp_t* chirp, uint8_t identity[CH_ID_SIZE], uint32_t serial);
// UV

typedef enum {
  UV_RUN_DEFAULT = 0,
  UV_RUN_ONCE,
  UV_RUN_NOWAIT
} uv_run_mode;

struct uv_timer_s {
  void* data;
  ...;
};

struct uv_loop_s {
  void* data;
  ...;
};

struct uv_handle_s {
  void* data;
  uv_loop_t* loop;
  ...;
};

struct uv_async_s {
  void* data;
  uv_loop_t* loop;
  ...;
};

int
uv_loop_init(uv_loop_t* loop);

int
uv_loop_close(uv_loop_t* loop);

int
uv_run(uv_loop_t*, uv_run_mode mode);

void
uv_stop(uv_loop_t*);

void
uv_close(uv_handle_t* handle, uv_close_cb close_cb);

int
uv_async_init(uv_loop_t*, uv_async_t* async, uv_async_cb async_cb);

int
uv_async_send(uv_async_t* async);

int
uv_timer_init(uv_loop_t* loop, uv_timer_t* handle);

int
uv_timer_start(
    uv_timer_t* handle,
    uv_timer_cb cb,
    uint64_t timeout,
    uint64_t repeat
);

int
uv_timer_stop(uv_timer_t* handle);

// Config

struct ch_config_s {
    float    REUSE_TIME;
    float    TIMEOUT;
    uint16_t PORT;
    uint8_t  BACKLOG;
    uint8_t  MAX_SLOTS;
    char     SYNCHRONOUS;
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
    uint8_t         ip_protocol;
    uint8_t         address[CH_IP_ADDR_SIZE]; // 16
    int32_t         port;
    uint8_t         remote_identity[CH_ID_SIZE];
    void*           user_data;
    uint8_t         _flags;
    ch_send_cb_t    _send_cb;
    ch_release_cb_t _release_cb;
    uint8_t         _slot;
    void*           _pool;
    void*           _ssl_context;
    ch_message_t*   _next;
};

void
ch_msg_free_data(ch_message_t* message);

ch_error_t
ch_msg_init(ch_message_t* message);

int
ch_msg_has_slot(ch_message_t* message);

ch_error_t
ch_chirp_release_msg_slot_ts(
        ch_chirp_t* rchirp, ch_message_t* msg, ch_release_cb_t release_cb);

// Chirp

struct ch_chirp_s {
    void*           user_data;
    ...;
};

ch_error_t
ch_chirp_init(
        ch_chirp_t*        chirp,
        const ch_config_t* config,
        uv_loop_t*         loop,
        ch_recv_cb_t       recv_cb,
        ch_start_cb_t      start_cb,
        ch_done_cb_t       done_cb,
        ch_log_cb_t        log_cb);

ch_error_t
ch_chirp_close_ts(ch_chirp_t* chirp);

int
ch_loop_close(uv_loop_t* loop);

ch_error_t
ch_chirp_send_ts(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb);

typedef struct ch_identity_s {
    uint8_t data[CH_ID_SIZE];
} ch_identity_t;

ch_identity_t
ch_chirp_get_identity(ch_chirp_t* chirp);
"""
if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "debug":
        print("debug build")
        comp.extend([
            "-ggdb3", "-O0", "-DCH_ENABLE_LOGGING", "-DCH_ENABLE_ASSERTS",
            "-UNDEBUG"
        ])

ffibuilder.set_source(
    "_libchirp_cffi",
    _source,
    libraries=libs,
    library_dirs=libdirs,
    include_dirs=incdirs,
    extra_compile_args=comp,
    extra_link_args=link,
)
ffibuilder.cdef(_header)

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "debug":
        ffibuilder.compile(verbose=True, debug=True)
    else:
        ffibuilder.compile(verbose=True)
