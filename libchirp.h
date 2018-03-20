/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// ================================
// libchirp 1.0.0-beta amalgamation
// ================================

// =========
// Constants
// =========
//
// Defines constants used throughout the project.
//
// .. code-block:: cpp

#ifndef ch_libchirp_const_h
#define ch_libchirp_const_h

// System includes
// ===============
//
// .. code-block:: cpp
//
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else // _WIN32
#include <arpa/inet.h>
#endif // _WIN32

// Declarations
// ============

// .. c:type:: ch_ip_protocol_t
//
//    This definition is for documentation purposes only. The user should use
//    AF_INET/AF_INET6.
//
//    IP protocol definition. This is either IPV4 or IPV6.
//
//    .. c:member:: CH_IPV4
//
//       Defines the usage of IP protocol version 4.
//
//    .. c:member:: CH_IPV6
//
//       Defines the usage of IP protocol version 6.
//
// .. code-block:: cpp
//
typedef enum { _CH_IPV4 = AF_INET, _CH_IPV6 = AF_INET6 } ch_ip_protocol_t;

// The maximum size of an IP address
//
// .. code-block:: cpp
//
#define CH_IP_ADDR_SIZE 16

// The size of an IP4 address
//
// .. code-block:: cpp
//
#define CH_IP4_ADDR_SIZE 4

// The size of an id
//
// .. code-block:: cpp
//
#define CH_ID_SIZE 16

#endif // ch_libchirp_const_h
// ======
// Errors
// ======
//
// Chirp error constants.
//
// .. code-block:: cpp

#ifndef ch_libchirp_error_h
#define ch_libchirp_error_h

// .. c:type:: ch_error_t
//
//    Represents a chirp error.
//
//    .. c:member:: CH_SUCCESS
//
//       No error.
//
//    .. c:member:: CH_VALUE_ERROR
//
//       Supplied value is not allowed.
//
//    .. c:member:: CH_UV_ERROR
//
//       General libuv error.
//
//    .. c:member:: CH_PROTOCOL_ERROR
//
//       Happens when bad values are received or the remote dies unexpectedly.
//
//    .. c:member:: CH_EADDRINUSE
//
//       The chirp port is already in use.
//
//    .. c:member:: CH_FATAL
//
//       Error that should not happen in normal operation, but the underlaying
//       system has errors for.
//
//       * We do not have an entropy source
//
//       * Closing closed chirp
//
//       * Cannot get remote IP-Address
//
//       * Cannot accept remote connection
//
//       * Cannot set socket options
//
//    .. c:member:: CH_TLS_ERROR
//
//       General TLS error.
//
//    .. c:member:: CH_WRITE_ERROR
//
//       Error while writing to socket.
//
//    .. c:member:: CH_NOT_INITIALIZED
//
//       Chirp or another object is not initialized.
//
//    .. c:member:: CH_IN_PROGRESS
//
//       Action is already in progress.
//
//    .. c:member:: CH_TIMEOUT
//
//       A timeout happened during an action.
//
//    .. c:member:: CH_ENOMEM
//
//       Could not get memory. We consider this as totally fatal, but try to
//       handle it transparent for the user. We try to chain this error up to
//       the user, but often it might only be logged. In debug mode it is
//       asserted.
//
//    .. c:member:: CH_SHUTDOWN
//
//       Indicates that error occurred because chirp is shutting down. For
//       example the connection that is currently sending a message got closed.
//
//    .. c:member:: CH_CANNOT_CONNECT
//
//       Indicates that the remote has refused the connection or has timed out.
//
//    .. c:member:: CH_QUEUED
//
//       Indicates that the message as been placed in the send queue.
//
//    .. c:member:: CH_USED
//
//       Indicates that the message is already in use. The message will not be
//       sent.
//
//    .. c:member:: CH_MORE
//
//       Indicates that the message has not been sent completely.
//
//    .. c:member:: CH_BUSY
//
//       Indicates that the writer was busy or we are still busy waiting for an
//       ack. Therefore no message was processed.
//
//    .. c:member:: CH_EMPTY
//
//       Indicates that queues are empty and no message has been sent.
//
//    .. c:member:: CH_INIT_FAIL
//
//       Initializing some resource failed.
//
// .. code-block:: cpp
//
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

#endif // ch_libchirp_error_h
// =============
// Common header
// =============
//
// .. code-block:: cpp
//
#ifndef ch_libchirp_common_h
#define ch_libchirp_common_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "libchirp/const.h" */
/* #include "libchirp/error.h" */

// Library export
// ==============
//
// .. code-block:: cpp
//
#ifdef CH_BUILD
#if defined __GNUC__ || __clang__
#define CH_EXPORT __attribute__((visibility("default")))
#endif
#else // CH_BUILD
#define CH_EXPORT
#endif

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <stdint.h>
#include <stdio.h>
#include <uv.h>

// Typedefs
// ========
//
// .. code-block:: cpp

typedef char ch_buf; /* Used to show it is not a c-string but a buffer. */

// .. c:type:: ch_text_address_t
//
//    Type to be used with :c:func:`ch_msg_get_address`. Used for the textual
//    representation of the IP-address.
//
// .. code-block:: cpp
//
typedef struct ch_text_address_s {
    char data[INET6_ADDRSTRLEN];
} ch_text_address_t;

// .. c:type:: ch_identity_t
//
//    Struct containing the chirp identity.
//
//    .. c:member:: unsigned uint8_t[16] data
//
//       The chirp identity is uint8_t array of length 16.
//
// .. code-block:: cpp
//
typedef struct ch_identity_s {
    uint8_t data[CH_ID_SIZE];
} ch_identity_t;

// Forward declarations
// =====================
//
// .. code-block:: cpp

struct ch_chirp_s;
typedef struct ch_chirp_s ch_chirp_t;
struct ch_config_s;
typedef struct ch_config_s ch_config_t;
struct ch_message_s;
typedef struct ch_message_s ch_message_t;

// Logging
// =======

// .. c:macro:: CH_WRITE_LOG
//
//    Logs the given message including arbitrary arguments to a custom callback
//    in debug-/development-mode.
//
//    The logging macro CH_WRITE_LOG(chirp, message, ...) behaves like printf
//    and allows to log to a custom callback. Usually used to log into pythons
//    logging facility.
//
//    The logging macro CH_WRITE_LOGC(chirp, message, clear, ...) behaves like
//    CH_WRITE_LOG except you can add part that isn't highlighted.
//
//    :param chirp:   Pointer to a chirp object.
//    :param message: The highlighted message to report.
//    :param clear:   The clear message (not highlighted to report.
//    :param ...:     Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#define CH_WRITE_LOGC(chirp, message, clear, ...)                              \
    ch_write_log(chirp, __FILE__, __LINE__, message, clear, 0, __VA_ARGS__);
#define CH_WRITE_LOG(chirp, message, ...)                                      \
    CH_WRITE_LOGC(chirp, message, "", __VA_ARGS__)

#define CH_NO_ARG 1

// .. c:function::
CH_EXPORT void
ch_write_log(
        ch_chirp_t* chirp,
        char*       file,
        int         line,
        char*       message,
        char*       clear,
        int         error,
        ...);
//
//    Write log message, either to logging callback (if defined) or to stderr.
//
//    :param char* file: file to log
//    :param int line: line to log
//    :param char* message: message to log
//    :param char* clear: clear message to log
//    :param int error: message is an error
//    :param ...: variable args passed to vsnprintf

#endif // ch_libchirp_common_h
// ===============
// Chirp Callbacks
// ===============
//
// .. code-block:: cpp
//
/* clang-format off */

// If you are on an embedded platform you have to set the memory functions of
// chirp, libuv and openssl.
//
// * ch_set_allocators
// * uv_replace_allocator_
// * CRYPTO_set_mem_functions
//
// .. _uv_replace_allocator: http://docs.libuv.org/en/v1.x/misc.html
// .. _CRYPTO_set_mem_functions: https://www.openssl.org/docs/man1.1.0/crypto/OPENSSL_malloc.html
//
// .. code-block:: cpp
//
/* clang-format on */

#ifndef ch_libchirp_callbacks_h
#define ch_libchirp_callbacks_h

/* #include "common.h" */

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <stdlib.h>

// .. c:type:: ch_alloc_cb_t
//
//    Callback used by chirp to request memory.
//
//    .. c:member:: size_t size
//
//       The size to allocate
//
// .. code-block:: cpp
//
typedef void* (*ch_alloc_cb_t)(size_t size);

// .. c:type:: ch_done_cb_t
//
//    Callback called when chirp has closed.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Chirp object closed.
//
// .. code-block:: cpp
//
typedef void (*ch_done_cb_t)(ch_chirp_t* chirp);

// .. c:type:: ch_free_cb_t
//
//    Callback used by chirp to free memory.
//
// .. code-block:: cpp
//
typedef void (*ch_free_cb_t)(void* buf);

// .. c:type:: ch_log_cb_t
//
//    Logging callback
//
//    .. c:member:: char msg[]
//
//       The message to log
//
//    .. c:member:: char error
//
//       The message is a error
//
// .. code-block:: cpp

typedef void (*ch_log_cb_t)(char msg[], char error);

// .. c:type:: ch_send_cb_t
//
//    Called by chirp when message is sent and can be freed.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Chirp instance sending
//
//    .. c:member:: int status
//
//       Error code: CH_SUCCESS, CH_TIMEOUT, CH_CANNOT_CONNECT, CH_TLS_ERROR,
//       CH_WRITE_ERROR, CH_SHUTDOWN, CH_FATAL, CH_PROTOCOL_ERROR, CH_ENOMEM
//
// .. code-block:: cpp
//
typedef void (*ch_send_cb_t)(
        ch_chirp_t* chirp, ch_message_t* msg, ch_error_t status);

// .. c:type:: ch_recv_cb_t
//
//    Called by chirp when message is received.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Chirp instance receiving
//
//    .. c:member:: ch_message_t* msg
//
//       Received message. The address is the remote address, so changing only
//       the user_data and send it, will send the message back to the sender.
//
// .. code-block:: cpp
//
typedef void (*ch_recv_cb_t)(ch_chirp_t* chirp, ch_message_t* msg);

// .. c:type:: ch_release_cb_t
//
//    Called by chirp when message is released.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Chirp instance
//
//    .. c:member:: uint8_t identity[CH_ID_SIZE] identity
//
//       Identity of the message released
//
//    .. c:member:: uint32_t serial
//
//       Serial of the message released
//
// .. code-block:: cpp
//
typedef void (*ch_release_cb_t)(
        ch_chirp_t* chirp, uint8_t identity[CH_ID_SIZE], uint32_t serial);

// .. c:type:: ch_start_cb_t
//
//    Callback called when chirp is started
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Chirp instance started
//
// .. code-block:: cpp
//
typedef void (*ch_start_cb_t)(ch_chirp_t* chirp);

// .. c:type:: ch_realloc_cb_t
//
//    Callback used by chirp to request memory reallocation.
//
//    .. c:member:: void* buf
//
//       The Buffer to reallocate
//
//    .. c:member:: size_t new_size
//
//       The requested new size
//
// .. code-block:: cpp
//
typedef void* (*ch_realloc_cb_t)(void* buf, size_t new_size);

#endif // ch_libchirp_callbacks_h
// =======
// Message
// =======
//
// The message structure, initialization and setting/getting address.
//
// .. code-block:: cpp
//
#ifndef ch_libchirp_message_h
#define ch_libchirp_message_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "callbacks.h" */
/* #include "chirp.h" */
/* #include "common.h" */

// Declarations
// ============

// .. c:type:: ch_message_t
//
//    Represents a message. Although many members are not marked as private.
//    Use the APIs to access the message. Never change the identity of a
//    message.
//
//    .. c:member:: uint8_t[16] identity
//
//       Identify the message and answers to it. Never change the identity of
//       message. The identity can be used to find answers to a message, since
//       replying to message won't change the identity.
//
//       If you need to uniquely identify the message, use the identity/serial
//       pair, since the serial will change when replying to messages.
//
//    .. c:member:: uint32_t serial
//
//       The serial number of the message. Increases monotonic. Be aware of
//       overflows, if want to use it for ordering use the delta: serialA -
//       serialB. Only received messages have a serial. But also received
//       messages can have the serial 0.
//
//    .. c:member:: uint8_t type
//
//       The type of the message.
//
//    .. c:member:: uint16_t header_len
//
//       Length of the message header.
//
//    .. c:member:: uint32_t data_len
//
//       Length of the data the message contains.
//
//    .. c:member:: ch_buf* header
//
//       Header used by upper-layer protocols. Users should not use it, except
//       if you know what you are doing. ch_buf* is an alias for char* and
//       denotes to binary buffer: the length has to be supplied (header_len)
//
//    .. c:member:: ch_buf* data
//
//       The data of the message as pointer to a buffer. ch_buf* is an alias
//       for char* and denotes to binary buffer: the length has to be supplied
//       (data_len). Please use :c:func:`ch_msg_set_data` to set this.
//
//    .. c:member:: uint8_t ip_protocol
//
//       The IP protocol which was / shall be used for this message.
//
//       If the message was received: The protocol of the remote the message
//       was received from.
//
//       If the message will be sent: The protocol to send the message to.
//
//       This allows to reply to messages just by replacing the data
//       :c:func:`ch_msg_set_data`.
//
//       This may either be IPv4 or IPv6. See :c:type:`ch_ip_protocol_t`.
//       Please use :c:func:`ch_msg_set_address` to set this.
//
//    .. c:member:: uint8_t[16] address
//
//       IPv4/6 address.
//
//       If the message was received: The address of the remote the message was
//       received from.
//
//       If the message will be sent: The address to send the message to.
//
//       This allows to reply to messages just by replacing the data
//       :c:func:`ch_msg_set_data`.
//
//       Please use :c:func:`ch_msg_set_address` to set and
//       :c:func:`ch_msg_get_address` to get the address.
//
//    .. c:member:: int32_t port
//
//       The port.
//
//       If the message was received: The port of the remote the message was
//       received from.
//
//       If the message will be sent: The port to send the message to.
//
//       This allows to reply to messages just by replacing the data
//       :c:func:`ch_msg_set_data`.
//
//       Please use :c:func:`ch_msg_set_address` to set the port.
//
//    .. c:member:: uint8_t remote_identity[CH_ID_SIZE]
//
//       Detect the remote instance. By default a node's identity will change
//       on each start of chirp. If multiple peers share state, a change in the
//       remote_identity should trigger a reset of the state. Simply use the
//       remote_identity as key in a dictionary of shared state.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Pointer to chirp instance of the message pool.
//
//    .. c:member:: void* user_data
//
//       Pointer to user data, can be used to access user-data in the
//       send-callback.
//
// .. code-block:: cpp
//
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

// Protocol receiver /Pseudo code/
//
// .. code-block:: cpp
//
//    ch_message_t msg;
//    recv_wait(buffer=&msg, size=39)
//    if(msg.header_len) {
//        msg.header = malloc(msg.header_len) *
//        recv_exactly(buffer=msg.header, msg.header_len)
//    }
//    if(msg.data_len) {
//        msg.data  = malloc(msg.data_len) *
//        recv_exactly(buffer=msg.data, msg.data_len)
//    }
//
// * Please use MAX_SLOTS preallocated buffers of size 32 for header
// * Please use MAX_SLOTS preallocated buffers of size 512 for data
//
// Either fields may exceed the limit, in which case you have to alloc and set
// the free_* field.

// .. c:function::
CH_EXPORT
void
ch_msg_free_data(ch_message_t* message);
//
//    Frees data attached to the message. This function is mostly intended to
//    implement bindings, where you can to copy the data into memory provided
//    by the host language, so it can freely garbage-collect the data.
//
//    After calling this the data/header fields will be NULL.
//
//    :param ch_message_t* message: Pointer to the message

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_get_address(const ch_message_t* message, ch_text_address_t* address);
//
//    Get the messages' address which is an IP-address. The port and
//    ip_protocol can be read directly from the message. Address must be of the
//    size INET(6)_ADDRSTRLEN.
//
//    :param ch_message_t* message: Pointer to the message
//    :param ch_text_address_t* address: Textual representation of IP-address
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
CH_EXPORT
ch_identity_t
ch_msg_get_identity(ch_message_t* message);
//
//    Get the identity of the message.
//
//    :param ch_message_t* message: Pointer to the message
//
//    :rtype:  ch_identity_t

// .. c:function::
CH_EXPORT
ch_identity_t
ch_msg_get_remote_identity(ch_message_t* message);
//
//    Get the identity of the remote chirp instance.
//
//    By default the remote_identity will change on each start of chirp. If
//    multiple peers share state, a change in the remote_identity should
//    trigger a reset of the state. Simply use the remote_identity as key in a
//    dictionary of shared state.
//
//    :param ch_message_t* message: Pointer to the message
//
//    :rtype:  ch_identity_t

// .. c:function::
CH_EXPORT
int
ch_msg_has_slot(ch_message_t* message);
//
//    Returns 1 if the message has a slot and therefore you have to call
//    ch_chirp_release_msg_slot.
//
//    :param ch_message_t* message: Pointer to the message
//
//    :rtype:  bool

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_init(ch_message_t* message);
//
//    Initialize a message. Memory provided by caller (for performance).
//
//    :param ch_message_t* message: Pointer to the message
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_set_address(
        ch_message_t*    message,
        ch_ip_protocol_t ip_protocol,
        const char*      address,
        int32_t          port);
//
//    Set the messages' address in terms of IP-address and port.
//
//    :param ch_message_t* message: Pointer to the message
//    :param ch_ip_protocol_t ip_protocol: IP-protocol of the address
//    :param char* address: Textual representation of IP
//    :param int32_t port: Port of the remote
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
CH_EXPORT
void
ch_msg_set_data(ch_message_t* message, ch_buf* data, uint32_t len);
//
//    Set the messages' data. ch_buf* is an alias for char* and denotes to a
//    binary buffer: the length has to be supplied. The data pointer has to be
//    valid until the :c:type:`ch_send_cb_t` supplied in
//    :c:func:`ch_chirp_send` has been called.
//
//    :param ch_message_t* message: Pointer to the message
//    :param ch_buf* data: Pointer to the data
//    :param uint32_t len: The length of the data
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. code-block:: cpp
//
#endif // ch_libchirp_message_h
// ==================
// External functions
// ==================
//
// Wrapper around commonly used libuv functions: init, close, run
//
// .. code-block:: cpp

#ifndef ch_libchirp_wrappers_h
#define ch_libchirp_wrappers_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "common.h" */

// Declarations
// ============

// .. c:function::
CH_EXPORT
int
ch_loop_init(uv_loop_t* loop);
//
//    An alias for uv_loop_init. Please refer to the libuv documentation.
//
//    :param uv_loop_t* loop: Loop struct allocated by user.
//
//    :return: the status of the initialization.
//    :rtype:  int
//

// .. c:function::
CH_EXPORT
int
ch_loop_close(uv_loop_t* loop);
//
//    An alias for uv_loop_close. Please refer to the libuv documentation.
//
//    :param uv_loop_t* loop: Loop struct allocated by the user.
//
//    :return: the status of the closing action.
//    :rtype:  int

// .. c:function::
CH_EXPORT
int
ch_run(uv_loop_t* loop);
//
//    A wrapper for uv_run.
//    Runs the loop once again, in case closing chirp's resources caused
//    additional requests/handlers.
//
//    Please refer to the libuv documentation.
//
//    :param uv_loop_t*  loop: Loop struct allocated by user.
//
//    :return: the status of the action.
//    :rtype:  int
//
// .. code-block:: cpp

#endif // ch_libchirp_wrappers_h
// =====
// Chirp
// =====
//
// Interface to chirp: Config, startup, closing and sending messages.
//
// .. code-block:: cpp
//
#ifndef ch_libchirp_chirp_h
#define ch_libchirp_chirp_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "callbacks.h" */
/* #include "common.h" */
/* #include "message.h" */
/* #include "wrappers.h" */

// Declarations
// ============

// .. c:type:: ch_config_t
//
//    Chirp configuration.
//
//    .. c:member:: float REUSE_TIME
//
//       Time until a connection gets garbage collected. Until then the
//       connection will be reused. Actual reuse time will be
//       max(REUSE_TIME, TIMEOUT * 3).
//
//    .. c:member:: float TIMEOUT
//
//       Send- and connect-timeout scaling in seconds. Send-timeout will be
//       TIMEOUT seconds. Connect-timeout will be min(TIMEOUT * 2, 60)
//       seconds.
//
//    .. c:member:: uint16_t PORT
//
//       Port for listening to connections.
//
//    .. c:member:: uint8_t BACKLOG
//
//       TCP-listen socket backlog.
//
//    .. c:member:: uint8_t MAX_SLOTS
//
//       The count of message-slots used. Allowed values are values between 1
//       and 32. The default is 0: Use 16 slots if SYNCHRONOUS=0 and 1
//       slot if SYNCHRONOUS=1.
//
//    .. c:member:: char SYNCHRONOUS
//
//       Enable connection-synchronous operations. Default 1. See
//       :ref:`modes-of-operation`
//
//    .. c:member:: char DISABLE_SIGNALS
//
//       By default chirp closes on SIGINT (Ctrl-C) and SIGTERM. Defaults to 0.
//
//    .. c:member:: uint32_t BUFFER_SIZE
//
//       Size of the buffer used for a connection. Defaults to 0, which means
//       use the size requested by libuv. Should not be set below 1024.
//
//    .. c:member:: uint32_t MAX_MSG_SIZE
//
//       Max message size accepted by chirp. If you are concerned about memory
//       usage set config.MAX_SLOTS=1 and config.MAX_MSG_SIZE to something
//       small, depending on your use-case. If you do this, a connection will
//       use about:
//
//       conn_buffers_size = config.BUFFER_SIZE +
//          min(config.BUFFER_SIZE, CH_ENC_BUFFER_SIZE) +
//          sizeof(ch_connection_t) +
//          sizeof(ch_message_t) +
//          (memory allocated by TLS implementation)
//
//       conn_size = conn_buffers_size + config.MAX_MSG_SIZE
//
//       With the default config and SSL conn_buffers_size should be about
//       64k + 16k + 2k + 32k -> 114k. Derived from documentation, no
//       measurement done.
//
//    .. c:member:: uint8_t[16] BIND_V6
//
//       Override IPv6 bind address.
//
//    .. c:member:: uint8_t[4] BIND_V4
//
//       Override IPv4 bind address.
//
//    .. c:member:: uint8_t[16] IDENTITY
//
//       Override the chirp-nodes IDENTITY (this chirp instance). By default
//       all chars are 0, which means chirp will generate a IDENTITY.
//
//    .. c:member:: char* CERT_CHAIN_PEM
//
//       Path to the verification certificate.
//
//    .. c:member:: char* DH_PARAMS_PEM
//
//       Path to the file containing DH parameters.
//
//    .. c:member:: char DISABLE_ENCRYPTION
//
//       Disables encryption. Only use if you know what you are doing.
//       Connections to "127.0.0.1" and "::1" aren't encrypted anyways.
//       Defaults to 0.
//
// .. code-block:: cpp
//
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

// .. c:type:: ch_chirp_int_t
//    :noindex:
//
//    Opaque pointer to internals.
//
//    see: :c:type:`ch_chirp_int_t`
//
// .. code-block:: cpp
//
typedef struct ch_chirp_int_s ch_chirp_int_t;

// .. c:type:: ch_chirp_t
//
//    Chirp object. It has no public members except user_data and uses an
//    opaque pointer to its internal data structures.
//
//    .. c:member:: void* user_data;
//
//       Pointer to user-data, which can be accessed in :c:type`ch_start_cb_t`
//       and :c:type`ch_done_cb_t` or any other callback that gives access to
//       ch_chirp_t*.
//
// .. code-block:: cpp
//
struct ch_chirp_s {
    void*           user_data;
    ch_chirp_int_t* _;
    uv_thread_t     _thread;
    ch_log_cb_t     _log;
    int             _init;
};

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_close_ts(ch_chirp_t* chirp);
//
//    Clean up chirp object. Close all connections. The chirp object can be
//    freed after the done callback passed to :c:func:`ch_chirp_init` was
//    called.
//
//    This function is thread-safe.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//
//    :return: A chirp error. See: :c:type:`ch_error_t`.
//    :rtype: ch_error_t

// .. c:function::
CH_EXPORT
void
ch_chirp_config_init(ch_config_t* config);
//
//    Initialize chirp configuration with defaults.
//
//    :param ch_config_t* config: Pointer to a chirp configuration.

// .. c:function::
CH_EXPORT
ch_identity_t
ch_chirp_get_identity(ch_chirp_t* chirp);
//
//    Get the identity of the given chirp instance. Is thread-safe after chirp
//    has been initialized.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//
//    :return: a chirp identity. See: :c:type:`ch_identity_t`.
//    :rtype:  ch_identity_t

// .. c:function::
CH_EXPORT
uv_loop_t*
ch_chirp_get_loop(ch_chirp_t* chirp);
//
//    Get the loop of the given chirp object. Is thread-safe after chirp has
//    been initialized.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//
//    :return: a pointer to a libuv event loop object.
//    :rtype:  uv_loop_t*

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_init(
        ch_chirp_t*        chirp,
        const ch_config_t* config,
        uv_loop_t*         loop,
        ch_recv_cb_t       recv_cb,
        ch_start_cb_t      start_cb,
        ch_done_cb_t       done_cb,
        ch_log_cb_t        log_cb);
//
//    Initialiaze a chirp object. Memory is provided by the caller. You must
//    call :c:func:`ch_chirp_close_ts` to cleanup the chirp object.
//
//    You can free **chirp**, **config** and **loop** either after the **done**
//    callback has been called or if chirp is set to auto-stop, after the loop
//    has finished.
//
//    For any other return value than CH_ENOMEM, please await the done_cb
//    before freeing chirp. Config can be freed after ch_chirp_init returns.
//
//    Please call `ch_loop_close(loop)` before freeing the loop. Of course if
//    the loop will continue to run, feel free not to close/free the loop.
//
//    :param ch_chirp_t* chirp: Out: Pointer to a chirp object.
//    :param ch_config_t* config: Pointer to a chirp configration.
//    :param uv_loop_t* loop: Reference to a libuv loop.
//    :param ch_recv_cb_t recv_cb: Called when chirp receives a message.
//    :param ch_start_cb_t start_cb: Called when chirp is started, can be NULL.
//    :param ch_done_cb_t done_cb: Called when chirp is finished, can be NULL.
//    :param ch_log_cb_t log_cb: Callback to the logging facility, can be NULL.
//
//    :return: A chirp error. See: :c:type:`ch_error_t`.
//    :rtype: ch_error_t
//

// .. c:function::
CH_EXPORT
void
ch_chirp_release_msg_slot(
        ch_chirp_t* rchirp, ch_message_t* msg, ch_release_cb_t release_cb);
//
//    Release the internal message-slot and acknowledge the message if the
//    remote requested an acknowledge-message. Must be called when the message
//    isn't needed anymore, afterwards the message may NOT be used anymore. The
//    message may belong to another chirp instance.
//
//    IMPORTANT: Neglecting to release the slot will lockup chirp. Never ever
//    change a messages identity.
//
//    Before closing chirp you should await release_cb otherwise the remote
//    might not get the requested acknowledge-message.
//
//    :param ch_chirp_t* rchirp: Chirp instances for release_cb
//    :param ch_message_t* msg: The message representing the slot.
//    :param ch_release_cb_t release_cb: Called once the message is released.
//

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_release_msg_slot_ts(
        ch_chirp_t* rchirp, ch_message_t* msg, ch_release_cb_t release_cb);
//
//    Release the internal message-slot and acknowledge the message if the
//    remote requested an acknowledge-message. Must be called when the message
//    isn't needed anymore, afterwards the message may NOT be used anymore. The
//    message may belong to another chirp instance.
//
//    IMPORTANT: Neglecting to release the slot will lockup chirp. Never ever
//    change a messages identity.
//
//    Before closing chirp you should await release_cb otherwise the remote
//    might not get the requested acknowledge-message.
//
//    This function is thread-safe.
//
//    :param ch_chirp_t* rchirp: Chirp instances for release_cb
//    :param ch_message_t* msg: The message representing the slot.
//    :param ch_release_cb_t release_cb: Called once the message is released.
//

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_run(
        const ch_config_t* config,
        ch_chirp_t**       chirp,
        ch_recv_cb_t       recv_cb,
        ch_start_cb_t      start_cb,
        ch_done_cb_t       done_cb,
        ch_log_cb_t        log_cb);
//
//    Initializes, runs and cleans everything. Everything being:
//
//      * chirp object
//      * uv-loop
//      * uv-sockets
//      * callbacks
//
//     The method blocks, but chirp paramenter will be set. Can be used to run
//     chirp in a user defined thread. Use :c:func:`ch_chirp_close_ts` to close
//     chirp in any other thread.
//
//    :param ch_config_t* config: Pointer to a chirp configuration.
//    :param ch_chirp_t** chirp: Out: Pointer to a chirp object pointer. Can be
//                               NULL.
//
//    :param ch_recv_cb_t recv_cb: Called when chirp receives a message,
//                                 can be NULL.
//    :param ch_start_cb_t start_cb: Called when chirp is started, can be NULL.
//    :param ch_done_cb_t done_cb: Called when chirp is finished, can be NULL.
//    :param ch_log_cb_t log_cb: Callback to the logging facility, can be NULL.
//    :return: A chirp error. See: :c:type:`ch_error_t`.
//    :rtype: ch_error_t

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_send(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb);
//
//    Send a message. Messages can be sent concurrently to different nodes.
//    Messages to the same remote node will be queued if you don't wait for the
//    callback.
//
//    If you don't want to allocate messages on sending, we recommend to use a
//    pool of messages.
//
//    Returns CH_SUCCESS when has being sent and CH_QUEUED when the message has
//    been placed in the send queue. CH_USED if the message is already used
//    elsewhere, the message will not be sent.
//
//    The send message queue is not bounded, the user has to pause sending.
//
//    Sending only one message at the same time while having multiple peers
//    would prevent concurrency, but since chirp is quite fast, it is a valid
//    solution.
//
//    A simple pattern is counting the open :c:type:`ch_send_cb_t` and stop
//    sending at a certain threshold.
//
//    The optimal pattern would be sending a message at the time per remote. A
//    remote is defined by the (ip_protocol, address, port) tuple. This is
//    more complex to implement.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//    :param ch_message_t* msg: The message to send. The memory of the message
//                              must stay valid until the callback is called.
//    :param ch_send_cb_t send_cb: The callback, that will be called after
//                                 sending.

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_send_ts(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb);
//
//    Send a message. Messages can be sent in concurrently to different nodes.
//    Messages to the same remote node will be queued if you don't wait for the
//    callback.
//
//    See :c:func:`ch_chirp_send`
//
//    This function is thread-safe. ATTENTION: Callback will be called by the
//    uv-loop-thread.
//
//    Returns CH_SUCCESS when the message has been successfully queue and
//    CH_USED if the message is already used elsewhere, the message will not be
//    sent. This differs from ch_chirp_send, since the message is always
//    queued to be passed to the uv-loop-thread.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//    :param ch_message_t msg: The message to send. The memory of the message
//                             must stay valid until the callback is called.
//    :param ch_send_cb_t send_cb: The callback, that will be called after
//                                 sending.

// .. c:function::
CH_EXPORT
void
ch_chirp_set_always_encrypt(void);
//
//    Also encrypt local connections. This is set globally to all chirp
//    instances.

// .. c:function::
CH_EXPORT
void
ch_chirp_set_auto_stop_loop(ch_chirp_t* chirp);
//
//    Tells chirp to stop the uv-loop when closing (by setting the
//    corresponding flag).
//
//    After this function is called, :c:func:`ch_chirp_close_ts` will also stop
//    the loop.
//
//    This function is thread-safe.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.

// .. c:function::
CH_EXPORT
void
ch_chirp_set_log_callback(ch_chirp_t* chirp, ch_log_cb_t log_cb);
//
//    Set a callback for sending log messages.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//    :param ch_log_cb_t   log: Callback to be called when logging messages.

// .. c:function::
CH_EXPORT
void
ch_chirp_set_public_port(ch_chirp_t* chirp, uint16_t port);
//
//    Set a different public port. Used if your are behind a firewall/NAT that
//    will change public port from the actual port.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//    :param uint16_t port: New public port

// .. c:function::
CH_EXPORT
void
ch_chirp_set_recv_callback(ch_chirp_t* chirp, ch_recv_cb_t recv_cb);
//
//    Set a callback for receiving a message.
//
//    :param ch_chirp_t* chirp: Pointer to a chirp object.
//    :param ch_recv_cb_t recv_cb: Called when chirp receives a message,
//                                 can be NULL.
//
//
//    .. code-block:: cpp

#endif // ch_libchirp_chirp_h
// ==========
// Encryption
// ==========
//
// Interface for manual or fine-grained encryption setup: Only use if you know
// what you are doing. All functions become no-ops if CH_WITHOUT_TLS is
// defined.
//
// .. code-block:: cpp

#ifndef ch_libchirp_encryption_h
#define ch_libchirp_encryption_h

// Declarations
// ============

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_tls_init(void);
//
//    Initialize LibreSSL or OpenSSL according to configuration. Is a no-op if
//    CH_WITHOUT_TLS is defined.
//
//   :return: A chirp error. see: :c:type:`ch_error_t`
//   :rtype:  ch_error_t

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_tls_cleanup(void);
//
//    Cleanup LibreSSL or OpenSSL. Is a no-op if CH_WITHOUT_TLS is defined.
//
//   :return: A chirp error. see: :c:type:`ch_error_t`
//   :rtype:  ch_error_t

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_tls_threading_cleanup(void);
//
//    DO NOT USE, unless you really really know what you are doing. Provided
//    for the rare case where your host application initializes libressl or
//    openssl without threading support, but you need threading. Chirp usually
//    doesn't need threading. Is a no-op if CH_WITHOUT_TLS is defined.
//
//    Cleanup libressl or openssl threading by setting destroying the locks and
//    freeing memory.
//

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_tls_threading_setup(void);
//
//    DO NOT USE, unless you really really know what you are doing. Provided
//    for the rare case where your host application initializes openssl without
//    threading support, but you need threading. Chirp usually doesn't need
//    threading. Is a no-op if CH_WITHOUT_TLS is defined.
//
//    Setup openssl threading by initializing the required locks and setting
//    the lock and the thread_id callbacks.
//

// .. c:function::
CH_EXPORT
void
ch_en_set_manual_tls_init(void);
//
//    Manually initialize LibreSSL or OpenSSL. Is a no-op if CH_WITHOUT_TLS is
//    set.
//
//    By default chirp will initialize libressl or openssl on the first
//    instance of chirp and cleanup libressl or openssl on the last instance of
//    chirp. If for some reason the count of chirp instances can drop to zero
//    sometimes and you do not want libressl or openssl to get uninitialized
//    you can manually call :c:func:`ch_en_tls_init` before creating the first
//    chirp instance and :c:func:`ch_en_tls_cleanup` after closing the last
//    chirp instance.
//
//    You can also initialize libress or openssl yourself if you have to or not
//    initialize it at all if your host application has already initialized
//    libressl or openssl.
//

#endif // ch_libchirp_encryption_h
// ========
// Libchirp
// ========
//
// .. code-block:: cpp

#ifndef ch_libchirp_h
#define ch_libchirp_h

/* #include "libchirp/chirp.h" */
/* #include "libchirp/encryption.h" */
/* #include "libchirp/message.h" */

// .. c:var:: extern char* ch_version
//
//    Version of chirp.
//
// .. code-block:: cpp
//
CH_EXPORT
extern char* ch_version;

// .. c:function::
CH_EXPORT
ch_error_t
ch_libchirp_cleanup(void);
//
//    Cleanup the global libchirp structures, including encryption/libssl
//

// .. c:function::
CH_EXPORT
ch_error_t
ch_libchirp_init(void);
//
//    Initialize the global libchirp structures, including encryption/libssl
//

// .. c:function::
CH_EXPORT
void
ch_set_alloc_funcs(
        ch_alloc_cb_t alloc, ch_realloc_cb_t realloc, ch_free_cb_t free);
//
//    Set allocation functions.
//
//    .. note::
//
//       The user can change the functions multiple times. The user has to
//       ensure consistency of allocation/free pairs.
//
//    :param ch_alloc_cb_t alloc:     Memory allocation callback. Has the same
//                                    signature as malloc
//    :param ch_realloc_cb_t realloc: Memory reallocation callback. Has the same
//                                    signature as realloc
//    :param ch_free_cb_t free:       Callback to free memory. Has the same
//                                    signature as free
//
// .. code-block:: cpp
//

#endif // ch_libchirp_h
