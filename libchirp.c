/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// ================================
// libchirp 1.0.0-beta amalgamation
// ================================

#include "libchirp.h"
#include "libchirp-config.h"

// =============
// Common header
// =============
//
// Defines global forward declarations and utility macros.
//
// .. code-block:: cpp
//
#ifndef ch_common_h
#define ch_common_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "libchirp-config.h" */
/* #include "libchirp/common.h" */
/* #include "util.h" */

// System includes
// ===============
//
// .. code-block:: cpp
//

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
// =====================
//
// .. code-block:: cpp

struct ch_remote_s;
typedef struct ch_remote_s ch_remote_t;
struct ch_connection_s;
typedef struct ch_connection_s ch_connection_t;
struct ch_protocol_s;
typedef struct ch_protocol_s ch_protocol_t;

//
// .. _double-eval:
//
// Double evaluation
// =================
//
// In order to get proper file and line number we have to implement validation
// and assertion using macros. But macros are prone to double evaluation, if you
// pass function having side-effects. Passing a pure function or data is valid.
//
// So ONLY pass variables to the V and A macros!
//
// But what is double evaluation?
//     The macro will just copy the function call, passed to it. So it will
//     be evaluated inside the macro, which people will often forget, the
//     code will behave in unexpected ways. You might think that this is
//     obvious, but forgetting it so easy. Humans have no problem writing
//     something like this and be amazed that it doesn't work.
//
// .. code-block:: cpp
//
//    int i = generate_int();
//    /* A few lines in between, so it isn't that obvious */
//    V(chirp, generate_int() == 0, "The first int generated isn't zero");
//
// Which should look like this.
//
// .. code-block:: cpp
//
//    int i = generate_int();
//    V(chirp, i == 0, "The first int generated isn't zero");
//
// Definitions
// ===========

// .. c:type:: ch_chirp_uninit_t
//
//    Used to uninitialize on ch_chirp_init failure.
//
// .. code-block:: cpp
//
typedef enum {
    CH_UNINIT_INIT_LOCK     = 1 << 0,
    CH_UNINIT_ICHIRP        = 1 << 1,
    CH_UNINIT_ASYNC_CLOSE   = 1 << 2,
    CH_UNINIT_ASYNC_DONE    = 1 << 3,
    CH_UNINIT_ASYNC_START   = 1 << 4,
    CH_UNINIT_ASYNC_SEND_TS = 1 << 5,
    CH_UNINIT_SEND_TS_LOCK  = 1 << 6,
    CH_UNINIT_ASYNC_RELE_TS = 1 << 7,
    CH_UNINIT_RELE_TS_LOCK  = 1 << 8,
    CH_UNINIT_SERVERV4      = 1 << 9,
    CH_UNINIT_SERVERV6      = 1 << 10,
    CH_UNINIT_TIMER_GC      = 1 << 11,
    CH_UNINIT_TIMER_RECON   = 1 << 12,
    CH_UNINIT_SIGNAL        = 1 << 13,
} ch_chirp_uninit_t;


// .. c:macro:: EC
//
//    Reports an error.
//
//    The error macro E(chirp, message, ...) behaves like printf and allows to
//    log to a custom callback. Usually used to log into pythons logging
//    facility. On the callback it sets the argument error to true and it will
//    log to stderr if no callback is set.
//
//    :param chirp: Pointer to a chirp object.
//    :param message: The highlighted message to report.
//    :param clear:   The clear message to report.
//    :param ...: Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#define EC(chirp, message, clear, ...)                                         \
    ch_write_log(chirp, __FILE__, __LINE__, message, clear, 1, __VA_ARGS__);
#define E(chirp, message, ...) EC(chirp, message, "", __VA_ARGS__)

// .. c:macro:: V
//
//    Validates the given condition and reports a message including arbitrary
//    given arguments when the condition is not met in debug-/development-mode.
//
//    Be aware of :ref:`double-eval`
//
//    The validate macro V(chirp, condition, message, ...) behaves like printf
//    and allows to print a message given an assertion. If that assertion is
//    not fullfilled, it will print the given message and return
//    :c:member:`ch_error_t.CH_VALUE_ERROR`, even in release mode.
//
//    :param chirp: Pointer to a chirp object.
//    :param condition: A boolean condition to check.
//    :param message: Message to print when the condition is not met.
//    :param ...: Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#define V(chirp, condition, message, ...)                                      \
    if (!(condition)) {                                                        \
        ch_write_log(chirp, __FILE__, __LINE__, message, "", 1, __VA_ARGS__);  \
        return CH_VALUE_ERROR;                                                 \
    }

#ifdef CH_ENABLE_LOGGING

// .. c:macro:: LC
//
//    Logs the given message including arbitrary arguments to a custom callback
//    in debug-/development-mode.
//
//    The logging macro L(chirp, message, ...) behaves like printf
//    and allows to log to a custom callback. Usually used to log into pythons
//    logging facility.
//
//    The logging macro LC(chirp, message, clear, ...) behaves like
//    L except you can add part that isn't highlighted.
//
//    :param chirp:   Pointer to a chirp object.
//    :param message: The highlighted message to report.
//    :param clear:   The clear message (not highlighted) to report.
//    :param ...:     Variadic arguments for xprintf
//
// .. code-block:: cpp
//
#define LC(chirp, message, clear, ...)                                         \
    CH_WRITE_LOGC(chirp, message, clear, __VA_ARGS__)
#define L(chirp, message, ...) CH_WRITE_LOG(chirp, message, __VA_ARGS__)

#else // CH_ENABLE_LOGGING

// .. c:macro:: L
//
//    See :c:macro:`L`. Does nothing in release-mode.
// .. code-block:: cpp
//
#define L(chirp, message, ...)                                                 \
    (void) (chirp);                                                            \
    (void) (message)
#define LC(chirp, message, clear, ...)                                         \
    (void) (chirp);                                                            \
    (void) (message);                                                          \
    (void) (clear)
#endif

#ifdef CH_ENABLE_ASSERTS

// .. c:macro:: A
//
//    Validates the given condition and reports arbitrary arguments when the
//    condition is not met in debug-/development-mode.
//
//    Be aware of :ref:`double-eval`
//
//    :param condition: A boolean condition to check.
//    :param message:   Message to display
//
// .. code-block:: cpp
//
#define A(condition, message)                                                  \
    if (!(condition)) {                                                        \
        fprintf(stderr,                                                        \
                "%s:%d: Assert failed: %s\n",                                  \
                __FILE__,                                                      \
                __LINE__,                                                      \
                message);                                                      \
        exit(1);                                                               \
    }

// .. c:macro:: AP
//
//    Validates the given condition and reports arbitrary arguments when the
//    condition is not met in debug-/development-mode.
//
//    Be aware of :ref:`double-eval`
//
//    The assert macro AP(condition, ...) behaves like printf and allows to
//    print a arbitrary arguments when the given assertion fails.
//
//    :param condition: A boolean condition to check.
//    :param message:   Message to display
//
// .. code-block:: cpp
//
#define AP(condition, message, ...)                                            \
    if (!(condition)) {                                                        \
        fprintf(stderr, "%s:%d: Assert failed: ", __FILE__, __LINE__);         \
        fprintf(stderr, message "\n", __VA_ARGS__);                            \
        exit(1);                                                               \
    }

// .. c:macro:: ch_chirp_check_m()
//
//    Validates that we have a valid chirp object and we are on the right
//    thread.
//
// .. code-block:: cpp

#define ch_chirp_check_m(chirp)                                                \
    {                                                                          \
        A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");                \
        uv_thread_t __ch_chirp_check_self_ = uv_thread_self();                 \
        A(uv_thread_equal(&__ch_chirp_check_self_, &chirp->_thread) != 0,      \
          "Call on the wrong thread");                                         \
    }

#else // CH_ENABLE_ASSERTS

//.. c:macro:: A
//
//    See :c:macro:`A`. Does nothing in release-mode.
//
//    Be aware of :ref:`double-eval`
//
// .. code-block:: cpp
//
#define A(condition, message) (void) 0

//.. c:macro:: AP
//
//    See :c:macro:`AP`. Does nothing in release-mode.
//
//    Be aware of :ref:`double-eval`
//
// .. code-block:: cpp
//
#define AP(condition, message, ...) (void) 0

#define ch_chirp_check_m(chirp) (void) (chirp)

#endif

#ifndef A
#error Assert macro not defined
#endif
#ifndef AP
#error Assert print macro not defined
#endif
#ifndef ch_chirp_check_m
#error ch_chirp_check_m macro not defined
#endif
#ifndef L
#error Log macro L not defined
#endif
#ifndef LC
#error Log macro LC not defined
#endif
#ifndef V
#error Validate macro not defined
#endif

#define CH_CHIRP_MAGIC 42429
/* Used cheat clang format */
#define CH_ALLOW_NL static void _ch_noop_func__(void)

// Generic functions
// =================
//
// .. code-block:: text
//
#define MINMAX_FUNCS(type)                                                     \
    static type ch_max_##type(type a, type b) { return (a > b) ? a : b; }      \
    static type ch_min_##type(type a, type b) { return (a < b) ? a : b; }

#endif // ch_common_h
// =================
// Queue / Stack 0.7
// =================
//
// Queue and stack with a rbtree-style interface. Both queue and stack use one
// next pointer and can be described as linked lists, except that the queue is
// actually an ring.
//
// Installation
// ============
//
// Copy qs.h into your source.
//
// Development
// ===========
//
// See `README.rst`_
//
// .. _`README.rst`: https://github.com/concretecloud/rbtree
//
// API
// ===
//
// Queue
// -----
//
// qs_queue_bind_decl_m(context, type) alias rb_bind_decl_cx_m
//    Bind the qs_queue function declarations for *type* to *context*. Usually
//    used in a header.
//
// qs_queue_bind_impl_m(context, type)
//    Bind the qs_queue function implementations for *type* to *context*.
//    Usually used in a c-file. This variant uses the standard rb_*_m traits.
//
// qs_queue_bind_impl_cx_m(context, type)
//    Bind the qs_queue function implementations for *type* to *context*.
//    Usually used in a c-file. This variant uses cx##_*_m traits, which means
//    you have to define them.
//
// qs_queue_bind_cx_m/qs_queue_bind_m(context, type)
//    Shortcut to bind declaration and implementation at once.
//
// Then the following functions will be available.
//
// cx##_enqueue(type** queue, type* item)
//    Enqueue an item to the queue.
//
// cx##_dequeue(type** queue, type** item)
//    Dequeue an item from the queue. Queue will be set to NULL when empty.
//
// cx##_head(type* queue, type* item)
//    *item* will be set to the item in front of the queue, which equals the
//    oldest item or the item that is going to be dequeud. Is NULL when the
//    queue is empty.
//
// cx##_head(type* queue, type* item)
//    *item* will be set to the item in the back of the queue, which equals the
//    newest item. Is NULL when the queue is empty.
//
// cx##_iter_init(type* tree, cx##_iter_t* iter, type** elem)
//    Initializes *elem* to point to the first element in the queue. Use
//    qs_queue_iter_decl_m to declare *iter* and *elem*. If the queue is empty
//    *elem* will be a NULL-pointer.
//
// cx##_iter_next(cx##_iter_t* iter, type** elem)
//    Move *elem* to the next element in the queue. *elem* will point to
//    NULL at the end.
//
// cx##_top(type* stack, type* item)
//    *item* will be set to the item on top of the stack.
//
// You can use rb_for_m from rbtree.h with qs.
//
// Stack
// -----
//
// qs_stack_bind_decl_m(context, type) alias rb_bind_decl_cx_m
//    Bind the qs_stack function declarations for *type* to *context*. Usually
//    used in a header.
//
// qs_stack_bind_impl_m(context, type)
//    Bind the qs_stack function implementations for *type* to *context*.
//    Usually used in a c-file. This variant uses the standard rb_*_m traits.
//
// qs_stack_bind_impl_cx_m(context, type)
//    Bind the qs_stack function implementations for *type* to *context*.
//    Usually used in a c-file. This variant uses cx##_*_m traits, which means
//    you have to define them.
//
// qs_stack_bind_cx_m/qs_stack_bind_m(context, type)
//    Shortcut to bind declaration and implementation at once.
//
// Then the following functions will be available.
//
// cx##_push(type** stack, type* item)
//    Push an item to the stack.
//
// cx##_pop(type** stack, type** item)
//    Pop an item from the stack. Stack will be set to NULL when empty.
//
// cx##_iter_init(type* tree, cx##_iter_t* iter, type** elem)
//    Initializes *elem* to point to the first element in the stack. Use
//    qs_stack_iter_decl_m to declare *iter* and *elem*. If the stack is empty
//    *elem* will be a NULL-pointer.
//
// cx##_iter_next(cx##_iter_t* iter, type** elem)
//    Move *elem* to the next element in the stack. *elem* will point to
//    NULL at the end.
//
// You can use rb_for_m from rbtree.h with qs.
//
// Implementation
// ==============
//
// Includes
// --------
//
// .. code-block:: cpp
//
#ifndef qs_stack_queue_h
#define qs_stack_queue_h
#include <assert.h>

// Traits
// ------
//
// .. code-block:: cpp
//
#define qs_next_m(x) (x)->next

// Queue
// -----
//
// Common arguments
//
// next
//    Get the next item of the queue
//
//
// .. code-block:: text
//
//    .---.        .---.
//    | 2 |<-next--| 1 |
//    '---'        '---'
//      |next        ^
//      v        next|
//    .---.        .---.
//    | 3 |--next->| 4 |<--queue--
//    '---'        '---'
//
// qs_enqueue_m
// ------------
//
// Bound: cx##_enqueue
//
// Enqueues an item to the queue.
//
// queue
//    Beginning of the queue
//
// item
//    Item to enqueue.
//
// .. code-block:: cpp
//
#define qs_enqueue_m( \
        next, \
        queue, \
        item \
) \
{ \
    assert(next(item) == NULL && "Item already in use"); \
    if(queue == NULL) { \
        next(item) = item; \
    } else { \
        next(item) = next(queue); \
        next(queue) = item; \
    } \
    queue = item; \
} \


// qs_dequeue_m
// ------------
//
// Bound: cx##_dequeue
//
// Dequeue an item from the queue. Returns the first item in the queue (FIFO).
// Does nothing if the queue is empty.
//
// queue
//    Beginning of the queue
//
// item
//    Item dequeued.
//
// .. code-block:: cpp
//
#define qs_dequeue_m( \
        next, \
        queue, \
        item \
) \
{ \
    if(queue != NULL) { \
        item = next(queue); \
        if(next(queue) == queue) { \
            queue = NULL; \
        } else { \
            next(queue) = next(item); \
        } \
        next(item) = NULL; \
    } else { \
        item = NULL; \
    } \
} \


// qs_queue_bind_decl_m
// --------------------
//
// Alias: qs_queue_bind_decl_cx_m
//
// Bind queue functions to a context. This only generates declarations.
//
// cx
//    Name of the new context.
//
// type
//    The type of the items of the queue.
//
// .. code-block:: cpp
//
#define qs_queue_bind_decl_m(cx, type) \
    typedef type cx##_iter_t; \
    typedef type cx##_type_t; \
    void \
    cx##_enqueue( \
            type** queue, \
            type* item \
    ); \
    void \
    cx##_dequeue( \
            type** queue, \
            type** item \
    ); \
    void \
    cx##_iter_init( \
            type* queue, \
            cx##_iter_t** iter, \
            type** elem \
    ); \
    void \
    cx##_iter_next( \
            cx##_iter_t* iter, \
            type** elem \
    ); \
    void \
    cx##_head( \
            type* queue, \
            type** item \
    ); \
    void \
    cx##_tail( \
            type* queue, \
            type** item \
    ); \


#define qs_queue_bind_decl_cx_m(cx, type) \
    qs_queue_bind_decl_m(cx, type) \


// qs_queue_bind_impl_m
// ---------------------
//
// Bind queue functions to a context. This only generates implementations.
//
// qs_queue_bind_impl_m uses qs_next_m. qs_queue_bind_impl_cx_m uses
// cx##_next_m.
//
// cx
//    Name of the new context.
//
// type
//    The type of the items of the queue.
//
// .. code-block:: cpp
//
#define _qs_queue_bind_impl_tr_m(cx, type, next) \
    void \
    cx##_enqueue( \
            type** queue, \
            type* item \
    ) qs_enqueue_m( \
            next, \
            *queue, \
            item \
    ) \
    void \
    cx##_dequeue( \
            type** queue, \
            type** item \
    ) qs_dequeue_m( \
            next, \
            *queue, \
            *item \
    ) \
    void \
    cx##_iter_init( \
            type* queue, \
            cx##_iter_t** iter, \
            type** elem \
    ) \
    { \
        qs_queue_iter_init_m( \
            next, \
            queue, \
            *iter, \
            *elem \
        ); \
    } \
    void \
    cx##_iter_next( \
            cx##_iter_t* iter, \
            type** elem \
    ) \
    { \
        qs_queue_iter_next_m( \
            next, \
            iter, \
            *elem \
        ) \
    } \
    void \
    cx##_head( \
            type* queue, \
            type** item \
    ) { \
        if(queue != NULL) { \
            *item = next(queue); \
        } else { \
            *item = NULL; \
        } \
    } \
    void \
    cx##_tail( \
            type* queue, \
            type** item \
    ) { \
        *item = queue; \
    } \


#define qs_queue_bind_impl_cx_m(cx, type) \
    _qs_queue_bind_impl_tr_m(cx, type, cx##_next_m) \


#define qs_queue_bind_impl_m(cx, type) \
    _qs_queue_bind_impl_tr_m(cx, type, qs_next_m) \


#define qs_queue_bind_cx_m(cx, type) \
    qs_queue_bind_decl_cx_m(cx, type) \
    qs_queue_bind_impl_cx_m(cx, type) \


#define qs_queue_bind_m(cx, type) \
    qs_queue_bind_decl_m(cx, type) \
    qs_queue_bind_impl_m(cx, type) \


// qs_queue_iter_decl_m
// ---------------------
//
// Also: qs_queue_iter_decl_cx_m
//
// Declare iterator variables.
//
// iter
//    The new iterator variable.
//
// elem
//    The pointer to the current element.
//
// .. code-block:: cpp
//
#define qs_queue_iter_decl_m(type, iter, elem) \
    type* iter = NULL; \
    type* elem = NULL; \


#define qs_queue_iter_decl_cx_m(cx, iter, elem) \
    cx##_type_t* iter = NULL; \
    cx##_type_t* elem = NULL; \


// qs_queue_iter_init_m
// ---------------------
//
// Bound: cx##_iter_init
//
// Initialize iterator. It will point to the first element or NULL if the queue
// is empty.
//
// queue
//    The queue.
//
// iter
//    The iterator.
//
// elem
//    The pointer to the current element.
//
//
// .. code-block:: cpp
//
#define qs_queue_iter_init_m(next, queue, iter, elem) \
{ \
    iter = queue; \
    if(queue == NULL) { \
        elem = NULL; \
    } else { \
        elem = next(queue); \
    } \
} \


// qs_queue_iter_next_m
// --------------------
//
// Bound: cx##_iter_next
//
// Initialize iterator. It will point to the first element. The element will be
// NULL, if the iteration is at the end.
//
// queue
//    The queue.
//
// elem
//    The pointer to the current element.
//
// .. code-block:: cpp
//
#define qs_queue_iter_next_m( \
        next, \
        queue, \
        elem \
) \
{ \
    if(elem == queue) { \
        elem = NULL; \
    } else { \
        elem = next(elem); \
    } \
} \



// Stack
// -----
//
// Common arguments
//
// next
//    Get the next item of the stack
//
// qs_push_m
// ---------
//
// Bound: cx##_push
//
// Push an item to the stack.
//
// stack
//    Base pointer to the stack.
//
// item
//    Item to push.
//
// .. code-block:: cpp
//
#define qs_push_m( \
        next, \
        stack, \
        item \
) \
{ \
    assert(next(item) == NULL && "Item already in use"); \
    next(item) = stack; \
    stack = item; \
} \


// qs_pop_m
// --------
//
// Bound: cx##_pop
//
// Pop an item from the stack. Returns the last item in the stack (LIFO).
// Does nothing if the stack is empty.
//
// stack
//    Base pointer to the stack.
//
// item
//    Item popped.
//
// .. code-block:: cpp
//
#define qs_pop_m( \
        next, \
        stack, \
        item \
) \
{ \
    item = stack; \
    if(stack != NULL) { \
        stack = next(stack); \
        next(item) = NULL; \
    } \
} \


// qs_stack_bind_decl_m
// --------------------
//
// Alias: qs_stack_bind_decl_cx_m
//
// Bind stack functions to a context. This only generates declarations.
//
// cx
//    Name of the new context.
//
// type
//    The type of the items of the stack.
//
// .. code-block:: cpp
//
#define qs_stack_bind_decl_m(cx, type) \
    typedef type cx##_iter_t; \
    typedef type cx##_type_t; \
    void \
    cx##_push( \
            type** stack, \
            type* item \
    ); \
    void \
    cx##_pop( \
            type** stack, \
            type** item \
    ); \
    void \
    cx##_iter_init( \
            type* stack, \
            cx##_iter_t** iter, \
            type** elem \
    ); \
    void \
    cx##_iter_next( \
            cx##_iter_t* iter, \
            type** elem \
    ); \
    void \
    cx##_top( \
            type* stack, \
            type** item \
    ); \


#define qs_stack_bind_decl_cx_m(cx, type) qs_stack_bind_decl_m(cx, type)

// qs_stack_bind_impl_m
// ---------------------
//
// Bind stack functions to a context. This only generates implementations.
//
// qs_stack_bind_impl_m uses qs_next_m. qs_stack_bind_impl_cx_m uses
// cx##_next_m.
//
// cx
//    Name of the new context.
//
// type
//    The type of the items of the stack.
//
// .. code-block:: cpp
//
#define _qs_stack_bind_impl_tr_m(cx, type, next) \
    void \
    cx##_push( \
            type** stack, \
            type* item \
    ) qs_push_m( \
            next, \
            *stack, \
            item \
    ) \
    void \
    cx##_pop( \
            type** stack, \
            type** item \
    ) qs_pop_m( \
            next, \
            *stack, \
            *item \
    ) \
    void \
    cx##_iter_init( \
            type* stack, \
            cx##_iter_t** iter, \
            type** elem \
    ) \
    { \
        (void)(iter); \
        qs_stack_iter_init_m( \
            next, \
            stack, \
            *elem \
        ); \
    } \
    void \
    cx##_iter_next( \
            cx##_iter_t* iter, \
            type** elem \
    ) \
    { \
        (void)(iter); \
        qs_stack_iter_next_m( \
            next, \
            *elem \
        ) \
    } \
    void \
    cx##_top( \
            type* stack, \
            type** item \
    ) { \
        *item = stack; \
    } \


#define qs_stack_bind_impl_cx_m(cx, type) \
    _qs_stack_bind_impl_tr_m(cx, type, cx##_next_m) \


#define qs_stack_bind_impl_m(cx, type) \
    _qs_stack_bind_impl_tr_m(cx, type, qs_next_m) \


#define qs_stack_bind_cx_m(cx, type) \
    qs_stack_bind_decl_cx_m(cx, type) \
    qs_stack_bind_impl_cx_m(cx, type) \


#define qs_stack_bind_m(cx, type) \
    qs_stack_bind_decl_m(cx, type) \
    qs_stack_bind_impl_m(cx, type) \


// qs_stack_iter_decl_m
// ---------------------
//
// Also: qs_stack_iter_decl_cx_m
//
// Declare iterator variables.
//
// iter
//    The new iterator variable.
//
// elem
//    The pointer to the current element.
//
// .. code-block:: cpp
//
#define qs_stack_iter_decl_m(type, iter, elem) \
    type* iter = NULL; \
    type* elem = NULL; \


#define qs_stack_iter_decl_cx_m(cx, iter, elem) \
    cx##_type_t* iter = NULL; \
    cx##_type_t* elem = NULL; \


// qs_stack_iter_init_m
// ---------------------
//
// Bound: cx##_iter_init
//
// Initialize iterator. It will point to the first element or NULL if the stack
// is empty.
//
// stack
//    Base pointer to the stack.
//
// elem
//    The pointer to the current element.
//
//
// .. code-block:: cpp
//
#define qs_stack_iter_init_m(next, stack, elem) \
{ \
    elem = stack; \
} \


// qs_stack_iter_next_m
// --------------------
//
// Bound: cx##_iter_next
//
// Initialize iterator. It will point to the first element. The element will be
// NULL, if the iteration is at the end.
//
// elem
//    The pointer to the current element.
//
// .. code-block:: cpp
//
#define qs_stack_iter_next_m( \
        next, \
        elem \
) \
{ \
    elem = next(elem); \
} \

#endif //qs_stack_queue_h

// MIT License
// ===========
//
// Copyright (c) 2017 Jean-Louis Fuchs
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// .. image:: https://travis-ci.org/concretecloud/rbtree.svg?branch=master
//    :target: https://travis-ci.org/concretecloud/rbtree/
//
// ==================
// Red-Black Tree 0.7
// ==================
//
// * Bonus: `qs.h`_ (Queue / Stack), mpipe_ (message-pack over pipe)
// * Textbook implementation
// * Extensive tests
// * Has parent pointers and therefore faster delete_node and constant time
//   replace_node
// * Composable
// * Readable
// * Generic
// * C99
// * Easy to use, a bit complex to extend because it is generic [1]_
// * MIT license
// * By Jean-Louis Fuchs <ganwell@fangorn.ch>
// * Based on Introduction To Algorithms
// * Text review by Eva Fuchs
// * Code review by Oliver Sauder @sliverc
// * Thanks a lot to both
//
// .. [1] My rgc preprocessor and make MACRO_DEBUG=True are very helpful.
//
// .. _`qs.h`: https://github.com/concretecloud/rbtree/blob/master/qs.rst
// .. _mpipe: https://github.com/concretecloud/rbtree/blob/master/mpipe.rst
//
// Installation
// ============
//
// Copy rbtree.h into your source.
//
// Changes
// =======
//
// 0.3 -> 0.4
// ----------
//
// * Correctly enable cppcheck
// * Fix style errors reported by cppcheck
// * Fix bad asserts in performance tests
//
// 0.4 -> 0.5
// ----------
//
// * Remove static from functions (its a bad habit)
// * Typos
//
// 0.5 -> 0.6
// ----------
//
// * Use same code-guidelines as libchirp
// * Allow msgpack or umsgpack in mpipe.py
//
// 0.6 -> 0.7
// ----------
//
// * Remove next from _qs_queue_bind_decl_tr_m and renaming it to
//   qs_queue_bind_decl_m
// * Document bind shortcuts for rb and qs
//
// Development
// ===========
//
// Developed on github_
//
// .. _github: https://github.com/concretecloud/rbtree
//
// Requirements
// ------------
//
// * gcc or clang
// * pytest
// * hypothesis
// * u-msgpack-python
// * cffi
// * rst2html
//
// Build and test
// --------------
//
// .. code-block:: bash
//
//    make
//
// Debug macros
// ------------
//
// .. code-block:: bash
//
//    make clean all MACRO_DEBUG=True
//
// This will expand the macros into .c files, so gdb can step into them.
//
// Usage
// =====
//
// Getting started
// ---------------
//
// The example is on github: example_h_, example_c_
//
// .. _example_h: https://github.com/concretecloud/rbtree/blob/master/src/example.h
// .. _example_c: https://github.com/concretecloud/rbtree/blob/master/src/example.c
//
// First we have to define the struct we use in the red-black tree
// (example.h).
//
// .. code-block:: cpp
//
//    struct book_s;
//    typedef struct book_s book_t;
//    struct book_s {
//        char    isbn[14];
//        char*   title;
//        char*   author;
//        char    color;
//        book_t* parent;
//        book_t* left;
//        book_t* right;
//    };
//
// You can add the fields color, parent, left and right to any existing struct.
// It is also possible to use different names than the above (see Extended_).
//
// Next we have to define the comparator function, since we want to lookup
// books using the ISBN-number, we compare it using memcmp.
//
// .. code-block:: cpp
//
//    #define bk_cmp_m(x, y) memcmp(x->isbn, y->isbn, 13)
//
// Since rbtree is implemented as macros, using uppercase for macros isn't a
// good convention, therefore we use the suffix _m to denote macros.
//
// The prefix bk\_ is used as convenience, see context_.
//
// Note if you do something like:
//
// .. code-block:: cpp
//
//    #define bk_cmp_m(x, y) (x->value - y->value)
//
// You may only use values from (MIN_INT / 4) - 1 to (MAX_INT / 4) since rbtree
// uses a int to store the result. To be safe write the comparator as:
//
// .. code-block:: cpp
//
//    #define rb_safe_cmp_m(x, y) (((x)>(y) ? 1 : ((x)<(y) ? -1 : 0)))
//    #define bk_cmp_m(x, y) rb_safe_cmp_m(x->value, y->value)
//
// rb_safe_cmp_m is provided by rbtree.
//
// .. _context:
//
// Then we have to declare all the rbtree functions. rbtree uses a concept, I
// call context, to find functions it needs. For example the rbtree functions
// look for a macro called $CONTEXT_cmp_m. I developed this concept to make
// functions composable without being too verbose.
//
// .. code-block:: cpp
//
//    rb_for_m(bk, tree, bk_iter, bk_elem)
//
// will look for the functions bk_iter_init and bk_iter_next.
//
// rb_bind_decl_m takes the context, bk in this case and the type as arguments.
//
// .. code-block:: cpp
//
//    rb_bind_decl_m(bk, book_t)
//
// Now we switch to example.c and define all the rbtree functions and the trees
// root node.
//
// .. code-block:: cpp
//
//    /* #include "example.h" */
//    rb_bind_impl_m(bk, book_t)
//    book_t* tree;
//
// In order to use the tree, we have to initialize it, which actually is
// assigning *bk_nil_ptr* to it.
//
// .. code-block:: cpp
//
//    bk_tree_init(&tree);
//
// Now we can register a book:
//
// .. code-block:: cpp
//
//    void
//    register_book(char isbn[14], char* title, char* author)
//    {
//        book_t* book = malloc(sizeof(book_t));
//        bk_node_init(book);
//        book->title  = title;
//        book->author = author;
//        memcpy(book->isbn, isbn, 14);
//        bk_insert(&tree, book);
//    }
//
// Note that we pass a double pointer to bk_insert, since it might need to change
// the root node.
//
// Or we can lookup a book:
//
// .. code-block:: cpp
//
//    void
//    lookup_book(char isbn[14])
//    {
//        book_t* book;
//        book_t key;
//        memcpy(key.isbn, isbn, 14);
//        bk_find(tree, &key, &book);
//        printf(
//            "ISBN:   %s\nTitle:  %s\nAuthor: %s\n\n",
//            book->isbn,
//            book->title,
//            book->author
//        );
//    }
//
// The *key* is just another node, we don't have to initialize it, but only set
// the fields used by the comparator. bk_find will set *book* to the node found.
//
// We can also iterate over the tree, the result will be sorted, lesser element
// first. The tree may not be modified during iteration.
//
// .. code-block:: cpp
//
//    rb_iter_decl_cx_m(bk, bk_iter, bk_elem);
//    rb_for_m(bk, tree, bk_iter, bk_elem) {
//        printf("%s\n", bk_elem->isbn);
//    }
//
// Removing a book is straight forward.
//
// .. code-block:: cpp
//
//    void
//    remove_book(book_t* book)
//    {
//        printf("Removing %s\n", book->isbn);
//        bk_delete_node(&tree, book);
//        free(book);
//    }
//
// But we cannot use the iterator. Therefore we just remove the root till the
// tree is empty.
//
// .. code-block:: cpp
//
//    while(tree != bk_nil_ptr) {
//        remove_book(tree);
//    }
//
// API
// ---
//
// rb_bind_decl_m(context, type) alias rb_bind_decl_cx_m
//    Bind the rbtree function declarations for *type* to *context*. Usually
//    used in a header.
//
// rb_bind_impl_m(context, type)
//    Bind the rbtree function implementations for *type* to *context*. Usually
//    used in a c-file. This variant uses the standard rb_*_m traits.
//
// rb_bind_impl_cx_m(context, type)
//    Bind the rbtree function implementations for *type* to *context*. Usually
//    used in a c-file. This variant uses cx##_*_m traits, which means you have
//    to define them.
//
// rb_bind_cx_m/rb_bind_m(context, type)
//    Shortcut to bind declaration and implementation at once.
//
// rb_safe_value_cmp_m(x, y)
//    Basis for safe value comparators. *x* and *y* are comparable values of
//    the same type.
//
// Then the following functions will be available.
//
// cx##_tree_init(type* tree)
//    Initialize *tree* by assigning *cx##_nil_ptr* to it.
//
// cx##_node_init(type* node)
//    Initialize *node* by initializing the color, parent, left and right fields.
//
// cx##_insert(type** tree, type* node)
//    Insert *node* into *tree*. If a node with the same key exists the
//    function returns 1 and *node* is not inserted, 0 on success.
//
// cx##_delete_node(type** tree, type* node)
//    Delete the known *node* from *tree*.
//
// cx##_delete(type** tree, type* key, type** node)
//    Delete the node matching *key* from *tree*. If *key* is not in the tree
//    the function returns 1, 0 on success. On success *node* is set to the
//    deleted node.
//
// cx##_replace_node(type** tree, type* old, type* new)
//    Replace known node *old* with *new*. If *old* and *new* are not equal the
//    function will not do anything and returns 1, 0 on success.
//
// cx##_replace(type** tree, type* key, type* new, type** old)
//    Replace the node matching *key* with *new*. If *key* and *new* are not
//    equal the function will not do anything and returns 1. If *key* is not in
//    the tree the function will not do anything and returns 1. It returns 0 on
//    success. On success *old* is set to the old node.
//
// cx##_find(type* tree, type* key, type** node)
//    Find the node matching *key* and assign it to *node*. If *key* is not in
//    the tree *node* will not be assigned and the function returns 1, 0 on
//    success.
//
// cx##_size(type* tree)
//    Returns the size of tree. By default RB_SIZE_T is int to avoid additional
//    dependencies. Feel free to define RB_SIZE_T as size_t for example. O(log
//    (N)).
//
// rb_iter_decl_m(cx, iter, elem)
//    Declares the variables *iter* and *elem* for the context *cx*.
//
// cx##_iter_init(type* tree, cx##_iter_t* iter, type** elem)
//    Initializes *elem* to point to the first element in tree. Use
//    rb_iter_decl_m to declare *iter* and *elem*. If the tree is empty
//    *elem* will be NULL.
//
// cx##_iter_next(cx##_iter_t* iter, type** elem)
//    Move *elem* to the next element in the tree. *elem* will point to
//    NULL at the end.
//
// cx##_check_tree(type* tree)
//    Check the consistency of a tree. Only interesting for development of
//    rbtree itself. If will fail with an assert if there is an inconsistency.
//
// Extended
// --------
//
// .. _Extended:
//
// Many functions x come in two flavors
//
// cx_x
//    These functions are bound to a type. Traits and the comparator are mapped
//    to the context. You have to define the type and the traits for the
//    context and then you bind the function.
//
//    .. code-block:: cpp
//
//       #define my_color_m(x) (x)->color
//       #define my_parent_m(x) (x)->parent
//       #define my_left_m(x) (x)->left
//       #define my_right_m(x) (x)->right
//       #define my_cmp_m(x, y) rb_safe_value_cmp_m(x, y)
//       rb_bind_cx_m(my, node_t)
//
//    .. code-block:: cpp
//
//       my_tree_init(&tree);
//       my_node_init(node);
//
//    There is also a shortcut if you know your are going to use all standard
//    fields in your struct (color, parent, left right)
//
//    .. code-block:: cpp
//
//       #define my_cmp_m(x, y) rb_safe_value_cmp_m(x, y)
//       rb_bind_m(my, node_t)
//
//    .. code-block:: cpp
//
//       my_tree_init(&tree);
//       my_node_init(node);
//
//    Of course usually, you want to split declaration and implementation of the
//    function, so it is: header.h:
//
//    .. code-block:: cpp
//
//       #define my_cmp_m(x, y) rb_safe_value_cmp_m(x, y)
//       rb_bind_decl_m(my, node_t)
//
//    And object.c:
//
//    .. code-block:: cpp
//
//       /* #include "header.h" */
//       rb_bind_impl_m(my, node_t)
//
//       int main(void) { my_node_init(node); return 0; }
//
// rb_x_m
//    These functions are macros and take a type and traits as standard
//    arguments and are the most verbose. Used to extend rbtree.
//
//    To use the rb_x_m functions you also need to initialize the nil pointer.
//
//    .. code-block:: cpp
//
//       tree = my_nil_ptr;
//       rb_node_init_m(
//           my_nil_ptr,
//           rb_color_m,
//           rb_parent_m,
//           rb_left_m,
//           rb_right_m,
//           my_nil_ptr
//       ); // Instead of my_tree_init in the bound functions
//
// Questions
// =========
//
// Why don't you just generate typed functions from the beginning?
//    I want to be able to reuse and compose my code. Especially for
//    composability I need access to the generic functions.
//
// Why is the iterator so complicated?
//    rbtree may become part of a larger set of data-structures, some need more
//    complicated iterator setups, to make the data-structures interchangeable,
//    all have to follow the iterator protocol. Use rb_for_m.
//
// Why yet another red-black tree?
//    I often joke that C programmers will reimplement every thing till it
//    perfectly fits their use-case/payload. I need the replace_node function
//    in my project. I found no way to avoid creating rbtree. sglib is the only
//    generic red-black tree implementation I know of and it has no parent
//    pointers, which makes replace_node impossible.
//
// Performance
// ===========
//
// I compare with sglib_, because it is the best and greatest I know. Kudos to
// Marian Vittek.
//
// .. _sglib: http://sglib.sourceforge.net/
//
// .. image:: https://github.com/concretecloud/rbtree/raw/master/perf_insert.png
//    :width: 90%
//    :align: center
//    :alt: insert
//
// .. image:: https://github.com/concretecloud/rbtree/raw/master/perf_delete.png
//    :width: 90%
//    :align: center
//    :alt: delete
//
// sglib has no delete_node. For many applications, a delete_node and a
// replace_node function is handy, since the application already has the right
// node to delete or replace.
//
// .. image:: https://github.com/concretecloud/rbtree/raw/master/perf_replace.png
//    :width: 90%
//    :align: center
//    :alt: replace
//
// Because we have parent pointer we can implement replace_node in constant
// time O(1). With sglib we have to add/remove for a replacement.
//
// Code size
// =========
//
// .. code-block:: text
//
//    0x018 T my_node_init
//    0x01b T my_tree_init
//    0x020 C my_nil_mem
//    0x02d T my_size
//    0x032 T my_iter_init
//    0x03d T my_find
//    0x042 T my_check_tree
//    0x043 T my_check_tree_rec
//    0x048 T my_iter_next
//    0x05d T my_replace
//    0x060 T my_delete
//    0x08b T my_replace_node
//    0x20e T my_insert
//    0x356 T my_delete_node
//
// About 2100 bytes. If NDEBUG or RB_NO_CHECK is defined the my_check_tree and
// my_check_tree_rec will be removed.
//
// Also _rb_rotate_left_m could be bound and called by delete and insert. But
// in my opinion 2100 bytes is small.
//
// Lessons learned
// ===============
//
// I thought I don't have to understand the red-black trees and could simply
// adjust an existing implementation. I chose poorly and the thing was
// inherently broken. I wasted a lot of time on it. They replaced the nil
// pointer with NULL and it resulted in a tree that works, but is not balanced.
// So my check_tree function failed and I tried to fix that implementation. It
// turns out bottom-up-fixups are very difficult to implement with NULL
// pointers. So after many hours wasted I just read Introductions to Algorithms
// and fixed my implementation.
//
// I thought I could adapt this code easily to make a persistent data-structure,
// but I found it is more important to have the parent pointers and therefore
// keep complexity at bay. If I am going to implement any persistent
// data-structures, I am going to build the persistent vector as used in closure
// and then convert the red-black tree to use vector-indexes and make it
// persistent on top of the persistent vector. It seems like the persistent
// vector can be built using reference-counting: pyrsistent_, so it should be
// possible.
//
// With the right mindset, generic and composable programming in C is awesome.
// Well, you need to expand the macros to debug. The following command will do
// that:
//
// .. code-block:: bash
//
//    $CC $CFLAGS -E -P infile.c | clang-format > outfile.c
//
// .. _pyrsistent: https://github.com/tobgu/pyrsistent/blob/master/pvectorcmodule.c
//
// Implementation
// ==============
//
// Based on Introduction to Algorithms: official_, wiki_, web_, pdf_ and
// archive_.
//
// .. _official: https://mitpress.mit.edu/books/introduction-algorithms
// .. _wiki: https://en.wikipedia.org/wiki/Introduction_to_Algorithms
// .. _web: http://staff.ustc.edu.cn/~csli/graduate/algorithms/book6/chap14.htm
// .. _pdf: http://www.realtechsupport.org/UB/SR/algorithms/Cormen_Algorithms_3rd.pdf
// .. _archive: https://archive.org/details/IntroductionToAlgorithms3edCorman_201508
//
// Properties
// ----------
//
// A binary search tree is a red-black tree if it satisfies the following
// red-black properties:
//
// 1. Every node is either red or black.
//
// 2. Every leaf (NIL) is black.
//
// 3. If a node is red, then both its children are black.
//
// 4. Every simple path from a node to a descendant leaf contains the same
//    number of black nodes.
//
// In order to understand the deletion, the concept of double (extra) blackness
// is introduced. If a black node was deleted its blackness is pushed down and a
// child can become extra black. This is the way property 1 can be violated.
//
// Definitions
// ===========
//
// RB_SIZE_T can be defined by the user to use size_t for example.
//
// .. code-block:: cpp
//
#ifndef rb_tree_h
#define rb_tree_h
#include <assert.h>
#ifndef RB_SIZE_T
#   define RB_SIZE_T int
#endif
#   define RB_NO_CHECK
//
// Basic traits
// ============
//
// Traits used by default (rb_x_m macros)
//
// .. code-block:: cpp
//
#define rb_color_m(x) (x)->color
#define rb_parent_m(x) (x)->parent
#define rb_left_m(x) (x)->left
#define rb_right_m(x) (x)->right
#define rb_value_m(x) (x)->value
//
// Context creation
// ================
//
// Create the type aliases. Actually only cx##_iter_t is used, since we can
// just refer to *type*. Note the const before cx##_nil_ptr, is the secret
// to make the code so small: the compiler just inserts the value into all
// comparisons with nil.
//
// .. code-block:: cpp
//
#define rb_new_context_m(cx, type) \
    typedef type cx##_type_t; \
    typedef type cx##_iter_t; \
    extern cx##_type_t* const cx##_nil_ptr; \


// Comparators
// ===========
//
// Some basic comparators, you would usually define your own.
//
// rb_safe_cmp_m
// ----------------
//
// Base for safe value comparators.
//
// x, y
//    Values to compare
//
// .. code-block:: cpp
//
#define rb_safe_cmp_m(x, y) \
    (((x)>(y) ? 1 : ((x)<(y) ? -1 : 0))) \

//
// rb_pointer_cmp_m
// ----------------
//
// Compares pointers.
//
// x, y
//    Nodes to compare
//
// .. code-block:: cpp
//
#define rb_pointer_cmp_m(x, y) \
    rb_safe_cmp_m(x, y) \


// rb_safe_value_cmp_m
// --------------------
//
// Safe value comparator. Compares nodes that have the rb_value_m trait.
//
// x, y
//    Nodes to compare
//
// .. code-block:: cpp
//
#define rb_safe_value_cmp_m(x, y) \
    rb_safe_cmp_m(rb_value_m(x), rb_value_m(y)) \


// rb_value_cmp_m
// ---------------
//
// Compares nodes that have the rb_value_m trait. Only safe if you only use
// 30bit values.
//
// x, y
//    Nodes to compare
//
// .. code-block:: cpp
//
#define rb_value_cmp_m(x, y) \
    (rb_value_m(x) - rb_value_m(y)) \


// Colors
// ======
//
// The obvious colors.
//
// .. code-block:: cpp
//
#define RB_BLACK 0
#define RB_RED   1

#define rb_is_black_m(x)   (x == RB_BLACK)
#define rb_is_red_m(x)     (x == RB_RED)

#define rb_make_black_m(x) x = RB_BLACK
#define rb_make_red_m(x)   x = RB_RED

// API
// ===
//
// Functions that are part of the API. The standard arguments are documented
// once:
//
// type
//    The type of the nodes in the red-black tree.
//
// nil
//    A pointer to the nil object.
//
// color
//    The color trait of the nodes in the rbtree.
//
// parent
//    The parent trait of the nodes in the rbtree is a pointer back to the
//    parent node.
//
// left
//    The left trait of the nodes in the rbtree is a pointer to the left branch
//    of the node.
//
// right
//    The right trait of the nodes in the rbtree is a pointer to the right
//    branch of the node.
//
// rb_node_init_m
// --------------
//
// Bound: cx##_node_init
//
// Initializes a node by setting the color to black and all pointers to nil.
//
// node
//    The node to initialize.
//
// .. code-block:: cpp
//
#define rb_node_init_m( \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        node \
) \
{ \
    color(node) = RB_BLACK; \
    parent(node) = nil; \
    left(node) = nil; \
    right(node) = nil; \
} \


// rb_for_m
// --------
//
// Generates a for-loop-header using the iterator.
//
// iter
//    The new iterator variable.
//
// elem
//    The pointer to the current element.
//
// .. code-block:: cpp
//
#define rb_for_m(cx, tree, iter, elem) \
    for( \
            cx##_iter_init(tree, &iter, &elem); \
            elem != NULL; \
            cx##_iter_next(iter, &elem) \
    ) \


// rb_iter_decl_m
// ---------------
//
// Also: rb_iter_decl_cx_m
//
// Declare iterator variables.
//
// iter
//    The new iterator variable.
//
// elem
//    The pointer to the current element.
//
// .. code-block:: cpp
//
#define rb_iter_decl_m(type, iter, elem) \
    type* iter = NULL; \
    type* elem = NULL; \


#define rb_iter_decl_cx_m(cx, iter, elem) \
    cx##_type_t* iter = NULL; \
    cx##_type_t* elem = NULL; \


// rb_iter_init_m
// --------------
//
// Bound: cx##_iter_init
//
// Initialize iterator. It will point to the first element.
//
// tree
//    The root node of the tree. A pointer to nil represents an empty tree.
//
// iter
//    The iterator.
//
// elem
//    The pointer to the current element. Is NULL if the tree is empty.
//
//
// .. code-block:: cpp
//
#define rb_iter_init_m(nil, left, tree, elem) \
{ \
    if(tree == nil) { \
        elem = NULL; \
    } else { \
        elem = tree; \
        while(left(elem) != nil) \
            elem = left(elem); \
    } \
    if(elem == nil) { \
        elem = NULL; \
    } \
} \


// rb_iter_next_m
// --------------
//
// Bound: cx##_iter_next
//
// Initialize iterator. It will point to the first element. The element will be
// NULL, if the iteration is at the end.
//
// elem
//    The pointer to the current element.
//
// .. code-block:: cpp
//
#define _rb_iter_next_m( \
    nil, \
    parent, \
    left, \
    right, \
    elem, \
    tmp \
) \
do { \
    tmp = right(elem); \
    if(tmp != nil) { \
        elem = tmp; \
        while(left(elem) != nil) \
            elem = left(elem); \
        break; \
    } \
    for(;;) { \
        /* Next would be the root, we are done. */ \
        if(parent(elem) == nil) { \
            elem = NULL; \
            break; \
        } \
        tmp = parent(elem); \
        /* tmp is a left node, therefore it is the next node. */ \
        if(elem == left(tmp)) { \
            elem = tmp; \
            break; \
        } \
        elem = tmp; \
    } \
} while(0) \


#define rb_iter_next_m( \
    nil, \
    type, \
    parent, \
    left, \
    right, \
    elem \
) \
{ \
    type* __rb_next_tmp_; \
    _rb_iter_next_m( \
        nil, \
        parent, \
        left, \
        right, \
        elem, \
        __rb_next_tmp_ \
    ); \
} \


// rb_insert_m
// ------------
//
// Bound: cx##_insert
//
// Insert the node into the tree. This function might replace the root node
// (*tree*). If an equal node exists in the tree, the node will not be added and
// will still be in its initialized state.
//
// The bound function will return 0 on success.
//
// cmp
//    Comparator (rb_pointer_cmp_m or rb_safe_value_cmp_m could be used)
//
// tree
//    The root node of the tree. A pointer to nil represents an empty tree.
//
// node
//    The node to insert.
//
// .. code-block:: cpp
//
#define _rb_insert_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        cmp, \
        tree, \
        node, \
        c, /* current */ \
        p, /* parent */ \
        r  /* result */ \
) \
do { \
    assert(tree != NULL && "Tree was not initialized"); \
    assert(node != nil && "Cannot insert nil node"); \
    assert(node != nil && "Cannot insert nil node"); \
    assert( \
        parent(node) == nil && \
        left(node) == nil && \
        right(node) == nil && \
        tree != node && \
        "Node already used or not initialized" \
    ); \
    if(tree == nil) { \
        tree = node; \
        rb_make_black_m(color(tree)); \
        break; \
    } else { \
        assert(( \
            parent(tree) == nil && \
            rb_is_black_m(color(tree)) \
        ) && "Tree is not root"); \
    } \
    c = tree; \
    p = NULL; \
    r = 0; \
    while(c != nil) { \
        /* The node is already in the rbtree, we break. */ \
        r = cmp((c), (node)); \
        if(r == 0) { \
            break; \
        } \
        p = c; \
        /* Lesser on the left, greater on the right. */ \
        c = r > 0 ? left(c) : right(c); \
    } \
    /* The node is already in the rbtree, we break. */ \
    if(c != nil) { \
        break; \
    } \
 \
    parent(node) = p; \
    rb_make_red_m(color(node)); \
 \
    if(r > 0) { \
        left(p) = node; \
    } else { \
        right(p) = node; \
    } \
 \
    _rb_insert_fix_m( \
            type, \
            nil, \
            color, \
            parent, \
            left, \
            right, \
            tree, \
            node \
    ); \
} while(0); \


#define rb_insert_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        cmp, \
        tree, \
        node \
) \
{ \
    type* __rb_ins_current_; \
    type* __rb_ins_parent_; \
    int   __rb_ins_result_; \
    _rb_insert_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        cmp, \
        tree, \
        node, \
        __rb_ins_current_, \
        __rb_ins_parent_, \
        __rb_ins_result_ \
    ) \
} \


// rb_delete_node_m
// ----------------
//
// Bound: cx##_delete_node
//
// Delete a node from the tree. This function acts on an actual tree
// node. If you don't have it; use rb_find_m first or rb_delete_m. The root node
// (*tree*) can change.
//
// tree
//    The root node of the tree. A pointer to nil represents an empty tree.
//
// node
//    The node to delete.
//
// .. code-block:: cpp
//
#define _rb_delete_node_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node, \
        x, \
        y \
) \
{ \
    assert(tree != NULL && "Tree was not initialized"); \
    assert(tree != nil && "Cannot remove node from empty tree"); \
    assert(node != nil && "Cannot delete nil node"); \
    assert(( \
        parent(node) != nil || \
        left(node) != nil || \
        right(node) != nil || \
        rb_is_black_m(color(node)) \
    ) && "Node is not in a tree"); \
    if(left(node) == nil || right(node) == nil) { \
        /* This node has at least one nil node, delete is simple. */ \
        y = node; \
    } else { \
        /* We need to find another node for deletion that has only one child. \
         * This is tree-next. */ \
        y = right(node); \
        while(left(y) != nil) { \
            y = left(y); \
        } \
    } \
 \
    /* If y has a child we have to attach it to the parent. */ \
    if(left(y) != nil) { \
        x = left(y); \
    } else { \
        x = right(y); \
    } \
 \
    /* Remove y from the tree. */ \
    parent(x) = parent(y); \
    if(parent(y) != nil) { \
        if(y == left(parent(y))) { \
            left(parent(y)) = x; \
        } else { \
            right(parent(y)) = x; \
        } \
    } else { \
        tree = x; \
    } \
 \
    /* A black node was removed, to fix the problem we pretend to have pushed the \
     * blackness onto x. Therefore x is double black and violates property 1. */ \
    if(rb_is_black_m(color(y))) { \
        _rb_delete_fix_m( \
                type, \
                nil, \
                color, \
                parent, \
                left, \
                right, \
                tree, \
                x \
        ); \
    } \
 \
    /* Replace y with the node since we don't control memory. */ \
    if(node != y) { \
        if(parent(node) == nil) { \
            tree = y; \
            parent(y) = nil; \
        } else { \
            if(node == left(parent(node))) { \
                left(parent(node)) = y; \
            } else if(node == right(parent(node))) { \
                right(parent(node)) = y; \
            } \
        } \
        if(left(node) != nil) { \
            parent(left(node)) = y; \
        } \
        if(right(node) != nil) { \
            parent(right(node)) = y; \
        } \
        parent(y) = parent(node); \
        left(y) = left(node); \
        right(y) = right(node); \
        color(y) = color(node); \
    } \
    /* Clear the node. */ \
    parent(node) = nil; \
    left(node) = nil; \
    right(node) = nil; \
    color(node) = RB_BLACK; \
} \


#define rb_delete_node_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node \
) \
{ \
    type* __rb_del_x_; \
    type* __rb_del_y_; \
    _rb_delete_node_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node, \
        __rb_del_x_, \
        __rb_del_y_ \
    ) \
} \


// rb_find_m
// ---------
//
// Bound: cx##_find
//
// Find a node using another node as key. The node will be set to nil if the
// key was not found.
//
// The bound function will return 0 on success.
//
// cmp
//    Comparator (rb_pointer_cmp_m or rb_safe_value_cmp_m could be used).
//
// tree
//    The root node of the tree. A pointer to nil represents an empty tree.
//
// key
//    The node used as search key.
//
// node
//    The output node.
//
// .. code-block:: cpp

#define rb_find_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        cmp, \
        tree, \
        key, \
        node \
) \
{ \
    assert(tree != NULL && "Tree was not initialized"); \
    assert(key != nil && "Do not use nil as search key"); \
    if(tree == nil) { \
        node = nil; \
    } else { \
        node = tree; \
        int __rb_find_result_ = 1; \
        while(__rb_find_result_ && node != nil) { \
            __rb_find_result_  = cmp((node), (key)); \
            if(__rb_find_result_ == 0) { \
                break; \
            } \
            node = __rb_find_result_ > 0 ? left(node) : right(node); \
        } \
    } \
} \


// rb_replace_node_m
// -----------------
//
// Bound: cx##_replace_node
//
// Replace a node with another. The cmp(old, new) has to return 0 or the
// function won't do anything.
//
// The bound function will return 0 on success.
//
// cmp
//    Comparator (rb_pointer_cmp_m or rb_safe_value_cmp_m could be used).
//
// tree
//    The root node of the tree. A pointer to nil represents an empty tree.
//
// old
//    The node to be replaced.
//
// new
//    The new node. Has not to be initialized since all fields are replaced.
//
// .. code-block:: cpp

#define rb_replace_node_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        cmp, \
        tree, \
        old, \
        new \
) \
{ \
    assert(tree != NULL && "Tree was not initialized"); \
    assert(tree != nil && "The tree can't be nil"); \
    assert(old != nil && "The old node can't be nil"); \
    assert(new != nil && "The new node can't be nil"); \
    assert(new != old && "The old and new node must differ"); \
    if(cmp((old), (new)) == 0) { \
        if(old == tree) { \
            tree = new; \
        } else { \
            if(old == left(parent(old))) { \
                left(parent(old)) = new; \
            } else { \
                right(parent(old)) = new; \
            } \
        } \
        if(left(old) != nil) { \
            parent(left(old)) = new; \
        } \
        if(right(old) != nil) { \
            parent(right(old)) = new; \
        } \
        parent(new) = parent(old); \
        left(new) = left(old); \
        right(new) = right(old); \
        color(new) = color(old); \
        /* Clear the old node. */ \
        parent(old) = nil; \
        left(old) = nil; \
        right(old) = nil; \
        color(old) = RB_BLACK; \
    } \
} \


// rb_bind_decl_m
// --------------
//
// Bind rbtree functions to a context. This only generates declarations.
//
// rb_bind_decl_cx_m is just an alias for consistency.
//
// cx
//    Name of the new context.
//
// type
//    The type of the nodes in the red-black tree.
//
// .. code-block:: cpp
//
#define rb_bind_decl_cx_m(cx, type) \
    rb_new_context_m(cx, type) \
    void \
    cx##_tree_init( \
            type** tree \
    ); \
    void \
    cx##_iter_init( \
            type* tree, \
            cx##_iter_t** iter, \
            type** elem \
    ); \
    void \
    cx##_iter_next( \
            cx##_iter_t* iter, \
            type** elem \
    ); \
    void \
    cx##_node_init( \
            type* node \
    ); \
    int \
    cx##_insert( \
            type** tree, \
            type* node \
    ); \
    void \
    cx##_delete_node( \
            type** tree, \
            type* node \
    ); \
    int \
    cx##_delete( \
            type** tree, \
            type* key, \
            type** node \
    ); \
    int \
    cx##_replace_node( \
            type** tree, \
            type* old, \
            type* new \
    ); \
    int \
    cx##_replace( \
            type** tree, \
            type* key, \
            type* new, \
            type** old \
    ); \
    int \
    cx##_find( \
            type* tree, \
            type* key, \
            type** node \
    ); \
    RB_SIZE_T \
    cx##_size( \
            type* tree \
    ); \
    rb_bind_decl_debug_cx_m(cx, type) \

#ifndef RB_NO_CHECK
#define rb_bind_decl_debug_cx_m(cx, type) \
    void \
    cx##_check_tree(type* tree); \
    void \
    cx##_check_tree_rec( \
            type* node, \
            int depth, \
            int *pathdepth \
    ); \

#else
#   define rb_bind_decl_debug_cx_m(cx, type)
#endif
#define rb_bind_decl_m(cx, type) rb_bind_decl_cx_m(cx, type)

// rb_bind_impl_m
// --------------
//
// Bind rbtree functions to a context. This only generates implementations.
//
// rb_bind_impl_m uses the standard traits: rb_color_m, rb_parent_m,
// rb_left_m, rb_right_m, whereas rb_bind_impl_cx_m expects you to create:
// cx##_color_m, cx##_parent_m, cx##_left_m, cx##_right_m.
//
// cx
//    Name of the new context.
//
// type
//    The type of the nodes in the red-black tree.
//
// .. code-block:: cpp
//
#define _rb_bind_impl_tr_m( \
        cx, \
        type, \
        color, \
        parent, \
        left, \
        right, \
        cmp \
) \
    cx##_type_t cx##_nil_mem; \
    cx##_type_t* const cx##_nil_ptr = &cx##_nil_mem; \
    void \
    cx##_tree_init( \
            type** tree \
    ) \
    { \
        rb_node_init_m( \
                cx##_nil_ptr, \
                color, \
                parent, \
                left, \
                right, \
                cx##_nil_ptr \
        ); \
        *tree = cx##_nil_ptr; \
    } \
    void \
    cx##_iter_init( \
            type* tree, \
            cx##_iter_t** iter, \
            type** elem \
    ) \
    { \
        (void)(iter); \
        rb_iter_init_m( \
            cx##_nil_ptr, \
            left, \
            tree, \
            *elem \
        ); \
    } \
    void \
    cx##_iter_next( \
            cx##_iter_t* iter, \
            type** elem \
    ) \
    { \
        (void)(iter); \
        rb_iter_next_m( \
            cx##_nil_ptr, \
            type, \
            parent, \
            left, \
            right, \
            *elem \
        ) \
    } \
    void \
    cx##_node_init( \
            type* node \
    ) \
    { \
        rb_node_init_m( \
                cx##_nil_ptr, \
                color, \
                parent, \
                left, \
                right, \
                node \
        ); \
    } \
    int \
    cx##_insert( \
            type** tree, \
            type* node \
    ) \
    { \
        rb_insert_m( \
            type, \
            cx##_nil_ptr, \
            color, \
            parent, \
            left, \
            right, \
            cmp, \
            *tree, \
            node \
        ); \
        return !( \
            parent(node) != cx##_nil_ptr || \
            left(node) != cx##_nil_ptr || \
            right(node) != cx##_nil_ptr || \
            *tree == node \
        ); \
    } \
    void \
    cx##_delete_node( \
            type** tree, \
            type* node \
    ) rb_delete_node_m( \
        type, \
        cx##_nil_ptr, \
        color, \
        parent, \
        left, \
        right, \
        *tree, \
        node \
    ) \
    int \
    cx##_delete( \
            type** tree, \
            type* key, \
            type** node \
    ) \
    { \
        if(cx##_find(*tree, key, node) == 0) { \
            cx##_delete_node(tree, *node); \
            return 0; \
        } \
        return 1; \
    } \
    int \
    cx##_replace_node( \
            type** tree, \
            type* old, \
            type* new \
    ) \
    { \
        rb_replace_node_m( \
            type, \
            cx##_nil_ptr, \
            color, \
            parent, \
            left, \
            right, \
            cmp, \
            *tree, \
            old, \
            new \
        ); \
        return !( \
            parent(old) == cx##_nil_ptr && \
            left(old) == cx##_nil_ptr && \
            right(old) == cx##_nil_ptr && \
            old != *tree \
        ); \
    } \
    int \
    cx##_replace( \
            type** tree, \
            type* key, \
            type* new, \
            type** old \
    ) \
    { \
        if(cx##_find(*tree, key, old) == 0) { \
            return cx##_replace_node(tree, *old, new); \
        } \
        return 1; \
    } \
    int \
    cx##_find( \
            type* tree, \
            type* key, \
            type** node \
    ) \
    { \
        rb_find_m( \
            type, \
            cx##_nil_ptr, \
            color, \
            parent, \
            left, \
            right, \
            cmp, \
            tree, \
            key, \
            *node \
        ); \
        return *node == cx##_nil_ptr; \
    } \
    RB_SIZE_T \
    cx##_size( \
            type* tree \
    ) \
    { \
        if(tree == cx##_nil_ptr) { \
            return 0; \
        } else { \
            return ( \
                cx##_size(left(tree)) + \
                cx##_size(right(tree)) + 1 \
            ); \
        } \
    } \
    _rb_bind_impl_debug_tr_m( \
            cx, \
            type, \
            color, \
            parent, \
            left, \
            right, \
            cmp \
    ) \

#ifndef RB_NO_CHECK
#define _rb_bind_impl_debug_tr_m( \
        cx, \
        type, \
        color, \
        parent, \
        left, \
        right, \
        cmp \
) \
    void \
    cx##_check_tree(type* tree) \
    { \
        int pathdepth = -1; \
        cx##_check_tree_rec(tree, 0, &pathdepth); \
    } \
    void \
    cx##_check_tree_rec( \
            type* node, \
            int depth, \
            int *pathdepth \
    ) rb_check_tree_m( \
        cx, \
        type, \
        color, \
        parent, \
        left, \
        right, \
        cmp, \
        node, \
        depth, \
        *pathdepth \
    ) \

#else
#define _rb_bind_impl_debug_tr_m( \
        cx, \
        type, \
        color, \
        parent, \
        left, \
        right, \
        cmp \
) \

#endif

#define rb_bind_impl_cx_m(cx, type) \
    _rb_bind_impl_tr_m( \
        cx, \
        type, \
        cx##_color_m, \
        cx##_parent_m, \
        cx##_left_m, \
        cx##_right_m, \
        cx##_cmp_m \
    ) \


#define rb_bind_impl_m(cx, type) \
    _rb_bind_impl_tr_m( \
        cx, \
        type, \
        rb_color_m, \
        rb_parent_m, \
        rb_left_m, \
        rb_right_m, \
        cx##_cmp_m \
    ) \


#define rb_bind_cx_m(cx, type) \
    rb_bind_decl_cx_m(cx, type) \
    rb_bind_impl_cx_m(cx, type) \


#define rb_bind_m(cx, type) \
    rb_bind_decl_m(cx, type) \
    rb_bind_impl_m(cx, type) \


// rb_check_tree_m
// ----------------
//
// Recursive: only works bound cx##_check_tree
//
// Check consistency of a tree
//
// node
//    Node to check.
//
// result
//    Zero on success, other on failure.
//
// .. code-block:: cpp
//
#define _rb_check_tree_m( \
        cx, \
        type, \
        color, \
        parent, \
        left, \
        right, \
        cmp, \
        node, \
        depth, \
        pathdepth, \
        tmp \
) \
{ \
    type* nil = cx##_nil_ptr; \
    if(node == nil) { \
        if(pathdepth < 0) { \
            pathdepth = depth; \
        } else { \
            assert(pathdepth == depth); \
        } \
    } else { \
        tmp = left(node); \
        if(tmp != nil) { \
            assert(parent(tmp) == node); \
            assert(cmp((tmp), (node)) < 0); \
        } \
        tmp = right(node); \
        if(tmp != nil) { \
            assert(parent(tmp) == node); \
            assert(cmp((tmp), (node)) > 0); \
        } \
        if(rb_is_red_m(color(node))) { \
            tmp = left(node); \
            if(tmp != nil) { \
                assert(rb_is_black_m(color(tmp))); \
            } \
            tmp = right(node); \
            if(tmp != nil) { \
                assert(rb_is_black_m(color(tmp))); \
            } \
            cx##_check_tree_rec(left(node), depth, &pathdepth); \
            cx##_check_tree_rec(right(node), depth, &pathdepth); \
        } else { \
            cx##_check_tree_rec(left(node), depth + 1, &pathdepth); \
            cx##_check_tree_rec(right(node), depth + 1, &pathdepth); \
        } \
    } \
} \

#define rb_check_tree_m( \
        cx, \
        type, \
        color, \
        parent, \
        left, \
        right, \
        cmp, \
        node, \
        depth, \
        pathdepth \
) \
{ \
    type* __rb_check_tmp_; \
    _rb_check_tree_m( \
        cx, \
        type, \
        color, \
        parent, \
        left, \
        right, \
        cmp, \
        node, \
        depth, \
        pathdepth, \
        __rb_check_tmp_ \
    ) \
} \


// Internal
// ========
//
// Functions that are used internally.
//
// _rb_rotate_left_m
// ------------------
//
// Internal: not bound
//
// A rotation is a local operation in a search tree that preserves in-order
// traversal key ordering. It is used to fix insert/deletion discrepancies.
// This operation might change the current root.
//
// _rb_rotate_right_m is _rb_rotate_left_m where left and right had been
// switched.
//
// tree
//    The root node of the tree. A pointer to nil represents an empty tree.
//
// node
//    The node to initialize.
//
// .. code-block:: text
//
//               .---.   rotate_right   .---.
//               | y |     ------->     | x |
//               .---.                  .---.
//              /     ∖                /     ∖
//         .---'     .-'-.        .---'      .'--.
//         | x |     | C |        | A |      | y |
//         .---.     '---'        '---'      .---.
//        /     ∖                           /     ∖
//     .-'-.    .'--.                    .-'-.    .'--.
//     | A |    | B |      <------       | B |    | C |
//     '---'    '---'    rotate_left     '---'    '---'
//
// .. code-block:: cpp
//
#define __rb_rotate_left_m( \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node, \
        x, \
        y \
) \
{ \
    x = node; \
    y = right(x); \
 \
    /* Turn y's left sub-tree into x's right sub-tree. */ \
    right(x) = left(y); \
    if(left(y) != nil) { \
        parent(left(y)) = x; \
    } \
    /* y's new parent was x's parent. */ \
    parent(y) = parent(x); \
    if(parent(x) == nil) { \
        /* If x is root y becomes the new root. */ \
        tree = y; \
    } else { \
        /* Set the parent to point to y instead of x. */ \
        if(x == left(parent(x))) { \
            /* x was on the left of its parent. */ \
            left(parent(x)) = y; \
        } else { \
            /* x must have been on the right. */ \
            right(parent(x)) = y; \
        } \
    } \
    /* Finally, put x on y's left. */ \
    left(y) = x; \
    parent(x) = y; \
} \


#define _rb_rotate_left_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node \
) \
{ \
    type* __rb_rot_x_; \
    type* __rb_rot_y_; \
    __rb_rotate_left_m( \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node, \
        __rb_rot_x_, \
        __rb_rot_y_ \
    ); \
} \


#define _rb_rotate_left_tr_m(cx, tree, node) \
    _rb_rotate_left_m( \
        cx##_type_t, \
        cx##_nil_ptr, \
        rb_color_m, \
        rb_parent_m, \
        rb_left_m, \
        rb_right_m, \
        tree, \
        node \
    ) \


#define _rb_rotate_right_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node \
) \
    _rb_rotate_left_m( \
        type, \
        nil, \
        color, \
        parent, \
        right, /* Switched */ \
        left,  /* Switched */ \
        tree, \
        node \
    ) \


#define _rb_rotate_right_tr_m(cx, tree, node) \
    _rb_rotate_right_m( \
        cx##_type_t, \
        cx##_nil_ptr, \
        rb_color_m, \
        rb_parent_m, \
        rb_left_m, \
        rb_right_m, \
        tree, \
        node \
    ) \


// _rb_insert_fix_m
// ----------------
//
// Internal: not bound
//
// After inserting the new node is labeled red, and possibly destroys the
// red-black property. The main loop moves up the tree, restoring the red-black
// property.
//
// tree
//    The root node of the tree. A pointer to nil represents an empty tree.
//
// node
//    The start-node to fix.
//
// .. code-block:: cpp
//
#define __rb_insert_fix_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node, \
        x, \
        y \
) \
{ \
    x = node; \
    /* Move up the tree and fix property 3. */ \
    while( \
            (x != tree) && \
            rb_is_red_m(color(parent(x))) \
    ) { \
        if(parent(x) == left(parent(parent(x)))) { \
            _rb_insert_fix_node_m( \
                type, \
                nil, \
                color, \
                parent, \
                left, \
                right, \
                _rb_rotate_left_m, \
                _rb_rotate_right_m, \
                tree, \
                x, \
                y \
            ); \
        } else { \
            _rb_insert_fix_node_m( \
                type, \
                nil, \
                color, \
                parent, \
                right, /* Switched */ \
                left, /* Switched */ \
                _rb_rotate_left_m, \
                _rb_rotate_right_m, \
                tree, \
                x, \
                y \
            ); \
        } \
    } \
    rb_make_black_m(color(tree)); \
} \


#define _rb_insert_fix_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node \
) \
{ \
    type* __rb_insf_x_; \
    type* __rb_insf_y_; \
    __rb_insert_fix_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node, \
        __rb_insf_x_, \
        __rb_insf_y_ \
    ); \
} \


#define _rb_insert_fix_node_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        rot_left, \
        rot_right, \
        tree, \
        x, \
        y \
) \
{ \
    y = right(parent(parent(x))); \
    /* Case 1: z’s uncle y is red. */ \
    if(rb_is_red_m(color(y))) { \
        rb_make_black_m(color(parent(x))); \
        rb_make_black_m(color(y)); \
        rb_make_red_m(color(parent(parent(x)))); \
        /* Locally property 3 is fixed, but changing the color of the \
         * grandparent might have created a new violation. We continue with the \
         * grandparent. */ \
        x = parent(parent(x)); \
    } else { \
        /* Case 2: z’s uncle y is black and z is a right child. */ \
        if(x == right(parent(x))) { \
            x = parent(x); \
            rot_left( \
                type, \
                nil, \
                color, \
                parent, \
                left, \
                right, \
                tree, \
                x \
            ); \
        } \
        /* Case 3: z’s uncle y is black and z is a left child. */ \
        rb_make_black_m(color(parent(x))); \
        rb_make_red_m(color(parent(parent(x)))); \
        rot_right( \
            type, \
            nil, \
            color, \
            parent, \
            left, \
            right, \
            tree, \
            parent(parent(x)) \
        ); \
    } \
} \


// _rb_delete_fix_m
// ----------------
//
// Internal: not bound
//
// After deleting a black node, the blackness is pushed down to the child. If
// it is black, it is now double (extra) black. Property 1 has to be restored.
//
// tree
//    The root node of the tree. A pointer to nil represents an empty tree.
//
// node
//    The start-node to fix.
//
// .. code-block:: cpp
//
#define __rb_delete_fix_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node, \
        x, \
        y \
) \
{ \
    x = node; \
    /* Move up fix extra blackness till x is red. */ \
    while( \
            (x != tree) && \
            rb_is_black_m(color(x)) \
    ) { \
        if(x == left(parent(x))) { \
            _rb_delete_fix_node_m( \
                type, \
                nil, \
                color, \
                parent, \
                left, \
                right, \
                _rb_rotate_left_m, \
                _rb_rotate_right_m, \
                tree, \
                x, \
                y \
            ); \
        } else { \
            _rb_delete_fix_node_m( \
                type, \
                nil, \
                color, \
                parent, \
                right, /* Switched */ \
                left, /* Switched */ \
                _rb_rotate_left_m, \
                _rb_rotate_right_m, \
                tree, \
                x, \
                y \
            ); \
        } \
    } \
    /* If x is red we can introduce a real black node. */ \
    rb_make_black_m(color(x)); \
} \


#define _rb_delete_fix_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node \
) \
{ \
    type* __rb_delf_x_; \
    type* __rb_delf_y_; \
    __rb_delete_fix_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        tree, \
        node, \
        __rb_delf_x_, \
        __rb_delf_y_ \
    ); \
} \


#define _rb_delete_fix_node_m( \
        type, \
        nil, \
        color, \
        parent, \
        left, \
        right, \
        rot_left, \
        rot_right, \
        tree, \
        x, \
        w \
) \
{ \
    /* X is double (extra) black. Goal: introduce a real black node. */ \
    w = right(parent(x)); \
    /* Case 1: x’s sibling w is red. */ \
    if(rb_is_red_m(color(w))) { \
        rb_make_black_m(color(w)); \
        rb_make_red_m(color(parent(x))); \
        rot_left( \
            type, \
            nil, \
            color, \
            parent, \
            left, \
            right, \
            tree, \
            parent(x) \
        ); \
        /* Transforms into case 2, 3 or 4 */ \
        w = right(parent(x)); \
    } \
    if( \
            rb_is_black_m(color(left(w))) && \
            rb_is_black_m(color(right(w))) \
    ) { \
        /* Case 2: x’s sibling w is black, and both of w’s children are black. */ \
        rb_make_red_m(color(w)); \
        /* Double blackness move up. Reenter loop. */ \
        x = parent(x); \
    } else { \
        /* Case 3: x’s sibling w is black, w’s left child is red, and w’s right \
         * child is black. */ \
        if(rb_is_black_m(color(right(w)))) { \
            rb_make_black_m(color(left(w))); \
            rb_make_red_m(color(w)); \
            rot_right( \
                type, \
                nil, \
                color, \
                parent, \
                left, \
                right, \
                tree, \
                w \
            ); \
            w = right(parent(x)); \
        } \
        /* Case 3: x’s sibling w is black, w’s left child is red, and w’s right \
         * child is black. */ \
        color(w) = color(parent(x)); \
        rb_make_black_m(color(parent(x))); \
        rb_make_black_m(color(right(w))); \
        rot_left( \
            type, \
            nil, \
            color, \
            parent, \
            left, \
            right, \
            tree, \
            parent(x) \
        ); \
        /* Terminate the loop. */ \
        x = tree; \
    } \
    /* When the loop ends x is red and will be colored black. */ \
} \


#endif // rb_tree_h

// MIT License
// ===========
//
// Copyright (c) 2017 Jean-Louis Fuchs
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ==============
// Message header
// ==============
//
// Message queue and internal message flags.
//
// .. code-block:: cpp
//
#ifndef ch_msg_message_h
#define ch_msg_message_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "common.h" */
/* #include "libchirp/message.h" */
/* #include "qs.h" */

// Queue declarations
// ==================
//
// .. code-block:: cpp
//
#define ch_msg_next_m(x) (x)->_next
qs_queue_bind_decl_cx_m(ch_msg, ch_message_t) CH_ALLOW_NL;

// .. c:type:: ch_msg_type_t
//
//    Represents message type flags.
//
//    .. c:member:: CH_MSG_REQ_ACK
//
//       Message requires ack.
//
//    .. c:member:: CH_MSG_ACK
//
//       Message is an ack.
//
// .. code-block:: cpp
//
typedef enum {
    CH_MSG_REQ_ACK = 1 << 0,
    CH_MSG_ACK     = 1 << 1,
    CH_MSG_NOOP    = 1 << 2,
} ch_msg_types_t;

// .. c:type:: ch_msg_flags_t
//
//    Represents message flags.
//
//    .. c:member:: CH_MSG_FREE_HEADER
//
//       Header data has to be freed before releasing the message
//
//    .. c:member:: CH_MSG_FREE_DATA
//
//       Data has to be freed before releasing the message
//
//    .. c:member:: CH_MSG_USED
//
//       The message is used by chirp
//
//    .. c:member:: CH_MSG_ACK_RECEIVED
//
//       Writer has received ACK.
//
//    .. c:member:: CH_MSG_WRITE_DONE
//
//       Write is done (last callback has been called).
//
//    .. c:member:: CH_MSG_FAILURE
//
//       On failure we still want to finish the message, therefore failure is
//       CH_MSG_ACK_RECEIVED || CH_MSG_WRITE_DONE.
//
//    .. c:member:: CH_MSG_HAS_SLOT
//
//       The message has a slot and therefore you have to call
//       :c:func:`ch_chirp_release_msg_slot`
//
// .. code-block:: cpp
//
typedef enum {
    CH_MSG_FREE_HEADER  = 1 << 0,
    CH_MSG_FREE_DATA    = 1 << 1,
    CH_MSG_USED         = 1 << 2,
    CH_MSG_ACK_RECEIVED = 1 << 3,
    CH_MSG_WRITE_DONE   = 1 << 4,
    CH_MSG_FAILURE      = CH_MSG_ACK_RECEIVED | CH_MSG_WRITE_DONE,
    CH_MSG_HAS_SLOT     = 1 << 5,
    CH_MSG_SEND_ACK     = 1 << 6,
} ch_msg_flags_t;

#endif // ch_msg_message_h
// ===========
// Util Header
// ===========
//
// Common utility functions.
//
// .. code-block:: cpp
//
#ifndef ch_util_h
#define ch_util_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "common.h" */
/* #include "libchirp/callbacks.h" */


// Declarations
// ============

// .. c:function::
void*
ch_alloc(size_t size);
//
//    Allocate fixed amount of memory.
//
//    :param size_t size: The amount of memory in bytes to allocate
//
// Debug alloc tracking
// --------------------
//
// .. code-block:: cpp

#ifdef CH_ENABLE_ASSERTS
// .. c:function::
int
ch_at_allocated(void* buf);
//
//    Check if a buffer currently is allocated. Return 1 if buffer is
//    allocated.
//
//    :param void* buf: Pointer to the buffer to track.
//
// .. c:function::
void
ch_at_init(void);
//
//    Initialize alloc tracking for memory leak debugging.
//
// .. c:function::
void
ch_at_cleanup(void);
//
//    Cleanup and print memory leak summary.
//
//
#endif

// .. c:function::
void
ch_bytes_to_hex(uint8_t* bytes, size_t bytes_size, char* str, size_t str_size);
//
//    Convert a bytes array to a hex string.
//
//    :param uint8_t* bytes:     Bytes to convert.
//    :param size_t bytes_size:  Length of the bytes to convert.
//    :param char* str:          Destination string.
//    :param size_t str_size:    Length of the buffer to write the string to.

// .. c:function::
void
ch_free(void* buf);
//
//    Free a memory handle.
//
//    :param void* buf: The handle to free.

// .. c:function::
int
ch_is_local_addr(ch_text_address_t* addr);
//
//    Check if an address is either 127.0.0.1 or ::1
//
//    :param ch_text_address_t* addr: Address to check

// .. c:function::
void
ch_random_ints_as_bytes(uint8_t* bytes, size_t len);
//
//    Fill in random ints efficiently.
//    The parameter ``len`` MUST be multiple of four.
//
//    Thank you windows, for making this really complicated.
//
//    :param uint8_t* bytes:  The buffer to fill the bytes into.
//    :param size_t  len:     The length of the buffer.

// .. c:function::
void*
ch_realloc(void* buf, size_t size);
//
//    Resize allocated memory.
//
//    :param void* buf:    The handle to resize.
//    :param size_t size:  The new size of the memory in bytes.

// .. c:function::
ch_error_t
ch_textaddr_to_sockaddr(
        int                      af,
        ch_text_address_t*       text,
        uint16_t                 port,
        struct sockaddr_storage* addr);
//
//    Convert a text address to a struct sockaddr. As an input we want struct
//    sockaddr_storage, to have enough space for an IPv4 and IPv6 address, but
//    you can cast it to struct sockaddr afterwards.
//
//    :param int af:                  Either AF_INT or AF_INET6
//    :param ch_text_address_t* text: A text representation of the address
//    :param uint16_t port:           The port
//    :param sockaddr_storage* addr:  The socket to set

#endif // ch_util_h
// ===============
// Protocol Header
// ===============
//
// Handles connections (global) and low-level reading per connection.
//
// .. code-block:: cpp

#ifndef ch_protocol_h
#define ch_protocol_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "common.h" */
/* #include "connection.h" */
/* #include "libchirp/chirp.h" */
/* #include "rbtree.h" */

// Declarations
// ============

// .. c:type:: ch_protocol_t
//
//    Protocol object.
//
//    .. c:member:: struct sockaddr_in addrv4
//
//       BIND_V4 address converted to a sockaddr_in.
//
//    .. c:member:: struct sockaddr_in addrv6
//
//       BIND_V6 address converted to a sockaddr_in6.
//
//    .. c:member:: uv_tcp_t serverv4
//
//       Reference to the libuv tcp server handle, IPv4.
//
//    .. c:member:: uv_tcp_t serverv6
//
//       Reference to the libuv tcp server handle, IPv6.
//
//    .. c:member:: ch_remote_t* remotes
//
//       Pointer to tree of remotes. They can have a connection.
//
//    .. c:member:: ch_remote_t* reconnect_remotes
//
//       A stack of remotes that should be reconnected after a timeout.
//
//    .. c:member:: uv_timer_t reconnect_timeout
//
//       Timeout after which we try to reconnect remotes.
//
//    .. c:member:: uv_timer_t gc_timeout
//
//       Recurrent timeout garbage-collecting old connections.
//
//       Timeout after which we try to reconnect remotes.
//
//    .. c:member:: ch_connection_t* old_connections
//
//       Pointer to old connections. This is mainly used when there is a
//       network race condition. The then current connections will be replaced
//       and saved as old connections for garbage collection.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Pointer to the chirp object. See: :c:type:`ch_chirp_t`.
//
// .. code-block:: cpp
//
struct ch_protocol_s {
    struct sockaddr_in  addrv4;
    struct sockaddr_in6 addrv6;
    uv_tcp_t            serverv4;
    uv_tcp_t            serverv6;
    ch_remote_t*        remotes;
    ch_remote_t*        reconnect_remotes;
    uv_timer_t          reconnect_timeout;
    uv_timer_t          gc_timeout;
    ch_connection_t*    old_connections;
    ch_connection_t*    handshake_conns;
    ch_chirp_t*         chirp;
};

// .. c:function::
ch_error_t
ch_pr_conn_start(
        ch_chirp_t* chirp, ch_connection_t* conn, uv_tcp_t* client, int accept);
//
//    Start the given connection
//
//    :param ch_chirp_t* chirp: Chirp object
//    :param ch_connection_t* conn: Connection object
//    :param uv_tcp_t* client: Client to start
//    :param int accept: Is accepted connection
//
//    :return: Void since only called from callbacks.

// .. c:function::
void
ch_pr_close_free_remotes(ch_chirp_t* chirp, int only_conns);
//
//    Close and free all remaining connections.
//
//    :param ch_chirpt_t* chirp: Chrip object
//    :paramint only_conns: (bool) Only close / free connections. Do not touch
//                          remotes
//

// .. c:function::
void
ch_pr_debounce_connection(ch_connection_t* conn);
//
//    Disallow reconnect for 50 - 550ms, in order to prevent network races and
//    give the remote time to recover.
//
//    :param ch_connection_t* conn: Connection object
//

#ifndef CH_WITHOUT_TLS
// .. c:function::
void
ch_pr_decrypt_read(ch_connection_t* conn, int* stop);
//
//    Reads data over SSL on the given connection. Returns 1 if something was
//    read, 0 otherwise.
//
//    :param ch_connection_t* conn: Pointer to a connection handle.
//    :param int* stop:             (Out) Stop the reading process.
//
#endif

// .. c:function::
void
ch_pr_reconnect_remotes_cb(uv_timer_t* handle);
//
//    Reconnects remotes that had connect failure after a timeout
//
//    :param uv_timer_t* handle: uv timer handle, data contains chirp

// .. c:function::
void
ch_pr_restart_stream(ch_connection_t* conn);
//
//    Try to restart the current stream on this connection.
//
//    :param ch_connection_t* conn: Connection to restart.

// .. c:function::
ch_error_t
ch_pr_start(ch_protocol_t* protocol, uint16_t* uninit);
//
//    Start the given protocol.
//
//    :param ch_protocol_t* protocol: Protocol which shall be started
//    :param uint16_t uninit: Track initialization state
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
ch_error_t
ch_pr_stop(ch_protocol_t* protocol);
//
//    Stop the given protocol.
//
//    :param ch_protocol_t* protocol: Protocol which shall be stopped.
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:function::
void
ch_pr_init(ch_chirp_t* chirp, ch_protocol_t* protocol);
//
//    Initialize the protocol structure.
//
//    :param ch_chirp_t* chirp: Chirp instance.
//    :param ch_protocol_t* protocol: Protocol to initialize.
//
// .. code-block:: cpp
//
#endif // ch_protocol_h
//
// .. code-block:: cpp

/* #include "libchirp-config.h" */
#ifndef CH_WITHOUT_TLS

// =================
// Encryption header
// =================
//
// Setting up and cleaning up TLS.
//
// .. code-block:: cpp

#ifndef ch_encryption_h
#define ch_encryption_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "libchirp/chirp.h" */

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <openssl/ssl.h>
#include <string.h>

// Determine OpenSSL API
// =====================
//
// .. code-block:: cpp

#ifdef LIBRESSL_VERSION_NUMBER
#define CH_OPENSSL_10_API
#define CH_LIBRESSL
#else
#define CH_OPENSSL
#if (OPENSSL_VERSION_NUMBER <= 0x10100000L)
#define CH_OPENSSL_10_API
#endif
#endif

// Declarations
// ============

// .. c:type:: ch_en_tls_ops_t
//
//    Represents TLS operations.
//
//    .. c:member:: CH_EN_OP_HANDSHAKE
//
//       Continue with handshake
//
//    .. c:member:: CH_EN_OP_READ
//
//       Read data from remote
//
//    .. c:member:: CH_EN_OP_WRITE
//
//       Write data to remote
//
//    .. c:member:: CH_EN_OP_SHUTDOWN
//
//       Continue with shutdown
//
// .. code-block:: cpp
//
typedef enum {
    CH_EN_OP_HANDSHAKE = 0,
    CH_EN_OP_READ      = 1,
    CH_EN_OP_WRITE     = 2,
    CH_EN_OP_SHUTDOWN  = 3,
} ch_en_tls_ops_t;

// .. c:type:: ch_encryption_t
//
//    Encryption object.
//
//    .. c:member:: ch_chirp_t*
//
//       reference back to chirp
//
// .. code-block:: cpp
//
typedef struct ch_encryption_s {
    ch_chirp_t* chirp;
    SSL_CTX*    ssl_ctx;
} ch_encryption_t;

// .. c:function::
ch_error_t
ch_en_start(ch_encryption_t* enc);
//
//    Start the encryption.
//
//    :param ch_encryption_t* enc: Pointer to a encryption object (holding chirp
//                                 and OpenSSL context)
//
//   :return: A chirp error. see: :c:type:`ch_error_t`
//   :rtype:  ch_error_t

// .. c:function::
ch_error_t
ch_en_stop(ch_encryption_t* enc);
//
//    Stop the encryption.
//
//    :param ch_encryption_t* enc: pointer to a encryption object (holding chirp
//                                 and openssl context)
//
//   :return: A chirp error. see: :c:type:`ch_error_t`
//   :rtype:  ch_error_t

// Definitions
// ===========

// .. c:function::
void
ch_en_init(ch_chirp_t* chirp, ch_encryption_t* enc);
//
//    Initialize the encryption struct.
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_encryption_t* enc: Encryption to initialize

#endif // ch_encryption_h
#endif // CH_WITHOUT_TLS
// =============
// Remote header
// =============
//
// ch_remote_t represents a remote node.
//
// .. code-block:: cpp
//
#ifndef ch_remote_h
#define ch_remote_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "common.h" */
/* #include "connection.h" */
/* #include "rbtree.h" */

// Declarations
// ============
//
// .. c:type:: ch_rm_flags_t
//
//    Represents connection flags.
//
//    .. c:member:: CH_RM_CONN_BLOCKED
//
//       Indicates remote is block from connecting
//
//    .. c:member:: CH_RM_GCED
//
//       Indicates that the remote has been garbage-collected
//
// .. code-block:: cpp

typedef enum {
    CH_RM_CONN_BLOCKED = 1 << 0,
} ch_rm_flags_t;

// .. c:type:: ch_remote_t
//
//    ch_remote_t represents remote node and is a dictionary used to lookup
//    remotes.
//
//    .. c:member:: uint8_t ip_protocol
//
//       What IP protocol (IPv4 or IPv6) shall be used for connections.
//
//    .. c:member:: uint8_t[16] address
//
//       IPv4/6 address of the sender if the message was received.  IPv4/6
//       address of the recipient if the message is going to be sent.
//
//    .. c:member:: int32_t port
//
//       The port that shall be used for connections.
//
//    .. c:member:: ch_connection_t* conn
//
//       The active connection to this remote. Can be NULL. Callbacks always
//       have to check if the connection is NULL. The code that sets the
//       connection to NULL has to notify the user. So callbacks can safely
//       abort if conn is NULL.
//
//    .. c:member:: ch_message_t* msg_queue
//
//       Queue of messages.
//
//    .. c:member:: ch_message_t* cntl_msg_queue
//
//       Queue of ack/noop messages. There can max be two acks plus a noop
//       in the queue, one ack from the current (new) connection and one the
//       old connection and noop from the remote.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Pointer to the chirp object. See: :c:type:`ch_chirp_t`.
//
//    .. c:member:: uint32_t serial
//
//       The current serial number for this remote
//
//    .. c:member:: uint8_t flags
//
//       Flags of the remote
//
//    .. c:member:: uint64_t timestamp
//
//       Timestamp when the connection was last used. Used to determine
//       garbage-collection.
//
//    .. c:member:: char color
//
//       rbtree member
//
//    .. c:member:: ch_remote_t* left
//
//       rbtree member
//
//    .. c:member:: ch_remote_t* right
//
//       rbtree member
//
//    .. c:member:: ch_remote_t* parent
//
//       rbtree member
//
//    .. c:member:: ch_remote_t* next
//
//       stack member
//
// .. code-block:: cpp
//
struct ch_remote_s {
    uint8_t          ip_protocol;
    uint8_t          address[CH_IP_ADDR_SIZE];
    int32_t          port;
    ch_connection_t* conn;
    ch_message_t*    noop;
    ch_message_t*    msg_queue;
    ch_message_t*    cntl_msg_queue;
    ch_message_t*    wait_ack_message;
    ch_chirp_t*      chirp;
    uint32_t         serial;
    uint8_t          flags;
    uint64_t         timestamp;
    char             color;
    ch_remote_t*     parent;
    ch_remote_t*     left;
    ch_remote_t*     right;
    ch_remote_t*     next;
};

// rbtree prototypes
// ------------------
//
// .. code-block:: cpp
//
#define ch_rm_cmp_m(x, y) ch_remote_cmp(x, y)

// stack prototypes
// ----------------
//
// .. code-block:: cpp

qs_stack_bind_decl_m(ch_rm_st, ch_remote_t) CH_ALLOW_NL;

rb_bind_decl_m(ch_rm, ch_remote_t) CH_ALLOW_NL;

// .. c:function::
void
ch_rm_init_from_msg(
        ch_chirp_t* chirp, ch_remote_t* remote, ch_message_t* msg, int key);
//
//    Initialize the remote data-structure from a message.
//
//    :param ch_chirp_t*   chirp: Chirp object
//    :param ch_remote_t* remote: Remote to initialize
//    :param ch_message_t*   msg: Message to initialize from
//    :param int             key: Used as a key only

// .. c:function::
void
ch_rm_init_from_conn(
        ch_chirp_t* chirp, ch_remote_t* remote, ch_connection_t* conn, int key);
//
//    Initialize the remote data-structure from a connection.
//
//    :param ch_chirp_t*     chirp: Chirp object
//    :param ch_remote_t*   remote: Remote to initialize
//    :param ch_connection_t* conn: Connection to initialize from
//    :param int               key: Used as a key only
//
// .. c:function::
void
ch_rm_free(ch_remote_t* remote);
//
//    Free the remote data-structure.
//
//    :param ch_remote_t* remote: Remote to initialize
//
// .. code-block:: cpp
//
#endif // ch_remote_h
// =================
// Serializer header
// =================
//
// * Convert the wire message to a buffer in network order
// * Convert a buffer in network order to the wire message
//
// Wire message refers to a part of ch_message_t before the ch_buf* header
// member.
//
// * Convert ch_sr_handshake_t to a buffer in network order
// * Convert a buffer in network order to ch_sr_handshake_t
//
// .. code-block:: cpp

#ifndef ch_serializer_h
#define ch_serializer_h

// Project includes
// ================
//
// .. code-block:: cpp

/* #include "common.h" */
/* #include "libchirp/message.h" */

// Declarations
// ============

// .. c:type:: ch_sr_handshake_t
//
//    Handshake data structure.
//
//    .. c:member:: uint16_t port
//
//       Public port which is passed to a connection on a successful handshake.
//
//    .. c:member:: uint8_t[16] identity
//
//       The identity of the remote target which is passed to a connection upon
//       a successful handshake. It is used by the connection for getting the
//       remote address.
//
// .. code-block:: cpp

typedef struct ch_sr_handshake_s {
    uint16_t port;
    uint8_t  identity[CH_ID_SIZE];
} ch_sr_handshake_t;

#define CH_SR_WIRE_MESSAGE_SIZE 27

#ifdef CH_ENABLE_ASSERTS
#define CH_SR_WIRE_MESSAGE_CHECK                                               \
    pos += 4;                                                                  \
    A(pos == CH_SR_WIRE_MESSAGE_SIZE, "Bad message serialization size");
#else
#define CH_SR_WIRE_MESSAGE_CHECK
#endif

#define CH_SR_WIRE_MESSAGE_LAYOUT                                              \
    size_t   pos      = 0;                                                     \
    uint8_t* identity = (void*) &buf[pos];                                     \
    pos += CH_ID_SIZE;                                                         \
                                                                               \
    uint32_t* serial = (void*) &buf[pos];                                      \
    pos += 4;                                                                  \
                                                                               \
    uint8_t* type = (void*) &buf[pos];                                         \
    pos += 1;                                                                  \
                                                                               \
    uint16_t* header_len = (void*) &buf[pos];                                  \
    pos += 2;                                                                  \
                                                                               \
    uint32_t* data_len = (void*) &buf[pos];                                    \
    CH_SR_WIRE_MESSAGE_CHECK

// .. c:function::
int
ch_sr_buf_to_msg(ch_buf* buf, ch_message_t* msg);
//
//    Convert a buffer containing the packed data of the wire message in network
//    order to an ch_message_t.
//
//    :param ch_buf* buf: Pointer to the packed buffer of at least
//                        CH_SR_WIRE_MESSAGE_SIZE
//    :param ch_message_t* msg: Pointer to the message

// .. c:function::
int
ch_sr_msg_to_buf(ch_message_t* msg, ch_buf* buf, uint32_t msg_serial);
//
//    Convert a ch_message_t to a buffer containing the packed data of
//    the wire message in network order.
//
//    :param ch_message_t* msg: Pointer to the message
//    :param ch_buf* buf: Pointer to the packed buffer of at least
//                        CH_SR_WIRE_MESSAGE_SIZE
//    :param uint32_t msg_serial: Serial of the message
//
// .. code-block:: cpp

#define CH_SR_HANDSHAKE_SIZE 18

#ifdef CH_ENABLE_ASSERTS
#define CH_SR_HANDSHAKE_CHECK                                                  \
    pos += CH_ID_SIZE;                                                         \
    A(pos == CH_SR_HANDSHAKE_SIZE, "Bad handshake serialization size");
#else
#define CH_SR_HANDSHAKE_CHECK
#endif

#define CH_SR_HANDSHAKE_LAYOUT                                                 \
    size_t    pos  = 0;                                                        \
    uint16_t* port = (void*) &buf[pos];                                        \
    pos += 2;                                                                  \
                                                                               \
    uint8_t* identity = (void*) &buf[pos];                                     \
    CH_SR_HANDSHAKE_CHECK

// .. c:function::
int
ch_sr_buf_to_hs(ch_buf* buf, ch_sr_handshake_t* hs);
//
//    Convert a buffer containing the packed data of a handshake in network
//    order to an ch_sr_handshake_t.
//
//    :param ch_buf* buf: Pointer to the packed buffer of at least
//                        CH_SR_HANDSHAKE_SIZE
//    :param ch_sr_handshake_t: Pointer to the handshake struct

// .. c:function::
int
ch_sr_hs_to_buf(ch_sr_handshake_t* hs, ch_buf* buf);
//
//    Convert a ch_sr_handshake_t to  a buffer containing the packed data of
//    the handshake in network order.
//
//    :param ch_sr_handshake_t: Pointer to the handshake struct
//    :param ch_buf* buf: Pointer to the packed buffer of at least
//                        CH_SR_HANDSHAKE_SIZE
//
// .. code-block:: cpp
//
#endif // ch_serializer_h
// =============
// Writer Header
// =============
//
// Chirp protocol writer. Everything about putting a message on the wire.
//
// .. code-block:: cpp
//
#ifndef ch_writer_h
#define ch_writer_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "common.h" */
/* #include "libchirp/callbacks.h" */
/* #include "libchirp/message.h" */
/* #include "serializer.h" */

// Declarations
// ============
//
// Direct declarations
// -------------------
//
// .. c:type:: ch_writer_t
//
//    The chirp protocol writer data strcture.
//
//    .. c:member:: ch_send_cb_t send_cb
//
//       Callback that will be called upon successful sending of a message.
//
//    .. c:member:: uv_timer_t send_timeout
//
//       Libuv timer handle for setting a timeout when trying to send a
//       message. At the end of the defined timeout time, the timer triggers
//       the :c:func:`_ch_wr_write_timeout_cb` callback.
//
//    .. c:member:: ch_message_t* msg
//
//       Pointer to a message. The message is set when sending through
//       :c:func:`ch_wr_write` and being read again during the callbacks.
//
//    .. c:member:: ch_msg_message_t net_msg
//
//       Used to serialize the wire message to.
//
// .. code-block:: cpp
//
typedef struct ch_writer_s {
    uv_timer_t    send_timeout;
    ch_message_t* msg;
    ch_buf        net_msg[CH_SR_WIRE_MESSAGE_SIZE];
} ch_writer_t;

// .. c:function::
void
ch_wr_free(ch_writer_t* writer);
//
//    Free the writer data structure.
//
//    :param ch_writer_t* writer: Pointer to writer (data-) structure.
//

// .. c:function::
ch_error_t
ch_wr_init(ch_writer_t* writer, ch_connection_t* conn);
//
//    Initialize the writer data structure.
//
//    Initializes the writers mutex and the libuv timer for handling timeouts
//    when sending. The connection gets set as data pointer for the sending
//    timeout.
//
//    :param ch_chirp_t* chirp:      Pointer to a chirp instance.
//    :param ch_connection_t* conn:  Pointer to a connection instance.

// .. c:function::
ch_error_t
ch_wr_process_queues(ch_remote_t* remote);
//
//    Sends queued message according to the following priorities:
//
//    1. Do nothing if the writer still has an active message
//    2. Send messages that don't require an ack (which might be acks)
//    3. Send messages that require an ack
//    4. Do nothing
//
//    :param ch_remote_t* remote: The remote to process queues

// .. c:function::
ch_error_t
ch_wr_send(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb);
//    Same as ch_chirp_send just for internal use.
//
//    see: :c:func:`ch_chirp_send`
//

// .. c:function::
void
ch_wr_send_ts_cb(uv_async_t* handle);
//
//    Send all messages in the message queue.
//
//    :param uv_async_t* handle: Async handler used to trigger sending message
//                               queue.

// .. c:function::
void
ch_wr_write(ch_connection_t* conn, ch_message_t* msg);
//
//    Send the message after a connection has been established.
//
//    :param ch_connection_t* conn:  Connection to send the message over.
//    :param ch_message_t msg:       The message to send. The memory of the
//                                   message must stay valid until the callback
//                                   is called.

#endif // ch_writer_h
// =============
// Buffer header
// =============
//
// Implements a buffer pool. There is header and data buffer per chirp
// message-slot.
//
// .. code-block:: cpp
//
#ifndef ch_buffer_h
#define ch_buffer_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "common.h" */
/* #include "libchirp-config.h" */
/* #include "message.h" */

// Declarations
// ============

// .. c:type:: ch_bf_slot_t
//
//    Preallocated buffer for a chirp message-slot.
//
//    .. c:member:: ch_message_t
//
//       Preallocated message.
//
//    .. c:member:: ch_buf* header
//
//       Preallocated buffer for the chirp header.
//
//    .. c:member:: ch_buf* data
//
//       Preallocated buffer for the data.
//
//    .. c:member:: uint8_t id
//
//       Identifier of the buffer.
//
//    .. c:member:: uint8_t used
//
//       Indicates how many times the buffer is/was used.
//
// .. code-block:: cpp
//
typedef struct ch_bf_slot_s {
    ch_message_t msg;
    ch_buf       header[CH_BF_PREALLOC_HEADER];
    ch_buf       data[CH_BF_PREALLOC_DATA];
    uint8_t      id;
    uint8_t      used;
} ch_bf_slot_t;

// .. c:type:: ch_buffer_pool_t
//
//    Contains the preallocated buffers for the chirp message-slot.
//
//    .. c:member:: unsigned int refcnf
//
//       Reference count
//
//    .. c:member:: uint8_t max_slots
//
//       The maximum number of buffers (slots).
//
//    .. c:member:: uint8_t used_slots
//
//       How many slots are currently used.
//
//    .. c:member:: uint32_t free_slots
//
//       Bit mask of slots that are currently free (and therefore may be used).
//
//    .. c:member:: ch_bf_slot_t* slots
//
//       Pointer of type ch_bf_slot_t to the actual slots. See
//       :c:type:`ch_bf_slot_t`.
//
//    .. c:member:: ch_connection_t*
//
//       Pointer to connection that owns the pool
//
// .. code-block:: cpp
//
typedef struct ch_buffer_pool_s {
    unsigned int     refcnt;
    uint8_t          max_slots;
    uint8_t          used_slots;
    uint32_t         free_slots;
    ch_bf_slot_t*    slots;
    ch_connection_t* conn;
} ch_buffer_pool_t;

// .. c:function::
void
ch_bf_free(ch_buffer_pool_t* pool);
//
//    Free the given buffer structure.
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure to free
//

// .. c:function::
ch_error_t
ch_bf_init(ch_buffer_pool_t* pool, ch_connection_t* conn, uint8_t max_slots);
//
//    Initialize the given buffer pool structure using given max slots.
//
//    :param ch_buffer_pool_t* pool: The buffer pool object
//    :param ch_connection_t* conn: Connection that owns the pool
//    :param uint8_t max_slots: Slots to allocate
//

// .. c:function::
ch_bf_slot_t*
ch_bf_acquire(ch_buffer_pool_t* pool);
//
//    Acquire and return a new buffer from the pool. If no slot can
//    be reserved NULL is returned.
//
//    :param ch_buffer_pool_t* pool: The buffer pool structure which the
//                                   reservation shall be made from.
//    :return: a pointer to a reserved buffer from the given buffer
//            pool. See :c:type:`ch_bf_slot_t`
//    :rtype:  ch_bf_slot_t
//
// .. c:function::
static inline int
ch_bf_is_exhausted(ch_buffer_pool_t* pool)
//
//    Returns 1 if the pool is exhausted.
//
//    :param ch_buffer_pool_t* pool: The buffer pool object
//
// .. code-block:: cpp
//
{
    return pool->used_slots >= pool->max_slots;
}

// .. c:function::
void
ch_bf_release(ch_buffer_pool_t* pool, int id);
//
//    Set given slot as unused in the buffer pool structure and (re-)add it to
//    the list of free slots.
//
//    :param int id: The id of the slot that should be marked free
//
// .. code-block:: cpp
//
#endif // ch_buffer_h
// =============
// Reader Header
// =============
//
// Reader state machine and buffer pool. Everything about reading a message
// from the wire.
//
// .. code-block:: cpp
//
#ifndef ch_reader_h
#define ch_reader_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "buffer.h" */
/* #include "common.h" */
/* #include "message.h" */
/* #include "serializer.h" */

// Declarations
// ============

// .. c:type:: ch_rd_state_t
//
//    Possible states of a reader.
//
//    .. c:member:: CH_RD_START
//
//       Initial state, chirp handshake has to be done.
//
//    .. c:member:: CH_RD_WAIT
//
//       Wait for the next message.
//
//    .. c:member:: CH_RD_SLOT
//
//       Acquire a slot.
//
//    .. c:member:: CH_RD_HEADER
//
//       Read header.
//
//    .. c:member:: CH_RD_DATA
//
//       Read data.
//
// .. code-block:: cpp
//
typedef enum {
    CH_RD_START     = 0,
    CH_RD_HANDSHAKE = 1,
    CH_RD_WAIT      = 2,
    CH_RD_SLOT      = 3,
    CH_RD_HEADER    = 4,
    CH_RD_DATA      = 5,
} ch_rd_state_t;

// .. c:type:: ch_reader_t
//
//    Defines the state of a reader.
//
//    .. c:member:: ch_rd_state_t state
//
//       Current state of the reader (finite-state machine).
//
//    .. c:member:: ch_message_t* msg
//
//       Current message
//
//    .. c:member:: ch_bf_slot_t* slot
//
//       Current message-slot
//
//    .. c:member:: ch_message_t wire_msg
//
//       Buffer used store wire message
//
//    .. c:member:: size_t bytes_read
//
//       Counter for how many bytes were already read by the reader. This is
//       used when :c:func:`ch_rd_read` is called with a buffer of
//       :c:member:`ch_rd_read.read` bytes to read but not enough bytes are
//       being delivered over the connection :c:member:`ch_rd_read.conn`.
//
//    .. c:member:: ch_buffer_pool_t pool
//
//       Data structure containing preallocated buffers for the chirp
//       message-slots.
//
// .. code-block:: cpp
//
typedef struct ch_reader_s {
    ch_rd_state_t     state;
    ch_bf_slot_t*     slot;
    ch_message_t      wire_msg;
    size_t            bytes_read;
    ch_buf            net_msg[CH_SR_WIRE_MESSAGE_SIZE];
    ch_buffer_pool_t* pool;
} ch_reader_t;

// .. c:function::
void
ch_rd_free(ch_reader_t* reader);
//
//    Free the (data-) buffer pool of the given reader instance.
//
//    :param ch_reader_t* reader: The reader instance whose buffer
//                                pool shall be freed.

// .. c:function::
ch_error_t
ch_rd_init(ch_reader_t* reader, ch_connection_t* conn, ch_chirp_int_t* ichirp);
//
//    Initialize the reader structure.
//
//    :param ch_reader_t* reader: The reader instance whose buffer pool shall
//                                be initialized with ``max_slots``.
//    :param ch_connection_t* conn: Connection that owns the reader
//    :param ch_chirp_int_t ichirp: Internal chirp instance
//    :rtype: ch_error_t

// .. c:function::
ssize_t
ch_rd_read(ch_connection_t* conn, ch_buf* buffer, size_t bytes_read, int* stop);
//
//    Implements the wire protocol reader part. Returns bytes handled.
//
//    :param ch_connection_t* conn: Connection the data was read from.
//    :param void* buffer:          The buffer containing ``read`` bytes read.
//    :param size_t bytes_read:     The number of bytes read.
//    :param int* stop:             (Out) Stop the reading process.
//
// .. code-block:: cpp

#endif // ch_reader_h
// =================
// Connection header
// =================
//
// Represents a connection and implements low-level write (low-level read is
// implemented in protocol.h/c).
//
// Connection state machine
// ========================
//
// Variables
// ---------
//
// * C = Connection pointer valid
//
// * Q = A message is queued
//
// * V = Connection timestamp is valid (connection did not timeout)
//
// Actions
// -------
//
// enqu = enqueue
//    Enqueue the message passed to ch_chirp_send().
//
// dequ = dequeue
//    Dequeue a message and send it, or go to idle if there is no message.
//
// bufs = create buffers
//    Create all the data-structures and buffers needed for running a
//    connection.
//
// chck = check
//    Check if a connection has been idle for REUSE_TIME seconds, since the
//    connection is valid: do nothing.
//
// gc = collect connection
//    Check if a connection has been idle for REUSE_TIME seconds, since the
//    connection is not valid: close it if needed and cleanup ch_remote_t.
//
// wait = wait
//    Reconnect after waiting for 1 seconds.
//
// clen = cleanup
//    Cleanup data-structures and buffers, remove pointer to connection from
//    ch_remote_t.
//
//
// States
// ------
//
// null = null
//    ch_remote_t for the destination address in the message does not exist.
//
// clos = closed
//    ch_remote_t does exist but there is no pointer to the connection.
//
// cing = connecting
//    ch_remote_t exists but the connection is still connecting.
//
// idle = idle
//    ch_remote_t exists, the connection is valid and idle.
//
// invalid idle = (Not reachable)
//    Same as idle, but the connection timed out. On next gc it will be removed.
//
// send = sending
//    The connection is blocked, messages will be enqueued and not sent.
//
// Triggers
// --------
//
// send
//    The user of libchirp sends a message
//
// done
//    Defined below
//
// fail
//    Sending of a message fails (all kinds of error are treated the same)
//
// timer
//    The garbage collector timer is triggered (callback from libuv)
//
//
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
// |                      | send      | done      | fail      | timer      |
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
// |Name         | C| Q| V| act | next| act | next| act | next| act  | next|
// +=============+==+==+==+=====+=====+=====+=====+=====+=====+======+=====+
// |null         | 0| 0| 0| init| cing| init| idle| xxx | xxx | xxx  | xxx |
// |             |  |  |  | enqu|     | bufs|     |     |     |      |     |
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
// |closed       | 0| 0| 1| conn| cing| xxx | xxx | xxx | xxx | chck | clos|
// |             |  |  |  | enqu|     |     |     |     |     |      |     |
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
// |xxx          | 0| 1| 0| xxx | xxx | xxx | xxx | xxx | xxx | xxx  | xxx |
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
// |connecting   | 0| 1| 1| enqu| cing| bufs| send| wait| cing| chck | cing|
// |             |  |  |  |     |     |     |     |     | clos|      |     |
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
// |invalid idle | 1| 0| 0| enqu| send| xxx | xxx | clen| clos| gc   | null|
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
// |idle         | 1| 0| 1| enqu| send| xxx | xxx | clen| clos| chck | idle|
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
// |xxx          | 1| 1| 0| xxx | xxx | xxx | xxx | xxx | xxx | xxx  | xxx |
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
// |sending      | 1| 1| 1| enqu| send| dequ| send| wait| cing| chck | send|
// |             |  |  |  |     |     |     | idle|     |     |      |     |
// +-------------+--+--+--+-----+-----+-----+-----+-----+-----+------+-----+
//
// Wide version of this table is in doc/chirp_state_machine.ods
//
// Done depnds on current state:
//
// * null: successful accept()
//
// * connecting: successful connect()
//
// * sending: successful send()
//
// One important thing to understand. There can be network races and if the two
// ends of a peer-association open a connection at the same time, while we just
// close one connection when we receive another one, we can end up in a endless
// loop. Since if both nodes do not decide to close the same one, both
// connections are gone and we start fresh. We do not solve that problem by
// randomized reconnect timeouts, but rather by allowing two connections for
// REUSE_TIME + random time. That is the reason, the queues and
// wait_ack_message are in a shared structure called ch_remote_t. If both
// connections are used they will stay.
//
// .. code-block:: cpp
//
#ifndef ch_connection_h
#define ch_connection_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "common.h" */
/* #include "libchirp/chirp.h" */
/* #include "message.h" */
/* #include "qs.h" */
/* #include "rbtree.h" */
/* #include "reader.h" */
/* #include "writer.h" */

// System includes
// ===============
//
// .. code-block:: cpp
//
#ifndef CH_WITHOUT_TLS
#include <openssl/bio.h>
#include <openssl/ssl.h>
#endif

// Declarations
// ============

// .. c:type:: ch_cn_flags_t
//
//    Represents connection flags.
//
//    .. c:member:: CH_CN_SHUTTING_DOWN
//
//       Indicates that the connection is shutting down.
//
//    .. c:member:: CH_CN_WRITE_PENDING
//
//       Indicates that There is a write pending.
//
//    .. c:member:: CH_CN_TLS_HANDSHAKE
//
//      Indicates that a handshake is running.
//
//    .. c:member:: CH_CN_ENCRYPTED
//
//       Indicates whether the connection is encrypted or not.
//
//    .. c:member:: CH_CN_BUF_WTLS_USED
//
//       Indicates if TLS for writing is used.
//
//    .. c:member:: CH_CN_BUF_RTLS_USED
//
//       Indicates if TLS for reading is used.
//
//    .. c:member:: CH_CN_BUF_UV_USED
//
//       Indicates that the connection buffer is currently used by libuv.
//
//    .. c:member:: CH_CN_DO_CLOSE_ACCOUTING
//
//       Indicates that this close should happened during chirp close and
//       therefore has to be accounted for in closeing_tasks.
//
//    .. c:member:: CH_CN_INIT_CLIENT
//
//       The uv tcp client has been initialized.
//
//    .. c:member:: CH_CN_INIT_READER_WRITER
//
//       The reader/writer has been initialized.
//
//    .. c:member:: CH_CN_INIT_SHUTDOWN_TIMEOUT
//
//       The shutdown timeout has been initialized.
//
//    .. c:member:: CH_CN_INIT_CONNECT_TIMEOUT
//
//       The connect timeout has been initialized.
//
//    .. c:member:: CH_CN_INIT_ENCRYPTION
//
//       The encryption has been initialized.
//
//    .. c:member:: CH_CN_INIT_BUFFERS
//
//       The buffers have been initialized.
//
// .. code-block:: cpp

typedef enum {
    CH_CN_SHUTTING_DOWN        = 1 << 0,
    CH_CN_CONNECTED            = 1 << 1,
    CH_CN_WRITE_PENDING        = 1 << 2,
    CH_CN_TLS_HANDSHAKE        = 1 << 3,
    CH_CN_ENCRYPTED            = 1 << 4,
    CH_CN_BUF_WTLS_USED        = 1 << 5,
    CH_CN_BUF_RTLS_USED        = 1 << 6,
    CH_CN_BUF_UV_USED          = 1 << 7,
    CH_CN_DO_CLOSE_ACCOUTING   = 1 << 8,
    CH_CN_STOPPED              = 1 << 9,
    CH_CN_INCOMING             = 1 << 10,
    CH_CN_ENCRYPTED_WRITE      = 1 << 11,
    CH_CN_INIT_CLIENT          = 1 << 12,
    CH_CN_INIT_READER_WRITER   = 1 << 13,
    CH_CN_INIT_CONNECT_TIMEOUT = 1 << 14,
    CH_CN_INIT_ENCRYPTION      = 1 << 15,
    CH_CN_INIT_BUFFERS         = 1 << 16,
    CH_CN_INIT =
            (CH_CN_INIT_CLIENT | CH_CN_INIT_READER_WRITER |
             CH_CN_INIT_ENCRYPTION | CH_CN_INIT_BUFFERS)
} ch_cn_flags_t;

// .. c:type:: ch_resume_state_t
//
//    Defines the state of a reader.
//
//    .. c:member:: void* rest_of_buffer:
//
//       Buffer active before stop. Point to the rest of the data, that aren't
//       handled yet.
//
//    .. c:member:: size_t bytes_to_read:
//
//       Bytes that still needs to be read after a stopping the reader.
//
// .. code-block:: cpp
//
typedef struct ch_resume_state_s {
    void*  rest_of_buffer;
    size_t bytes_to_read;
} ch_resume_state_t;

// .. c:type:: ch_connection_t
//
//    Connection dictionary implemented as red-black tree.
//
//    .. c:member:: uint8_t ip_protocol
//
//       What IP protocol (IPv4 or IPv6) shall be used for connections.
//
//    .. c:member:: uint8_t[16] address
//
//       IPv4/6 address of the sender if the message was received.  IPv4/6
//       address of the recipient if the message is going to be sent.
//
//    .. c:member:: int32_t port
//
//       The port that shall be used for connections.
//
//    .. c:member:: uint8_t[16] remote_identity
//
//       The identity of the remote target. This is used for getting the remote
//       address.
//
//    .. c:member:: uv_tcp_t client
//
//       The TCP handle (TCP stream) of the client, which is used to get the
//       address of the peer connected to the handle.
//
//    .. c:member:: uv_connect_t connect
//
//       Libuv connect handler.
//
//    .. c:member:: uv_buf* buffer_uv
//
//       Pointer to the libuv (data-) buffer data type.
//
//    .. c:member:: uv_buf* buffer_wtls
//
//       Pointer to the libuv buffer data type for writing data over TLS.
//
//    .. c:member:: uv_buf* buffer_rtls
//
//       Pointer to the libuv buffer data type for reading data over TLS.
//
//    .. c:member:: uv_buf_t buffer_uv_uv
//
//       The actual libuv (data-) buffer (using the buffer_uv data type).
//
//    .. c:member:: uv_buf_t buffer_wtls_uv
//
//       The actual libuv buffer for writing data over TLS (using the
//       buffer_wtls data type).
//
//    .. c:member:: unsigned int nbufs
//
//       Number of buffers to be written
//
//    .. c:member:: uv_buf_t* bufs
//
//       Generic libuv (data-) buffer used for writing over a connection.
//
//    .. c:member:: unsigned int bufs_size
//
//       The current size of bufs. Will be resized if needed.
//
//    .. c:member:: unsigned int bufs_index
//
//       The bufs index that is currently writing
//
//    .. c:member:: size_t buffer_size
//
//       The size of libuv (data-) buffers.
//
//    .. c:member:: size_t buffer_tls_size
//
//       The size of the tls buffers.
//
//    .. c:member:: uv_write_cb write_callback
//
//       A callback which will be called after a sucessful write over a
//       connection.
//
//    .. c:member:: size_t write_written
//
//       Holds how many bytes have been written over a connection. This is
//       typically zero at first and gets increased with each partial write.
//
//    .. c:member:: ch_remote_t* remote
//
//       Pointer to the remote of this connection.
//
//    .. c:member:: ch_remote_t* delete_remote
//
//       Remote to be deleted after shutdown.
//
//    .. c:member:: ch_chirp_t* chirp
//
//       Pointer to the chirp object. See: :c:type:`ch_chirp_t`.
//
//    .. c:member:: uv_write_t write_req
//
//       Write request objet, which is used to write data on a handle.
//
//    .. c:member:: uv_timer_t connect_timeout
//
//       Connect timeout. Used to stop connecting if there is no answer or
//       handshake get stalled.
//
//    .. c:member:: int8_t shutdown_tasks
//
//       Counter for tasks that need to be done when shutting down a connection.
//       Tasks may be calling closing-callbacks for example, on a request handle
//       or the shutdown timer handle. This acts as semaphore.
//
//    .. c:member:: uint32_t flags
//
//       Flags indicating the state of a connection, e.g. shutting down, write
//       pending, TLS handshake, whether the connection is encrypted or not and
//       so on, see :c:type:`ch_cn_flags_t`.
//
//    .. c:member:: SSL* ssl
//
//       Pointer to a SSL (data-) structure. This is used when using an
//       encrypted connection over SSL.
//
//    .. c:member:: BIO* bio_ssl
//
//       Pointer to the BIO structure of SSL. BIO is an I/O stream abstraction
//       and essentially OpenSSL's answer to the C library's FILE pointer. This
//       is used to create a connected BIO pair alongside with
//       :c:member:`bio_app` and is only used for the SSL connection.
//
//    .. c:member:: BIO* bio_app
//
//       Pointer to the applications BIO structure. This is used to read and
//       write (partial) data over TLS.
//
//    .. c:member:: int tls_handshake_state
//
//       Holds the current state of the SSL handshake when using an encrypted
//       connection and TLS handshakes. This is used within the protocol, see
//       :c:func:`_ch_pr_do_handshake`.
//
//    .. c:member:: ch_reader_t reader
//
//       Handle to a chirp reader, handles handshakes and reads (buffers) on a
//       connection.
//
//    .. c:member:: ch_writer_t writer
//
//       Handle to a chirp writer, handles sending and writing on a connection.
//
//    .. c:member:: uint64_t timestamp
//
//       Timestamp when the connection was last used. Used to determine
//       garbage-collection.
//
//    .. c:member:: uint32_t release_serial
//
//       The serial of the message currently released.
//
//    .. c:member:: ch_message_t ack_msg
//
//       Buffer used for ack message
//
//    .. c:member:: char color
//
//       rbtree member
//
//    .. c:member:: ch_connection_t* left
//
//       rbtree member
//
//    .. c:member:: ch_connection_t* right
//
//       rbtree member
//
//    .. c:member:: ch_connection_t* next
//
//       stack member
//
// .. code-block:: cpp
//
struct ch_connection_s {
    uint8_t           ip_protocol;
    uint8_t           address[CH_IP_ADDR_SIZE];
    int32_t           port;
    uint8_t           remote_identity[CH_ID_SIZE];
    ch_chirp_t*       chirp;
    ch_remote_t*      remote;
    ch_remote_t*      delete_remote;
    uv_tcp_t          client;
    uv_connect_t      connect;
    ch_buf*           buffer_uv;
    ch_buf*           buffer_wtls;
    ch_buf*           buffer_rtls;
    uv_buf_t          buffer_uv_uv;
    uv_buf_t          buffer_wtls_uv;
    unsigned int      nbufs;
    uv_buf_t*         bufs;
    unsigned int      bufs_size;
    unsigned int      bufs_index;
    size_t            buffer_size;
    size_t            buffer_rtls_size;
    uv_write_cb       write_callback;
    size_t            write_written;
    ch_resume_state_t read_resume;
    ch_resume_state_t tls_resume;
    uv_write_t        write_req;
    uv_timer_t        connect_timeout;
    int8_t            shutdown_tasks;
    uint32_t          flags;
#ifndef CH_WITHOUT_TLS
    SSL* ssl;
    BIO* bio_ssl;
    BIO* bio_app;
#endif
    int              tls_handshake_state;
    ch_reader_t      reader;
    ch_writer_t      writer;
    uint64_t         timestamp;
    uint32_t         release_serial;
    ch_message_t     ack_msg;
    char             color;
    ch_connection_t* parent;
    ch_connection_t* left;
    ch_connection_t* right;
    ch_connection_t* next;
};

// Data Struct Prototypes
// ----------------------
//
// .. code-block:: cpp

#define ch_cn_cmp_m(x, y) rb_pointer_cmp_m(x, y)
rb_bind_decl_m(ch_cn, ch_connection_t) CH_ALLOW_NL;

qs_stack_bind_decl_m(ch_cn_st, ch_connection_t) CH_ALLOW_NL;

// .. c:function::
void
ch_cn_abort_one_message(ch_remote_t* remote, ch_error_t error);
//
//    Abort one message in queue, because connecting failed.
//
//    :param ch_remote_t* remote: Remote failed to connect.
//    :param ch_error_t error: Status returned by connect.

// .. c:function::
void
ch_cn_close_cb(uv_handle_t* handle);
//
//    Called by libuv after closing a connection handle.
//
//    :param uv_handle_t* handle: The libuv handle holding the
//                                connection

// .. c:function::
void
ch_cn_read_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
//
//    Allocates buffers on the connection and reuses it for each subsequent
//    reads. Also allocates the buffer for TLS since it has to be the same
//    size.
//
//    :param uv_handle_t* handle: The libuv handle holding the
//                                connection
//    :param size_t suggested_size: The size of the connection buffer
//                                  in bytes.
//    :param uv_buf_t* buf: Libuv buffer which will hold the
//                          connection

// .. c:function::
ch_error_t
ch_cn_shutdown(ch_connection_t* conn, int reason);
//
//    Shutdown this connection.
//
//    :param ch_connection_t* conn: Connection dictionary holding a
//                                  chirp instance.
//    :param int reason: The error code that caused the shutdown, it will be
//                       reported to the user.
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype: ch_error_t

// .. c:function::
ch_error_t
ch_cn_init(ch_chirp_t* chirp, ch_connection_t* conn, uint8_t flags);
//
//    Initialize a connection. Please do:
//
//    .. code-block:
//
//       memset(conn, 0, sizeof(*conn));
//
//    before calling this. We usually do this, but this is an exception.
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_connection_t* conn: Connection to initialize
//    :param uint8_t flags: Pass CH_CN_ENCRYPTED for a encrypted connection, 0
//                          otherwise
//

#ifndef CH_WITHOUT_TLS
// .. c:function::
ch_error_t
ch_cn_init_enc(ch_chirp_t* chirp, ch_connection_t* conn);
//
//    Initialize encryption.
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_connection_t* conn: Connection to initialize
//
#endif

#ifndef CH_WITHOUT_TLS
// .. c:function::
void
ch_cn_send_if_pending(ch_connection_t* conn);
//
//    Send all pending handshake data from SSL
//
//    :param ch_connection_t* conn: Connection
//
#endif

// .. c:function::
void
ch_cn_write(
        ch_connection_t* conn,
        const uv_buf_t   bufs[],
        unsigned int     nbufs,
        uv_write_cb      callback);
//
//    Send data to remote
//
//    :param ch_connection_t* conn: Connection
//    :param const uv_buf_t: Buffers to send. The bases must stay valid till
//                           the callback is called.
//    :param unsigned int nbufs: Count of buffers to send
//    :param uv_write_cb: Callback when data is written, can be NULL
//
//
// .. code-block:: cpp

#endif // ch_connection_h
// ============
// Chirp header
// ============
//
// Features
// ========
//
// * Fully automatic connection setup
//
// * TLS support
//
//   * Connections to 127.0.0.1 and ::1 aren't encrypted
//   * We support and test with OpenSSL and LibreSSL
//
// * Easy message routing
//
// * Robust
//
//   * No message can be lost without an error (in sync mode)
//
// * Very thin API
//
// * Minimal code-base, all additional features will be implemented as modules
//   in an upper layer
//
// * Fast
//
//   * Up to 240'000 msg/s on a single-connection, in an unrealistic test:
//
//     * Run on loopback
//
//     * Nothing was executed
//
//     * No encryption
//
//     * Asynchronous
//
//     * The test shows that chirp is highly optimized, but if the network
//       delay is bigger star- or mesh-topology can improve throughput.
//
//   * Up to 55'000 msg/s in synchronous mode
//
// .. code-block:: cpp
//
// Structures
// ==========
//
// default is composition where the child is part of the parent
//
// -> means aggregation. It also means the parent structure is not the owner of
// the child structure.
//
// \* means composition with a pointer (can be replaced)
//
// .. code-block:: text
//
//    ch_chirp_t
//        ch_chirp_int_t (pimpl)
//            ch_protocol_t (connecting / accept)
//                remote dictionary (ch_remote_t)
//                old connection dictionary (ch_connection_t)
//            ch_encryption_t (interface to \*ssl)
//            ch_buffer_pool_t
//
//    ch_remote_t (allows replacing connection to remote note)
//        \*ch_connection_t (can be NULL)
//            ch_writer_t
//            ch_reader_t
//        message-queue (-> ch_message_t)
//
//    ch_message_t
//
#ifndef ch_chirp_h
#define ch_chirp_h

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "common.h" */
/* #include "encryption.h" */
/* #include "libchirp.h" */
/* #include "protocol.h" */

// Declarations
// ============

// .. c:type:: ch_chirp_flags_t
//
//    Represents chirps flags.
//
//    .. c:member:: CH_CHIRP_AUTO_STOP
//
//       Stop the loop on closing. This is useful if the loop is only used by
//       chirp.
//
//    .. c:member:: CH_CHIRP_CLOSED
//
//       Chirp is closed.
//
//    .. c:member:: CH_CHIRP_CLOSING
//
//       Chirp is being closed.
//
// .. code-block:: cpp
//
typedef enum {
    CH_CHIRP_AUTO_STOP = 1 << 0,
    CH_CHIRP_CLOSED    = 1 << 1,
    CH_CHIRP_CLOSING   = 1 << 2,
} ch_chirp_flags_t;

// .. c:type:: ch_chirp_int_t
//
//    Chirp object.
//
//    .. c:member:: ch_config_t config
//
//       The current chirp configuration.
//
//    .. c:member:: int closing_tasks
//
//       Counter for the number of tasks when closing a connection (e.g.
//       shutdown). This acts as semaphore.
//
//    .. c:member:: uint8_t flags
//
//       Holds the flags from :c:type:`ch_chirp_flags_t`, hence indicates
//       whether chirp is closing, already closed and if the loop shall be
//       stopped when closing.
//
//    .. c:member:: uv_async_t start
//
//       Asynchronous handler called when chirp is ready.
//
//    .. c:member:: ch_start_cb_t start_cb
//
//       User supplied callback called when chirp is ready.
//
//    .. c:member:: uv_async_t close
//
//       Asynchronous handler to close chirp on the main-loop.
//
//    .. c:member:: uv_signal_t signals[2]
//
//       Libuv handles for managing unix signals. We currently register two
//       handlers, one for SIGINT, one for SIGTERM.
//
//    .. c:member:: uv_prepare_t close_check
//
//       Handle which will run the given callback (close callback, closes chirp
//       when the closing semaphore reaches zero) once per loop iteration.
//
//    .. c:member:: ch_protocol_t protocol
//
//       Reference to protocol object. Provides access to connection and data
//       specific functions.
//
//    .. c:member:: ch_encryption_t encryption
//
//       Reference to encryption object. Is used when a encrypted connection is
//       used.
//
//    .. c:member:: uv_loop_t* loop
//
//       Pointer to the libuv (main) event loop. The event loop is the central
//       part of libuv’s functionality. It takes care of polling for i/o and
//       scheduling callbacks to be run based on different sources of events.
//
//    .. c:member:: uint8_t identity[16]
//
//       Array holding the identity of this chirp configuration.
//
//    .. c:member:: uint16_t public_port
//
//       The public port which this chirp configuration uses for connections.
//
//    .. c:member:: ch_message_t* send_ts_queue
//
//       The send queue for ch_chirp_send_ts
//
//    .. c:member:: uv_async_t send_ts
//
//       Async event for waking up ch_wr_send_ts_cb
//
//    .. c:member:: uv_mutex_t send_ts_queue_lock
//
//       Mutex to lock the send_ts_queue
//
//    .. c:member:: ch_recv_cb_t recv_cb
//
//       Callback when message is received
//
// .. code-block:: cpp
//
struct ch_chirp_int_s {
    ch_config_t   config;
    int           closing_tasks;
    uint8_t       flags;
    uv_async_t    close;
    uv_async_t    start;
    ch_start_cb_t start_cb;
    uv_signal_t   signals[2];
    uv_prepare_t  close_check;
    ch_protocol_t protocol;
#ifndef CH_WITHOUT_TLS
    ch_encryption_t encryption;
#endif
    uv_loop_t*    loop;
    uint8_t       identity[CH_ID_SIZE];
    uint16_t      public_port;
    ch_message_t* send_ts_queue;
    uv_async_t    send_ts;
    uv_mutex_t    send_ts_queue_lock;
    ch_message_t* release_ts_queue;
    uv_async_t    release_ts;
    uv_mutex_t    release_ts_queue_lock;
    ch_recv_cb_t  recv_cb;
    uv_async_t    done;
    ch_done_cb_t  done_cb;
};


// .. c:function::
void
ch_chirp_close_cb(uv_handle_t* handle);
//
//    Reduce closing callback semaphore.
//
//    :param uv_handle_t* handle: A libuv handle containing the chirp object
//

// .. c:function::
void
ch_chirp_finish_message(
        ch_chirp_t*      chirp,
        ch_connection_t* conn,
        ch_message_t*    msg,
        int              status);
//
//    Call the user callback and then check the message queue for further
//    messages and send them. It will finish once the writer is done and the
//    ACK as been received.
//
//    :param ch_chirp_t* writer: Chirp instance
//    :param ch_connection_t* conn: Pointer to connection
//    :param ch_message_t* msg: Pointer to the message
//    :param int status: Error code
//

// .. c:function::
void
ch_chirp_release_ts_cb(uv_async_t* handle);
//
//    Release all messages in the release queue.
//
//    :param uv_async_t* handle: Async handler used to trigger release queue.
//

#endif // ch_chirp_h
// ======
// Buffer
// ======

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "buffer.h" */
/* #include "util.h" */

// Definitions
// ===========
//
// .. c:function::
static inline int
ch_msb32(uint32_t x)
//
//    Get the most significant bit set of a set of bits.
//
//    :param uint32_t x:  The set of bits.
//
//    :return:            the most significant bit set.
//    :rtype:             uint32_t
//
// .. code-block:: cpp
//
{
    static const uint32_t bval[] = {
            0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4};

    uint32_t r = 0;
    if (x & 0xFFFF0000) {
        r += 16 / 1;
        x >>= 16 / 1;
    }
    if (x & 0x0000FF00) {
        r += 16 / 2;
        x >>= 16 / 2;
    }
    if (x & 0x000000F0) {
        r += 16 / 4;
        x >>= 16 / 4;
    }
    return r + bval[x];
}

// .. c:function::
void
ch_bf_free(ch_buffer_pool_t* pool)
//    :noindex:
//
//    See: :c:func:`ch_bf_free`
//
// .. code-block:: cpp
//
{
    pool->refcnt -= 1;
    if (pool->refcnt == 0) {
        ch_free(pool->slots);
        ch_free(pool);
    }
}

// .. c:function::
ch_error_t
ch_bf_init(ch_buffer_pool_t* pool, ch_connection_t* conn, uint8_t max_slots)
//    :noindex:
//
//    See: :c:func:`ch_bf_init`
//
// .. code-block:: cpp
//
{
    int i;
    A(max_slots <= 32, "can't handle more than 32 slots");
    memset(pool, 0, sizeof(*pool));
    pool->conn       = conn;
    pool->refcnt     = 1;
    size_t pool_mem  = max_slots * sizeof(ch_bf_slot_t);
    pool->used_slots = 0;
    pool->max_slots  = max_slots;
    pool->slots      = ch_alloc(pool_mem);
    if (!pool->slots) {
        fprintf(stderr,
                "%s:%d Fatal: Could not allocate memory for buffers. "
                "ch_buffer_pool_t:%p\n",
                __FILE__,
                __LINE__,
                (void*) pool);
        return CH_ENOMEM;
    }
    memset(pool->slots, 0, pool_mem);
    pool->free_slots = 0xFFFFFFFFU;
    pool->free_slots <<= (32 - max_slots);
    for (i = 0; i < max_slots; ++i) {
        pool->slots[i].id   = i;
        pool->slots[i].used = 0;
    }
    return CH_SUCCESS;
}

// .. c:function::
ch_bf_slot_t*
ch_bf_acquire(ch_buffer_pool_t* pool)
//    :noindex:
//
//    See: :c:func:`ch_bf_acquire`
//
// .. code-block:: cpp
//
{
    ch_bf_slot_t* slot_buf;
    if (pool->used_slots < pool->max_slots) {
        int free;
        pool->used_slots += 1;
        free = ch_msb32(pool->free_slots);
        /* Reserve the buffer. */
        pool->free_slots &= ~(1 << (free - 1));
        /* The msb represents the first buffer. So the value is inverted. */
        slot_buf = &pool->slots[32 - free];
        A(slot_buf->used == 0, "Slot already used.");
        slot_buf->used = 1;
        memset(&slot_buf->msg, 0, sizeof(slot_buf->msg));
        slot_buf->msg._slot  = slot_buf->id;
        slot_buf->msg._pool  = pool;
        slot_buf->msg._flags = CH_MSG_HAS_SLOT;
        return slot_buf;
    }
    return NULL;
}

// .. c:function::
void
ch_bf_release(ch_buffer_pool_t* pool, int id)
//    :noindex:
//
//    See: :c:func:`ch_bf_release`
//
// .. code-block:: cpp
//
{
    ch_bf_slot_t* slot_buf = &pool->slots[id];
    A(slot_buf->used == 1, "Double release of slot.");
    A(pool->used_slots > 0, "Buffer pool inconsistent.");
    A(slot_buf->id == id, "Id changed.");
    A(slot_buf->msg._slot == id, "Id changed.");
    int in_pool = pool->free_slots & (1 << (31 - id));
    A(!in_pool, "Buffer already in pool");
    if (in_pool) {
        fprintf(stderr,
                "%s:%d Fatal: Double release of slot. "
                "ch_buffer_pool_t:%p\n",
                __FILE__,
                __LINE__,
                (void*) pool);
        return;
    }
    pool->used_slots -= 1;
    /* Release the buffer. */
    slot_buf->used = 0;
    pool->free_slots |= (1 << (31 - id));
}
// =====
// Chirp
// =====

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "chirp.h" */
/* #include "common.h" */
/* #include "remote.h" */
/* #include "util.h" */

// System includes
// ===============
//
// .. code-block:: cpp
//
#include <openssl/err.h>
#include <signal.h>
#include <time.h>
#ifdef _WIN32
#define ch_access _access
#include <io.h>
#else
#define ch_access access
#include <unistd.h>
#endif

// Declarations
// ============
//
// .. code-block:: cpp

MINMAX_FUNCS(float)

// .. c:var:: uv_mutex_t _ch_chirp_init_lock
//
//    It seems that initializing signals accesses a shared data-structure. We
//    lock during init, just to be sure.
//
// .. code-block:: cpp
//
static uv_mutex_t _ch_chirp_init_lock;

// .. c:var:: int _ch_libchirp_initialized
//
//    Variable to check if libchirp is already initialized.
//
// .. code-block:: cpp
//
static int _ch_libchirp_initialized = 0;

// .. c:var:: ch_config_t ch_config_defaults
//
//    Default config of chirp.
//
// .. code-block:: cpp
//
static ch_config_t _ch_config_defaults = {
        .REUSE_TIME         = 30,
        .TIMEOUT            = 5.0,
        .PORT               = 2998,
        .BACKLOG            = 100,
        .MAX_SLOTS          = 0,
        .SYNCHRONOUS        = 1,
        .DISABLE_SIGNALS    = 0,
        .BUFFER_SIZE        = 0,
        .MAX_MSG_SIZE       = CH_MAX_MSG_SIZE,
        .BIND_V6            = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        .BIND_V4            = {0, 0, 0, 0},
        .IDENTITY           = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        .CERT_CHAIN_PEM     = NULL,
        .DH_PARAMS_PEM      = NULL,
        .DISABLE_ENCRYPTION = 0,
};


// .. c:function::
static void
_ch_chirp_ack_send_cb(ch_chirp_t* chirp, ch_message_t* msg, ch_error_t status);
//
//    Called by chirp once ack_msg is sent.
//
//    :param ch_chirp_t* chirp: Chirp instance
//    :param ch_message_t* msg: Ack message sent

// .. c:function::
static void
_ch_chirp_check_closing_cb(uv_prepare_t* handle);
//
//    Close chirp when the closing semaphore reaches zero.
//
//    :param uv_prepare_t* handle: Prepare handle which will be stopped (and
//                                 thus its callback)
//

// .. c:function::
static void
_ch_chirp_close_async_cb(uv_async_t* handle);
//
//    Internal callback to close chirp. Makes ch_chirp_close_ts thread-safe
//
//    :param uv_async_t* handle: Async handle which is used to closed chirp
//

// .. c:function::
static void
_ch_chirp_closing_down_cb(uv_handle_t* handle);
//
//    Close chirp after the check callback has been closed, calls
//    done-callback in next uv-loop iteration. To make sure that any open
//    requests are handled, before informing the user.
//
//    :param uv_handle_t* handle: Handle just closed
//

// .. c:function::
static void
_ch_chirp_done_cb(uv_async_t* handle);
//
//    The done async-callback calls the user supplied done callback when chirp
//    is finished. Then closes itself, which calls _ch_chirp_stop_cb, which
//    finally frees chirp.
//
//    :param uv_async_t* handle: Async handle
//

// .. c:function::
static void
_ch_chirp_init_signals(ch_chirp_t* chirp);
//
//    Setup signal handlers for chirp. Internally called from ch_chirp_init()
//
//   :param   ch_chirp_t* chrip: Instance of a chirp object

// .. c:function::
static void
_ch_chirp_sig_handler(uv_signal_t*, int);
//
//    Closes all chirp instances on sig int.
//
//    :param uv_signal_t* handle : The libuv signal handler structure
//
//    :param int signo: The signal number, that tells which signal should be
//                      handled.
//

// .. c:function::
static void
_ch_chirp_start_cb(uv_async_t* handle);
//
//    Start callback calls the user supplied done callback.
//
//    :param uv_async_t* handle: Async handler.
//

// .. c:function::
static void
_ch_chirp_stop_cb(uv_handle_t* handle);
//
//    Last close callback when stopping chirp. Frees chirp.
//
//    :param uv_handle_t* handle: handle just closed
//
//
// .. c:function::
static void
_ch_chirp_uninit(ch_chirp_t* chirp, uint16_t uninit);
//
//    Uninitializes resources on failed ch_chirp_init.
//
//    :param ch_chirp_t* chirp: Instance of a chirp object
//    :param uint16_t   uninit: Initialization state
//

// .. c:function::
static ch_error_t
_ch_chirp_verify_cfg(ch_chirp_t* chirp);
//
//    Verifies the configuration.
//
//    :param   ch_chirp_t* chirp: Instance of a chirp object
//
//    :return: A chirp error. see: :c:type:`ch_error_t`
//    :rtype:  ch_error_t

// .. c:var:: extern char* ch_version
//    :noindex:
//
//    Version of chirp.
//
// .. code-block:: cpp
//
CH_EXPORT
char* ch_version = CH_VERSION;

// Definitions
// ===========

// .. c:function::
static void
_ch_chirp_ack_send_cb(ch_chirp_t* chirp, ch_message_t* msg, ch_error_t status)
//    :noindex:
//
//    see: :c:func:`_ch_rd_handshake`
//
// .. code-block:: cpp
//
{
    (void) (msg);
    (void) (status);
    ch_chirp_check_m(chirp);
    if (msg->_release_cb != NULL) {
        ch_chirp_t* rchirp = msg->user_data;
        ch_chirp_check_m(rchirp);
        ch_connection_t* conn = msg->_pool;
        ch_release_cb_t  cb   = msg->_release_cb;
        msg->_release_cb      = NULL;
        cb(rchirp, msg->identity, conn->release_serial);
    }
}

// .. c:function::
static void
_ch_chirp_check_closing_cb(uv_prepare_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_check_closing_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    A(ichirp->closing_tasks > -1, "Closing semaphore dropped below zero");
    L(chirp, "Check closing semaphore (%d)", ichirp->closing_tasks);
    /* In production we allow the semaphore to drop below zero but log it as
     * an error. */
    if (ichirp->closing_tasks < 1) {
        int tmp_err;
        tmp_err = uv_prepare_stop(handle);
        A(tmp_err == CH_SUCCESS, "Could not stop prepare callback");
        (void) (tmp_err);
#ifndef CH_WITHOUT_TLS
        if (!ichirp->config.DISABLE_ENCRYPTION) {
            tmp_err = ch_en_stop(&ichirp->encryption);
            A(tmp_err == CH_SUCCESS, "Could not stop encryption");
        }
#endif
        uv_close((uv_handle_t*) handle, _ch_chirp_closing_down_cb);
    }
    if (ichirp->closing_tasks < 0) {
        E(chirp, "Check closing semaphore dropped blow 0", CH_NO_ARG);
    }
}

// .. c:function::
static void
_ch_chirp_close_async_cb(uv_async_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_close_async_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);
    if (chirp->_ == NULL) {
        E(chirp, "Chirp closing callback called on closed", CH_NO_ARG);
        return;
    }
    ch_chirp_int_t* ichirp = chirp->_;
    if (ichirp->flags & CH_CHIRP_CLOSED) {
        E(chirp, "Chirp closing callback called on closed", CH_NO_ARG);
        return;
    }
    L(chirp, "Chirp closing callback called", CH_NO_ARG);
    int tmp_err;
    tmp_err = ch_pr_stop(&ichirp->protocol);
    A(tmp_err == CH_SUCCESS, "Could not stop protocol");
    (void) (tmp_err);
#ifndef CH_DISABLE_SIGNALS
    if (!ichirp->config.DISABLE_SIGNALS) {
        uv_signal_stop(&ichirp->signals[0]);
        uv_signal_stop(&ichirp->signals[1]);
        uv_close((uv_handle_t*) &ichirp->signals[0], ch_chirp_close_cb);
        uv_close((uv_handle_t*) &ichirp->signals[1], ch_chirp_close_cb);
        ichirp->closing_tasks += 2;
    }
#endif
    uv_close((uv_handle_t*) &ichirp->send_ts, ch_chirp_close_cb);
    uv_close((uv_handle_t*) &ichirp->release_ts, ch_chirp_close_cb);
    uv_close((uv_handle_t*) &ichirp->close, ch_chirp_close_cb);
    ichirp->closing_tasks += 3;
    uv_mutex_destroy(&ichirp->send_ts_queue_lock);
    uv_mutex_destroy(&ichirp->release_ts_queue_lock);
    tmp_err = uv_prepare_init(ichirp->loop, &ichirp->close_check);
    A(tmp_err == CH_SUCCESS, "Could not init prepare callback");
    ichirp->close_check.data = chirp;
    /* We use a semaphore to wait until all callbacks are done:
     * 1. Every time a new callback is scheduled we do
     *    ichirp->closing_tasks += 1
     * 2. Every time a callback is called we do ichirp->closing_tasks -= 1
     * 3. Every uv_loop iteration before it blocks we check
     *    ichirp->closing_tasks == 0
     * -> if we reach 0 all callbacks are done and we continue freeing memory
     * etc. */
    tmp_err =
            uv_prepare_start(&ichirp->close_check, _ch_chirp_check_closing_cb);
    A(tmp_err == CH_SUCCESS, "Could not start prepare callback");
}

// .. c:function::
void
ch_chirp_close_cb(uv_handle_t* handle)
//    :noindex:
//
//    see: :c:func:`ch_chirp_close_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);
    chirp->_->closing_tasks -= 1;
    LC(chirp,
       "Closing semaphore (%d). ",
       "uv_handle_t:%p",
       chirp->_->closing_tasks,
       (void*) handle);
}

// .. c:function::
static void
_ch_chirp_closing_down_cb(uv_handle_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_closing_down_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);
    if (uv_async_send(&chirp->_->done) < 0) {
        E(chirp, "Could not call done callback", CH_NO_ARG);
    }
}
// .. c:function::
static void
_ch_chirp_stop_cb(uv_handle_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_stop_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_int_t* ichirp = handle->data;
    if (ichirp->flags & CH_CHIRP_AUTO_STOP) {
        uv_stop(ichirp->loop);
    }
    ch_free(ichirp);
}

// .. c:function::
static void
_ch_chirp_uninit(ch_chirp_t* chirp, uint16_t uninit)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_uninit`
//
// .. code-block:: cpp
//
{
    /* The existence of this function shows a little technical dept. Classes
     * like chirp/protocol/encryption only know how to close themselves when
     * they are fully initialized. This should be streamlined one day. */
    if (uninit & CH_UNINIT_ASYNC_DONE) {
        ch_chirp_int_t* ichirp   = chirp->_;
        ch_protocol_t*  protocol = &ichirp->protocol;

        if (uninit & CH_UNINIT_ASYNC_SEND_TS) {
            uv_close((uv_handle_t*) &ichirp->send_ts, ch_chirp_close_cb);
            ichirp->closing_tasks += 1;
        }
        if (uninit & CH_UNINIT_ASYNC_RELE_TS) {
            uv_close((uv_handle_t*) &ichirp->release_ts, ch_chirp_close_cb);
            ichirp->closing_tasks += 1;
        }
        if (uninit & CH_UNINIT_ASYNC_CLOSE) {
            uv_close((uv_handle_t*) &ichirp->close, ch_chirp_close_cb);
            ichirp->closing_tasks += 1;
        }
        if (uninit & CH_UNINIT_ASYNC_START) {
            uv_close((uv_handle_t*) &ichirp->start, ch_chirp_close_cb);
            ichirp->closing_tasks += 1;
        }
        if (uninit & CH_UNINIT_SEND_TS_LOCK) {
            uv_mutex_destroy(&ichirp->send_ts_queue_lock);
        }
        if (uninit & CH_UNINIT_RELE_TS_LOCK) {
            uv_mutex_destroy(&ichirp->release_ts_queue_lock);
        }
        if (uninit & CH_UNINIT_SERVERV4) {
            uv_close((uv_handle_t*) &protocol->serverv4, ch_chirp_close_cb);
            ichirp->closing_tasks += 1;
        }
        if (uninit & CH_UNINIT_SERVERV6) {
            uv_close((uv_handle_t*) &protocol->serverv6, ch_chirp_close_cb);
            ichirp->closing_tasks += 1;
        }
        if (uninit & CH_UNINIT_TIMER_GC) {
            uv_timer_stop(&protocol->gc_timeout);
            uv_close((uv_handle_t*) &protocol->gc_timeout, ch_chirp_close_cb);
            ichirp->closing_tasks += 1;
        }
        if (uninit & CH_UNINIT_TIMER_RECON) {
            uv_timer_stop(&protocol->reconnect_timeout);
            uv_close(
                    (uv_handle_t*) &protocol->reconnect_timeout,
                    ch_chirp_close_cb);
            ichirp->closing_tasks += 1;
        }
        if (uninit & CH_UNINIT_SIGNAL) {
            uv_signal_stop(&ichirp->signals[0]);
            uv_signal_stop(&ichirp->signals[1]);
            uv_close((uv_handle_t*) &ichirp->signals[0], ch_chirp_close_cb);
            uv_close((uv_handle_t*) &ichirp->signals[1], ch_chirp_close_cb);
            ichirp->closing_tasks += 2;
        }

        uv_prepare_init(ichirp->loop, &ichirp->close_check);
        ichirp->close_check.data = chirp;
        uv_prepare_start(&ichirp->close_check, _ch_chirp_check_closing_cb);
    } else {
        chirp->_init = 0;
        if (uninit & CH_UNINIT_ICHIRP) {
            ch_free(chirp->_);
        }
    }
    if (uninit & CH_UNINIT_INIT_LOCK) {
        uv_mutex_unlock(&_ch_chirp_init_lock);
    }
}

// .. c:function::
static void
_ch_chirp_done_cb(uv_async_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_done_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    handle->data           = ichirp;
    uv_close((uv_handle_t*) handle, _ch_chirp_stop_cb);
    L(chirp, "Closed.", CH_NO_ARG);
    if (ichirp->flags & CH_CHIRP_AUTO_STOP) {
        LC(chirp,
           "UV-Loop stopped by chirp. ",
           "uv_loop_t:%p",
           (void*) ichirp->loop);
    }
    if (ichirp->done_cb != NULL) {
        ichirp->done_cb(chirp);
    }
}

// .. c:function::
static void
_ch_chirp_init_signals(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_init_signals`
//
// .. code-block:: cpp
//
{
#ifndef CH_DISABLE_SIGNALS
    ch_chirp_int_t* ichirp = chirp->_;
    if (ichirp->config.DISABLE_SIGNALS) {
        return;
    }
    uv_signal_init(ichirp->loop, &ichirp->signals[0]);
    uv_signal_init(ichirp->loop, &ichirp->signals[1]);

    ichirp->signals[0].data = chirp;
    ichirp->signals[1].data = chirp;

    if (uv_signal_start(&ichirp->signals[0], &_ch_chirp_sig_handler, SIGINT)) {
        E(chirp, "Unable to set SIGINT handler", CH_NO_ARG);
        return;
    }

    if (uv_signal_start(&ichirp->signals[1], &_ch_chirp_sig_handler, SIGTERM)) {
        uv_signal_stop(&ichirp->signals[0]);
        uv_close((uv_handle_t*) &ichirp->signals[0], NULL);
        E(chirp, "Unable to set SIGTERM handler", CH_NO_ARG);
    }
#else
    (void) (chirp);
#endif
}

// .. c:function::
static void
_ch_chirp_sig_handler(uv_signal_t* handle, int signo)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_sig_handler`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);

    if (signo != SIGINT && signo != SIGTERM) {
        return;
    }

    ch_chirp_close_ts(chirp);
}

// .. c:function::
static void
_ch_chirp_start_cb(uv_async_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_start_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    uv_close((uv_handle_t*) handle, NULL);
    if (ichirp->start_cb != NULL) {
        ichirp->start_cb(chirp);
    }
}

// .. c:function::
static ch_error_t
_ch_chirp_verify_cfg(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`_ch_chirp_verify_cfg`
//
// .. code-block:: cpp
//
{
    ch_config_t* conf = &chirp->_->config;
#ifndef CH_WITHOUT_TLS
    if (!conf->DISABLE_ENCRYPTION) {
        V(chirp,
          conf->DH_PARAMS_PEM != NULL,
          "Config: DH_PARAMS_PEM must be set.",
          CH_NO_ARG);
        V(chirp,
          conf->CERT_CHAIN_PEM != NULL,
          "Config: CERT_CHAIN_PEM must be set.",
          CH_NO_ARG);
        V(chirp,
          ch_access(conf->CERT_CHAIN_PEM, F_OK) != -1,
          "Config: cert %s does not exist.",
          conf->CERT_CHAIN_PEM);
        V(chirp,
          ch_access(conf->DH_PARAMS_PEM, F_OK) != -1,
          "Config: cert %s does not exist.",
          conf->CERT_CHAIN_PEM);
    }
#endif
    V(chirp,
      conf->PORT > 1024,
      "Config: port must be > 1024. (%d)",
      conf->PORT);
    V(chirp,
      conf->BACKLOG < 128,
      "Config: backlog must be < 128. (%d)",
      conf->BACKLOG);
    V(chirp,
      conf->TIMEOUT <= 1200,
      "Config: timeout must be <= 1200. (%f)",
      conf->TIMEOUT);
    V(chirp,
      conf->TIMEOUT >= 0.1,
      "Config: timeout must be >= 0.1. (%f)",
      conf->TIMEOUT);
    V(chirp,
      conf->REUSE_TIME >= 0.5,
      "Config: resuse time must be => 0.5. (%f)",
      conf->REUSE_TIME);
    V(chirp,
      conf->REUSE_TIME <= 3600,
      "Config: resuse time must be <= 3600. (%f)",
      conf->REUSE_TIME);
    V(chirp,
      conf->TIMEOUT <= conf->REUSE_TIME,
      "Config: timeout must be <= reuse time. (%f, %f)",
      conf->TIMEOUT,
      conf->REUSE_TIME);
    if (conf->SYNCHRONOUS == 1) {
        V(chirp,
          conf->MAX_SLOTS == 1,
          "Config: if synchronous is enabled max slots must be 1.",
          CH_NO_ARG);
    }
    V(chirp,
      conf->MAX_SLOTS <= 32,
      "Config: max slots must be <= 1.",
      CH_NO_ARG);
    V(chirp,
      conf->BUFFER_SIZE >= CH_MIN_BUFFER_SIZE || conf->BUFFER_SIZE == 0,
      "Config: buffer size must be > %d (%u)",
      CH_MIN_BUFFER_SIZE,
      conf->BUFFER_SIZE);
    V(chirp,
      conf->BUFFER_SIZE >= sizeof(ch_message_t) || conf->BUFFER_SIZE == 0,
      "Config: buffer size must be > %lu (%u)",
      (unsigned long) sizeof(ch_message_t),
      conf->BUFFER_SIZE);
    V(chirp,
      conf->BUFFER_SIZE >= CH_SR_HANDSHAKE_SIZE || conf->BUFFER_SIZE == 0,
      "Config: buffer size must be > %lu (%u)",
      (unsigned long) CH_SR_HANDSHAKE_SIZE,
      conf->BUFFER_SIZE);
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_close_ts(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`ch_chirp_close_ts`
//
//    This function is thread-safe.
//
// .. code-block:: cpp
//
{
    char            chirp_closed = 0;
    ch_chirp_int_t* ichirp;
    if (chirp == NULL || chirp->_init != CH_CHIRP_MAGIC) {
        fprintf(stderr,
                "%s:%d Fatal: chirp is not initialzed. ch_chirp_t:%p\n",
                __FILE__,
                __LINE__,
                (void*) chirp);
        return CH_NOT_INITIALIZED;
    }
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    if (chirp->_ != NULL) {
        ichirp = chirp->_;
        if (ichirp->flags & CH_CHIRP_CLOSED) {
            chirp_closed = 1;
        }
    } else {
        chirp_closed = 1;
    }
    if (chirp_closed) {
        fprintf(stderr,
                "%s:%d Fatal: chirp is already closed. ch_chirp_t:%p\n",
                __FILE__,
                __LINE__,
                (void*) chirp);
        return CH_FATAL;
    }
    if (ichirp->flags & CH_CHIRP_CLOSING) {
        E(chirp, "Close already in progress", CH_NO_ARG);
        return CH_IN_PRORESS;
    }
    ichirp->flags |= CH_CHIRP_CLOSING;
    ichirp->close.data = chirp;
    L(chirp, "Closing chirp via callback", CH_NO_ARG);
    if (uv_async_send(&ichirp->close) < 0) {
        E(chirp, "Could not call close callback", CH_NO_ARG);
        return CH_UV_ERROR;
    }
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
void
ch_chirp_config_init(ch_config_t* config)
//    :noindex:
//
//    see: :c:func:`ch_chirp_config_init`
//
// .. code-block:: cpp
//
{
    *config = _ch_config_defaults;
}

// .. c:function::
CH_EXPORT
ch_identity_t
ch_chirp_get_identity(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`ch_chirp_get_identity`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_identity_t id;
    memcpy(id.data, chirp->_->identity, sizeof(id.data));
    return id;
}

// .. c:function::
CH_EXPORT
uv_loop_t*
ch_chirp_get_loop(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`ch_chirp_get_loop`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    return chirp->_->loop;
}

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
        ch_log_cb_t        log_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_init`
//
// .. code-block:: cpp
//
{
    int      tmp_err;
    uint16_t uninit = 0;
    uv_mutex_lock(&_ch_chirp_init_lock);
    uninit |= CH_UNINIT_INIT_LOCK;
    memset(chirp, 0, sizeof(*chirp));
    chirp->_init           = CH_CHIRP_MAGIC;
    chirp->_thread         = uv_thread_self();
    ch_chirp_int_t* ichirp = ch_alloc(sizeof(*ichirp));
    if (!ichirp) {
        fprintf(stderr,
                "%s:%d Fatal: Could not allocate memory for chirp. "
                "ch_chirp_t:%p\n",
                __FILE__,
                __LINE__,
                (void*) chirp);
        uv_mutex_unlock(&_ch_chirp_init_lock);
        return CH_ENOMEM;
    }
    uninit |= CH_UNINIT_ICHIRP;
    memset(ichirp, 0, sizeof(*ichirp));
    ichirp->done_cb         = done_cb;
    ichirp->config          = *config;
    ichirp->public_port     = config->PORT;
    ichirp->loop            = loop;
    ichirp->start_cb        = start_cb;
    ichirp->recv_cb         = recv_cb;
    ch_config_t*   tconf    = &ichirp->config;
    ch_protocol_t* protocol = &ichirp->protocol;
    chirp->_                = ichirp;
    if (log_cb != NULL) {
        ch_chirp_set_log_callback(chirp, log_cb);
    }

    unsigned int i = 0;
    while (i < (sizeof(tconf->IDENTITY) - 1) && tconf->IDENTITY[i] == 0)
        i += 1;
    if (tconf->IDENTITY[i] == 0) {
        ch_random_ints_as_bytes(ichirp->identity, sizeof(ichirp->identity));
    } else {
        *ichirp->identity = *tconf->IDENTITY;
    }

    if (tconf->SYNCHRONOUS) {
        tconf->MAX_SLOTS = 1;
    } else {
        if (tconf->MAX_SLOTS == 0) {
            tconf->MAX_SLOTS = 16;
        }
    }
    tconf->REUSE_TIME = ch_max_float(tconf->REUSE_TIME, tconf->TIMEOUT * 3);

    if (uv_async_init(loop, &ichirp->done, _ch_chirp_done_cb) < 0) {
        E(chirp, "Could not initialize done handler", CH_NO_ARG);
        _ch_chirp_uninit(chirp, uninit);
        /* Small inaccuracy for ease of use. The user can await the done_cb,
         * except if we return CH_ENOMEM */
        return CH_ENOMEM;
    }
    ichirp->done.data = chirp;
    uninit |= CH_UNINIT_ASYNC_DONE;

    tmp_err = _ch_chirp_verify_cfg(chirp);
    if (tmp_err != CH_SUCCESS) {
        _ch_chirp_uninit(chirp, uninit);
        return tmp_err;
    }

    if (uv_async_init(loop, &ichirp->close, _ch_chirp_close_async_cb) < 0) {
        E(chirp, "Could not initialize close callback", CH_NO_ARG);
        _ch_chirp_uninit(chirp, uninit);
        return CH_INIT_FAIL;
    }
    ichirp->close.data = chirp;
    uninit |= CH_UNINIT_ASYNC_CLOSE;
    if (uv_async_init(loop, &ichirp->start, _ch_chirp_start_cb) < 0) {
        E(chirp, "Could not initialize done handler", CH_NO_ARG);
        _ch_chirp_uninit(chirp, uninit);
        return CH_INIT_FAIL;
    }
    ichirp->start.data = chirp;
    uninit |= CH_UNINIT_ASYNC_START;
    if (uv_async_init(loop, &ichirp->send_ts, ch_wr_send_ts_cb) < 0) {
        E(chirp, "Could not initialize send_ts handler", CH_NO_ARG);
        _ch_chirp_uninit(chirp, uninit);
        return CH_INIT_FAIL;
    }
    ichirp->send_ts.data = chirp;
    uninit |= CH_UNINIT_ASYNC_SEND_TS;
    if (uv_mutex_init(&ichirp->send_ts_queue_lock) < 0) {
        E(chirp, "Could not initialize send_ts_lock", CH_NO_ARG);
        _ch_chirp_uninit(chirp, uninit);
        return CH_INIT_FAIL;
    }
    uninit |= CH_UNINIT_SEND_TS_LOCK;
    if (uv_async_init(loop, &ichirp->release_ts, ch_chirp_release_ts_cb) < 0) {
        E(chirp, "Could not initialize release_ts handler", CH_NO_ARG);
        _ch_chirp_uninit(chirp, uninit);
        return CH_INIT_FAIL;
    }
    ichirp->release_ts.data = chirp;
    uninit |= CH_UNINIT_ASYNC_RELE_TS;
    if (uv_mutex_init(&ichirp->release_ts_queue_lock) < 0) {
        E(chirp, "Could not initialize release_ts_lock", CH_NO_ARG);
        _ch_chirp_uninit(chirp, uninit);
        return CH_INIT_FAIL;
    }
    uninit |= CH_UNINIT_RELE_TS_LOCK;

    ch_pr_init(chirp, protocol);
    tmp_err = ch_pr_start(protocol, &uninit);
    if (tmp_err != CH_SUCCESS) {
        E(chirp, "Could not start protocol: %d", tmp_err);
        _ch_chirp_uninit(chirp, uninit);
        return tmp_err;
    }
#ifndef CH_WITHOUT_TLS
    ch_encryption_t* enc = &ichirp->encryption;
    if (!tconf->DISABLE_ENCRYPTION) {
        ch_en_init(chirp, enc);
        tmp_err = ch_en_start(enc);
        if (tmp_err != CH_SUCCESS) {
#ifdef CH_ENABLE_LOGGING
            ERR_print_errors_fp(stderr);
#endif
            E(chirp, "Could not start encryption: %d", tmp_err);
            _ch_chirp_uninit(chirp, uninit);
            return tmp_err;
        }
    }
#endif
#ifdef CH_ENABLE_LOGGING
    char id_str[CH_ID_SIZE * 2 + 1];
    ch_bytes_to_hex(
            ichirp->identity, sizeof(ichirp->identity), id_str, sizeof(id_str));
    LC(chirp,
       "Chirp initialized id: %s. ",
       "uv_loop_t:%p",
       id_str,
       (void*) loop);
#endif
    _ch_chirp_init_signals(chirp);
    uninit |= CH_UNINIT_SIGNAL;
    if (uv_async_send(&ichirp->start) < 0) {
        E(chirp, "Could not call start callback", CH_NO_ARG);
        _ch_chirp_uninit(chirp, uninit);
        return CH_UV_ERROR;
    }
    uv_mutex_unlock(&_ch_chirp_init_lock);
    return CH_SUCCESS;
}

// .. c:function::
void
ch_chirp_finish_message(
        ch_chirp_t* chirp, ch_connection_t* conn, ch_message_t* msg, int status)
//    :noindex:
//
//    see: :c:func:`ch_chirp_finish_message`
//
// .. code-block:: cpp
//
{
    char flags = msg->_flags;
    if (flags & CH_MSG_ACK_RECEIVED && flags & CH_MSG_WRITE_DONE) {
        msg->_flags &= ~(CH_MSG_ACK_RECEIVED | CH_MSG_WRITE_DONE);
#ifdef CH_ENABLE_LOGGING
        {
            char  id[CH_ID_SIZE * 2 + 1];
            char* action = "Success";
            if (status != CH_SUCCESS) {
                action = "Failure:";
            }
            ch_bytes_to_hex(
                    msg->identity, sizeof(msg->identity), id, sizeof(id));
            if (msg->type & CH_MSG_ACK) {
                LC(chirp,
                   "%s: sending ACK message id: %s\n"
                   "                            ",
                   "ch_message_t:%p",
                   action,
                   id,
                   (void*) msg);
            } else if (msg->type & CH_MSG_NOOP) {
                LC(chirp,
                   "%s: sending NOOP\n",
                   "ch_message_t:%p",
                   action,
                   (void*) msg);
            } else {
                LC(chirp,
                   "%s: finishing message id: %s\n"
                   "                            ",
                   "ch_message_t:%p",
                   action,
                   id,
                   (void*) msg);
            }
        }
#else
        (void) (chirp);
#endif
        uv_timer_stop(&conn->writer.send_timeout);
        msg->_flags &= ~CH_MSG_USED;
        if (msg->_send_cb != NULL) {
            /* The user may free the message in the cb */
            ch_send_cb_t cb = msg->_send_cb;
            msg->_send_cb   = NULL;
            cb(chirp, msg, status);
        }
    }
    if (conn->remote != NULL) {
        ch_wr_process_queues(conn->remote);
    } else {
        A(conn->flags & CH_CN_SHUTTING_DOWN, "Expected shutdown");
        /* Late write callback after shutdown. These are perfectly valid, since
         * we clear the remote early to improve consistency. We have to lookup
         * the remote. */
        ch_remote_t  key;
        ch_remote_t* remote = NULL;
        ch_rm_init_from_conn(chirp, &key, conn, 1);
        if (ch_rm_find(chirp->_->protocol.remotes, &key, &remote) ==
            CH_SUCCESS) {
            ch_wr_process_queues(remote);
        }
    }
}

CH_EXPORT
void
ch_chirp_release_msg_slot(
        ch_chirp_t* rchirp, ch_message_t* msg, ch_release_cb_t release_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_release_msg_slot`
//
// .. code-block:: cpp
//
{
    ch_buffer_pool_t* pool = msg->_pool;
    ch_connection_t*  conn = pool->conn;
    if (!(msg->_flags & CH_MSG_HAS_SLOT)) {
        fprintf(stderr,
                "%s:%d Fatal: Message does not have a slot. "
                "ch_buffer_pool_t:%p\n",
                __FILE__,
                __LINE__,
                (void*) pool);
        return;
    }
    int call_cb = 1;
    /* If the connection does not exist, it is already shutdown. The user may
     * release a message after a connection has been shutdown. We use reference
     * counting in the buffer pool to delay ch_free of the pool. */
    if (conn && !(conn->flags & CH_CN_SHUTTING_DOWN)) {
        ch_chirp_t* chirp = conn->chirp;
        A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
        if (msg->_flags & CH_MSG_SEND_ACK) {
            msg->_flags &= ~CH_MSG_SEND_ACK;
            /* Send the ack to the connection, in case the user changed the
             * message for his need, which is absolutely ok, and valid use
             * case. */
            ch_message_t* ack_msg = &conn->ack_msg;
            memcpy(ack_msg->identity, msg->identity, CH_ID_SIZE);
            ack_msg->user_data = rchirp;
            A(ack_msg->_release_cb == NULL, "ack_msg in use");
            ack_msg->_release_cb = release_cb;
            conn->release_serial = msg->serial;
            call_cb              = 0;
            ch_wr_send(chirp, ack_msg, _ch_chirp_ack_send_cb);
        }
    }
    if (msg->_flags & CH_MSG_FREE_DATA) {
        ch_free(msg->data);
    }
    if (msg->_flags & CH_MSG_FREE_HEADER) {
        ch_free(msg->header);
    }
    if (call_cb) {
        if (release_cb != NULL) {
            release_cb(rchirp, msg->identity, msg->serial);
        }
    }
    int pool_is_empty = ch_bf_is_exhausted(pool);
    ch_bf_release(pool, msg->_slot);
    /* Decrement refcnt and free if zero */
    ch_bf_free(pool);
    if (pool_is_empty && conn) {
        ch_pr_restart_stream(conn);
    }
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_release_msg_slot_ts(
        ch_chirp_t* rchirp, ch_message_t* msg, ch_release_cb_t release_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_release_msg_slot_ts`
//
// .. code-block:: cpp
//
{
    A(rchirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    A(msg->_release_cb == NULL, "Message already released");
    msg->_release_cb       = release_cb;
    ch_chirp_int_t* ichirp = rchirp->_;
    uv_mutex_lock(&ichirp->release_ts_queue_lock);
    ch_msg_enqueue(&ichirp->release_ts_queue, msg);
    uv_mutex_unlock(&ichirp->release_ts_queue_lock);
    if (uv_async_send(&ichirp->release_ts) < 0) {
        E(rchirp, "Could not call release_ts callback", CH_NO_ARG);
        return CH_UV_ERROR;
    }
    return CH_SUCCESS;
}

// .. c:function::
void
ch_chirp_release_ts_cb(uv_async_t* handle)
//    :noindex:
//
//    see: :c:func:`ch_chirp_release_ts_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    uv_mutex_lock(&ichirp->release_ts_queue_lock);
    ch_message_t* cur;
    ch_msg_dequeue(&ichirp->release_ts_queue, &cur);
    while (cur != NULL) {
        uv_mutex_unlock(&ichirp->release_ts_queue_lock);
        ch_chirp_release_msg_slot(chirp, cur, cur->_release_cb);
        uv_mutex_lock(&ichirp->release_ts_queue_lock);
        ch_msg_dequeue(&ichirp->release_ts_queue, &cur);
    }
    uv_mutex_unlock(&ichirp->release_ts_queue_lock);
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_run(
        const ch_config_t* config,
        ch_chirp_t**       chirp_out,
        ch_recv_cb_t       recv_cb,
        ch_start_cb_t      start_cb,
        ch_done_cb_t       done_cb,
        ch_log_cb_t        log_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_run`
//
// .. code-block:: cpp
//
{
    ch_chirp_t chirp;
    uv_loop_t  loop;
    ch_error_t tmp_err;
    if (chirp_out == NULL) {
        return CH_NOT_INITIALIZED;
    }
    *chirp_out = NULL;

    tmp_err    = ch_loop_init(&loop);
    chirp._log = NULL; /* Bootstrap order problem. E checks _log but
                        * ch_chirp_init() will initialize it. */
    if (tmp_err != CH_SUCCESS) {
        EC((&chirp),
           "Could not init loop: %d. ",
           "uv_loop_t:%p",
           tmp_err,
           (void*) &loop);
        return CH_INIT_FAIL;
    }
    tmp_err = ch_chirp_init(
            &chirp, config, &loop, recv_cb, start_cb, done_cb, log_cb);
    if (tmp_err != CH_SUCCESS) {
        EC((&chirp),
           "Could not init chirp: %d. ",
           "ch_chirp_t:%p",
           tmp_err,
           (void*) &chirp);
        return tmp_err;
    }
    chirp._->flags |= CH_CHIRP_AUTO_STOP;
    LC((&chirp), "UV-Loop run by chirp. ", "uv_loop_t:%p", (void*) &loop);
    /* This works and is not TOO bad because the function blocks. */
    // cppcheck-suppress autoVariables
    *chirp_out = &chirp;
    tmp_err    = ch_run(&loop);
    *chirp_out = NULL;
    if (tmp_err != 0) {
        fprintf(stderr,
                "uv_run returned with error: %d. uv_loop_t:%p",
                tmp_err,
                (void*) &loop);
        return tmp_err;
    }
    if (ch_loop_close(&loop)) {
        return CH_UV_ERROR;
    }
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
void
ch_chirp_set_auto_stop_loop(ch_chirp_t* chirp)
//    :noindex:
//
//    see: :c:func:`ch_chirp_set_auto_stop_loop`
//
//    This function is thread-safe
//
// .. code-block:: cpp
//
{
    ch_chirp_check_m(chirp);
    chirp->_->flags |= CH_CHIRP_AUTO_STOP;
}

// .. c:function::
CH_EXPORT
void
ch_chirp_set_log_callback(ch_chirp_t* chirp, ch_log_cb_t log_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_set_log_callback`
//
// .. code-block:: cpp
//
{
    ch_chirp_check_m(chirp);
    chirp->_log = log_cb;
}

// .. c:function::
CH_EXPORT
void
ch_chirp_set_public_port(ch_chirp_t* chirp, uint16_t port)
//    :noindex:
//
//    see: :c:func:`ch_chirp_set_public_port`
//
// .. code-block:: cpp
//
{
    ch_chirp_check_m(chirp);
    chirp->_->public_port = port;
}

// .. c:function::
CH_EXPORT
void
ch_chirp_set_recv_callback(ch_chirp_t* chirp, ch_recv_cb_t recv_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_set_recv_callback`
//
// .. code-block:: cpp
//
{
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    ichirp->recv_cb        = recv_cb;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_libchirp_cleanup(void)
//    :noindex:
//
//    see: :c:func:`ch_libchirp_cleanup`
//
// .. code-block:: cpp
//
{
    A(_ch_libchirp_initialized, "Libchirp is not initialized");
    if (!_ch_libchirp_initialized) {
        fprintf(stderr,
                "%s:%d Fatal: Libchirp is not initialized.\n",
                __FILE__,
                __LINE__);
        return CH_VALUE_ERROR;
    }
    _ch_libchirp_initialized = 0;
    uv_mutex_destroy(&_ch_chirp_init_lock);
#ifndef CH_WITHOUT_TLS
    ch_error_t ret = ch_en_tls_cleanup();
#else
    ch_error_t ret = CH_SUCCESS;
#endif
#ifdef CH_ENABLE_ASSERTS
    ch_at_cleanup();
#endif
    return ret;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_libchirp_init(void)
//    :noindex:
//
//    see: :c:func:`ch_libchirp_init`
//
// .. code-block:: cpp
//
{
    A(!_ch_libchirp_initialized, "Libchirp is already initialized");
    if (_ch_libchirp_initialized) {
        fprintf(stderr,
                "%s:%d Fatal: Libchirp is already initialized.\n",
                __FILE__,
                __LINE__);
        return CH_VALUE_ERROR;
    }
    srand(((unsigned int) time(NULL)) | ((unsigned int) uv_hrtime()));
    _ch_libchirp_initialized = 1;
    if (uv_mutex_init(&_ch_chirp_init_lock) < 0) {
        return CH_INIT_FAIL;
    }
#ifdef CH_ENABLE_ASSERTS
    ch_at_init();
#endif
#ifndef CH_WITHOUT_TLS
    return ch_en_tls_init();
#else
    return CH_SUCCESS;
#endif
}

// .. c:function::
CH_EXPORT
int
ch_loop_close(uv_loop_t* loop)
//    :noindex:
//
//    see: :c:func:`ch_loop_close`
//
// .. code-block:: cpp
//
{
    int tmp_err;
    tmp_err = uv_loop_close(loop);
#ifdef CH_ENABLE_LOGGING
    if (tmp_err != CH_SUCCESS) {
        fprintf(stderr,
                "%s:%d WARNING: Closing loop exitcode:%d. uv_loop_t:%p\n",
                __FILE__,
                __LINE__,
                tmp_err,
                (void*) loop);
    }
#endif
    return tmp_err;
}

// .. c:function::
CH_EXPORT
int
ch_loop_init(uv_loop_t* loop)
//    :noindex:
//
//    see: :c:func:`ch_loop_init`
//
// .. code-block:: cpp
//
{
    int tmp_err;
    uv_mutex_lock(&_ch_chirp_init_lock);
    tmp_err = uv_loop_init(loop);
    uv_mutex_unlock(&_ch_chirp_init_lock);
    return tmp_err;
}

// .. c:function::
CH_EXPORT
int
ch_run(uv_loop_t* loop)
//    :noindex:
//
//    see: :c:func:`ch_run`
//
// .. code-block:: cpp
//
{
    int tmp_err = uv_run(loop, UV_RUN_DEFAULT);
    if (tmp_err != 0) {
        /* uv_stop() was called and there are still active handles or requests.
         * This is clearly a bug in chirp or user code, we try to recover with
         * a warning. */
        fprintf(stderr, "WARNING: Cannot close all uv-handles/requests.\n");
        tmp_err = uv_run(loop, UV_RUN_NOWAIT);
        /* Now we have serious a problem */
        if (tmp_err != 0) {
            fprintf(stderr, "FATAL: Cannot close all uv-handles/requests.\n");
        }
    }
    return tmp_err;
}
// ==========
// Connection
// ==========
//

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "connection.h" */
/* #include "chirp.h" */
/* #include "common.h" */
/* #include "remote.h" */
/* #include "util.h" */

// System includes
// ===============
//
// .. code-block:: cpp
//
#ifndef CH_WITHOUT_TLS
#include <openssl/err.h>
#endif

// Data Struct Prototypes
// ======================
//
// .. code-block:: cpp

rb_bind_impl_m(ch_cn, ch_connection_t) CH_ALLOW_NL;

qs_stack_bind_impl_m(ch_cn_st, ch_connection_t) CH_ALLOW_NL;

// Declarations
// ============

// .. c:function::
static ch_error_t
_ch_cn_allocate_buffers(ch_connection_t* conn);
//
//    Allocate the connections communication buffers.
//
//    :param ch_connection_t* conn: Connection
//

// .. c:function::
static void
_ch_cn_closing(ch_connection_t* conn);
//
//    Called by ch_cn_shutdown to enter the closing stage.
//
//    :param ch_connection_t: Connection to close
//

#ifndef CH_WITHOUT_TLS
// .. c:function::
static void
_ch_cn_partial_write(ch_connection_t* conn);
//
//    Called by libuv when pending data has been sent.
//
//    :param ch_connection_t* conn: Connection
//
#endif

#ifndef CH_WITHOUT_TLS
// .. c:function::
static void
_ch_cn_send_pending_cb(uv_write_t* req, int status);
//
//    Called during handshake by libuv when pending data is sent.
//
//    :param uv_write_t* req: Write request type, holding the
//                            connection handle
//    :param int status: Send status
//
#endif

#ifndef CH_WITHOUT_TLS
// .. c:function::
static void
_ch_cn_write_cb(uv_write_t* req, int status);
//
//    Callback used for ch_cn_write.
//
//    :param uv_write_t* req: Write request
//    :param int status: Write status
//
#endif

// Definitions
// ===========
//
// .. code-block:: cpp

MINMAX_FUNCS(size_t)

// .. c:function::
void
ch_cn_abort_one_message(ch_remote_t* remote, ch_error_t error)
//    :noindex:
//
//    see: :c:func:`ch_cn_abort_one_message`
//
// .. code-block:: cpp
//
{
    ch_message_t* msg = NULL;
    if (remote->cntl_msg_queue != NULL) {
        ch_msg_dequeue(&remote->cntl_msg_queue, &msg);
    } else if (remote->msg_queue != NULL) {
        ch_msg_dequeue(&remote->msg_queue, &msg);
    }
    if (msg != NULL) {
#ifdef CH_ENABLE_LOGGING
        {
            char id[CH_ID_SIZE * 2 + 1];
            ch_bytes_to_hex(
                    msg->identity, sizeof(msg->identity), id, sizeof(id));
            if (msg->type & CH_MSG_ACK) {
                LC(remote->chirp,
                   "Abort message on queue id: %s\n"
                   "                             "
                   "ch_message_t:%p",
                   id,
                   (void*) msg);
            }
        }
#endif
        ch_send_cb_t cb = msg->_send_cb;
        if (cb != NULL) {
            msg->_send_cb = NULL;
            cb(remote->chirp, msg, error);
        }
    }
}

// .. c:function::
static ch_error_t
_ch_cn_allocate_buffers(ch_connection_t* conn)
//    :noindex:
//
//    See: :c:func:`_ch_cn_allocate_buffers`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    size_t          size   = ichirp->config.BUFFER_SIZE;
    if (size == 0) {
        size = CH_BUFFER_SIZE;
    }
    conn->buffer_uv   = ch_alloc(size);
    conn->buffer_size = size;
    if (conn->flags & CH_CN_ENCRYPTED) {
        conn->buffer_wtls = ch_alloc(size);
        size              = ch_min_size_t(size, CH_ENC_BUFFER_SIZE);
        conn->buffer_rtls = ch_alloc(size);
    }
    int alloc_nok = 0;
    if (conn->flags & CH_CN_ENCRYPTED) {
        alloc_nok =
                !(conn->buffer_uv && conn->buffer_wtls && conn->buffer_rtls);
    } else {
        alloc_nok = !conn->buffer_uv;
    }
    if (alloc_nok) {
        EC(chirp,
           "Could not allocate memory for libuv and tls. ",
           "ch_connection_t:%p",
           (void*) conn);
        return CH_ENOMEM;
    }
    conn->buffer_rtls_size = size;
    conn->buffer_uv_uv     = uv_buf_init(conn->buffer_uv, conn->buffer_size);
    conn->buffer_wtls_uv   = uv_buf_init(conn->buffer_wtls, conn->buffer_size);
    conn->bufs             = ch_alloc(sizeof(uv_buf_t) * 3);
    conn->bufs_size        = 3;
    conn->flags |= CH_CN_INIT_BUFFERS;
    A((conn->flags & CH_CN_INIT) == CH_CN_INIT,
      "Connection not fully initialized");
    return CH_SUCCESS;
}

// .. c:function::
static void
_ch_cn_closing(ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`_ch_cn_closing`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    LC(chirp, "Shutdown callback called. ", "ch_connection_t:%p", (void*) conn);
    conn->flags &= ~CH_CN_CONNECTED;
    uv_handle_t* handle = (uv_handle_t*) &conn->client;
    if (uv_is_closing(handle)) {
        EC(chirp,
           "Connection already closing on closing. ",
           "ch_connection_t:%p",
           (void*) conn);
    } else {
        uv_read_stop((uv_stream_t*) &conn->client);
        if (conn->flags & CH_CN_INIT_READER_WRITER) {
            ch_wr_free(&conn->writer);
            ch_rd_free(&conn->reader);
            conn->flags &= ~CH_CN_INIT_READER_WRITER;
        }
        if (conn->flags & CH_CN_INIT_CLIENT) {
            uv_close(handle, ch_cn_close_cb);
            conn->shutdown_tasks += 1;
            conn->flags &= ~CH_CN_INIT_CLIENT;
        }
        if (conn->flags & CH_CN_INIT_CONNECT_TIMEOUT) {
            uv_close((uv_handle_t*) &conn->connect_timeout, ch_cn_close_cb);
            conn->flags &= ~CH_CN_INIT_CONNECT_TIMEOUT;
            conn->shutdown_tasks += 1;
        }
        LC(chirp,
           "Closing connection after shutdown. ",
           "ch_connection_t:%p",
           (void*) conn);
    }
}

#ifndef CH_WITHOUT_TLS
// .. c:function::
static void
_ch_cn_partial_write(ch_connection_t* conn)
//    :noindex:
//
//    See: :c:func:`_ch_cn_partial_write`
//
// .. code-block:: cpp
//
{
    size_t      bytes_encrypted = 0;
    size_t      bytes_read      = 0;
    ch_chirp_t* chirp           = conn->chirp;
    ch_chirp_check_m(chirp);
    A(!(conn->flags & CH_CN_BUF_WTLS_USED), "The wtls buffer is still used");
    A(!(conn->flags & CH_CN_WRITE_PENDING), "Another uv write is pending");
#ifdef CH_ENABLE_ASSERTS
    conn->flags |= CH_CN_WRITE_PENDING;
    conn->flags |= CH_CN_BUF_WTLS_USED;
#endif
    for (;;) {
        /* Read all data pending in BIO */
        int can_read_more = 1;
        int pending       = BIO_pending(conn->bio_app);
        while (pending && can_read_more) {
            ssize_t read = BIO_read(
                    conn->bio_app,
                    conn->buffer_wtls + bytes_read,
                    conn->buffer_size - bytes_read);
            A(read > 0, "BIO_read failure unexpected");
            if (read < 1) {
                EC(chirp,
                   "SSL error reading from BIO, shutting down connection. ",
                   "ch_connection_t:%p",
                   (void*) conn);
                ch_cn_shutdown(conn, CH_TLS_ERROR);
                return;
            }
            bytes_read += read;
            can_read_more = bytes_read < conn->buffer_size;

            pending = BIO_pending(conn->bio_app);
        }
        if (!can_read_more) {
            break;
        }
        uv_buf_t* buf      = &conn->bufs[conn->bufs_index];
        int can_write_more = (bytes_encrypted + conn->write_written) < buf->len;
        if (!can_write_more) {
            /* Switch to next buffer if available or break */
            int changed = 0;
            while (conn->bufs_index < (conn->nbufs - 1)) {
                conn->bufs_index += 1;
                changed = 1;
                if (conn->bufs[conn->bufs_index].len != 0) {
                    break;
                }
            }
            if (conn->bufs[conn->bufs_index].len == 0 || !changed) {
                break;
            }
            conn->write_written = 0;
            bytes_encrypted     = 0;
            can_write_more      = 1;
        }
        buf = &conn->bufs[conn->bufs_index];
        /* Write data into BIO */
        if (can_read_more && can_write_more) {
            int tmp_err = SSL_write(
                    conn->ssl,
                    buf->base + bytes_encrypted + conn->write_written,
                    buf->len - bytes_encrypted - conn->write_written);
            bytes_encrypted += tmp_err;
            A(tmp_err > -1, "SSL_write failure unexpected");
            if (tmp_err < 0) {
                EC(chirp,
                   "SSL error writing to BIO, shutting down connection. ",
                   "ch_connection_t:%p",
                   (void*) conn);
                ch_cn_shutdown(conn, CH_TLS_ERROR);
                return;
            }
        }
    }
    conn->buffer_wtls_uv.len = bytes_read;
    uv_write(
            &conn->write_req,
            (uv_stream_t*) &conn->client,
            &conn->buffer_wtls_uv,
            1,
            _ch_cn_write_cb);
    LC(chirp,
       "Called uv_write with %d bytes. ",
       "ch_connection_t:%p",
       (int) bytes_read,
       (void*) conn);
    conn->write_written += bytes_encrypted;
}
#endif

#ifndef CH_WITHOUT_TLS
// .. c:function::
static void
_ch_cn_send_pending_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_cn_send_pending_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn  = req->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
#ifdef CH_ENABLE_ASSERTS
    conn->flags &= ~CH_CN_WRITE_PENDING;
    conn->flags &= ~CH_CN_BUF_WTLS_USED;
#endif
    if (status < 0) {
        LC(chirp,
           "Sending pending data failed. ",
           "ch_connection_t:%p",
           (void*) conn);
        ch_cn_shutdown(conn, CH_WRITE_ERROR);
        return;
    }
    if (conn->flags & CH_CN_SHUTTING_DOWN) {
        LC(chirp,
           "Write shutdown bytes to connection successful. ",
           "ch_connection_t:%p",
           (void*) conn);
    } else {
        LC(chirp,
           "Write handshake bytes to connection successful. ",
           "ch_connection_t:%p",
           (void*) conn);
    }
    ch_cn_send_if_pending(conn);
}
#endif

#ifndef CH_WITHOUT_TLS
// .. c:function::
static void
_ch_cn_write_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_cn_write_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn  = req->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
#ifdef CH_ENABLE_ASSERTS
    conn->flags &= ~CH_CN_WRITE_PENDING;
    conn->flags &= ~CH_CN_BUF_WTLS_USED;
#endif
    if (status < 0) {
        LC(chirp,
           "Sending pending data failed. ",
           "ch_connection_t:%p",
           (void*) conn);
        uv_write_cb cb       = conn->write_callback;
        conn->write_callback = NULL;
        cb(req, CH_WRITE_ERROR);
        ch_cn_shutdown(conn, CH_WRITE_ERROR);
        return;
    }
    /* Check if we can write data */
    uv_buf_t* buf     = &conn->bufs[conn->bufs_index];
    int       pending = BIO_pending(conn->bio_app);
    if (buf->len > conn->write_written || pending) {
        _ch_cn_partial_write(conn);
        LC(chirp,
           "Partially encrypted %d of %d bytes. ",
           "ch_connection_t:%p",
           (int) conn->write_written,
           (int) buf->len,
           (void*) conn);
    } else {
        A(pending == 0, "Unexpected pending data on TLS write");
        LC(chirp,
           "Completely sent %d bytes (unenc). ",
           "ch_connection_t:%p",
           (int) conn->write_written,
           (void*) conn);
        conn->write_written = 0;
#ifdef CH_ENABLE_ASSERTS
        conn->flags &= ~CH_CN_ENCRYPTED_WRITE;
#endif
        if (conn->write_callback != NULL) {
            uv_write_cb cb       = conn->write_callback;
            conn->write_callback = NULL;
            cb(req, status);
        }
    }
}
#endif

// .. c:function::
void
ch_cn_close_cb(uv_handle_t* handle)
//    :noindex:
//
//    see: :c:func:`ch_cn_close_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn  = handle->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    conn->shutdown_tasks -= 1;
    A(conn->shutdown_tasks > -1, "Shutdown semaphore dropped below zero");
    LC(chirp,
       "Shutdown semaphore (%d). ",
       "ch_connection_t:%p",
       conn->shutdown_tasks,
       (void*) conn);
    /* In production we allow the semaphore to drop below 0, but we log an
     * error. */
    if (conn->shutdown_tasks < 0) {
        E(chirp,
          "Shutdown semaphore dropped blow 0. ch_connection_t: %p",
          conn);
    }
    if (conn->shutdown_tasks < 1) {
        if (conn->flags & CH_CN_DO_CLOSE_ACCOUTING) {
            ichirp->closing_tasks -= 1;
            LC(chirp,
               "Closing semaphore (%d). ",
               "uv_handle_t:%p",
               ichirp->closing_tasks,
               (void*) handle);
        }
        if (conn->flags & CH_CN_INIT_BUFFERS) {
            A(conn->buffer_uv, "Initialized buffers inconsistent");
            ch_free(conn->bufs);
            ch_free(conn->buffer_uv);
            if (conn->flags & CH_CN_ENCRYPTED) {
                A(conn->buffer_wtls, "Initialized buffers inconsistent");
                A(conn->buffer_rtls, "Initialized buffers inconsistent");
                ch_free(conn->buffer_wtls);
                ch_free(conn->buffer_rtls);
            }
            conn->flags &= ~CH_CN_INIT_BUFFERS;
        }
#ifndef CH_WITHOUT_TLS
        if (conn->flags & CH_CN_ENCRYPTED) {
            /* The doc says this frees conn->bio_ssl I tested it. let's
             * hope they never change that. */
            if (conn->flags & CH_CN_INIT_ENCRYPTION) {
                A(conn->ssl, "Initialized ssl handles inconsistent");
                A(conn->bio_app, "Initialized ssl handles inconsistent");
                SSL_free(conn->ssl);
                BIO_free(conn->bio_app);
            }
        }
#endif
        /* Since we define a unencrypted connection as CH_CN_INIT_ENCRYPTION. */
        conn->flags &= ~CH_CN_INIT_ENCRYPTION;
        A(!(conn->flags & CH_CN_INIT),
          "Connection resources haven't been freed completely");
        A(!(conn->flags & CH_CN_CONNECTED),
          "Connection not properly disconnected");
        ch_remote_t* remote = conn->delete_remote;
        if (remote != NULL) {
            LC(chirp,
               "Deleted ch_remote_t: %p. ",
               "ch_connection_t:%p",
               (void*) remote,
               (void*) conn);
            ch_rm_free(remote);
        }
        ch_free(conn);
        LC(chirp,
           "Closed connection, closing semaphore (%d). ",
           "ch_connection_t:%p",
           chirp->_->closing_tasks,
           (void*) conn);
    }
}

// .. c:function::
ch_error_t
ch_cn_init(ch_chirp_t* chirp, ch_connection_t* conn, uint8_t flags)
//    :noindex:
//
//    see: :c:func:`ch_cn_init`
//
// .. code-block:: cpp
//
{
    int tmp_err;

    ch_chirp_int_t* ichirp = chirp->_;
    conn->chirp            = chirp;
    conn->write_req.data   = conn;
    conn->flags |= flags;
    conn->timestamp = uv_now(ichirp->loop);
    tmp_err         = ch_rd_init(&conn->reader, conn, ichirp);
    if (tmp_err != CH_SUCCESS) {
        return tmp_err;
    }
    tmp_err = ch_wr_init(&conn->writer, conn);
    if (tmp_err != CH_SUCCESS) {
        return tmp_err;
    }
    conn->flags |= CH_CN_INIT_READER_WRITER;
#ifndef CH_WITHOUT_TLS
    if (conn->flags & CH_CN_ENCRYPTED) {
        tmp_err = ch_cn_init_enc(chirp, conn);
    }
#endif
    if (tmp_err != CH_SUCCESS) {
        return tmp_err;
    }
    /* An unencrypted connection also has CH_CN_INIT_ENCRYPTION */
    conn->flags |= CH_CN_INIT_ENCRYPTION;
    return _ch_cn_allocate_buffers(conn);
}

#ifndef CH_WITHOUT_TLS
// .. c:function::
ch_error_t
ch_cn_init_enc(ch_chirp_t* chirp, ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_cn_init_enc`
//
// .. code-block:: cpp
//
{
    ch_chirp_int_t* ichirp = chirp->_;
    conn->ssl              = SSL_new(ichirp->encryption.ssl_ctx);
    if (conn->ssl == NULL) {
#ifdef CH_ENABLE_LOGGING
        ERR_print_errors_fp(stderr);
#endif
        EC(chirp, "Could not create SSL. ", "ch_connection_t:%p", (void*) conn);
        return CH_TLS_ERROR;
    }
    if (BIO_new_bio_pair(&(conn->bio_ssl), 0, &(conn->bio_app), 0) != 1) {
#ifdef CH_ENABLE_LOGGING
        ERR_print_errors_fp(stderr);
#endif
        EC(chirp,
           "Could not create BIO pair. ",
           "ch_connection_t:%p",
           (void*) conn);
        SSL_free(conn->ssl);
        return CH_TLS_ERROR;
    }
    SSL_set_bio(conn->ssl, conn->bio_ssl, conn->bio_ssl);
#ifdef CH_CN_PRINT_CIPHERS
    STACK_OF(SSL_CIPHER)* ciphers = SSL_get_ciphers(conn->ssl);
    while (sk_SSL_CIPHER_num(ciphers) > 0)
        fprintf(stderr,
                "%s\n",
                SSL_CIPHER_get_name(sk_SSL_CIPHER_pop(ciphers)));
    sk_SSL_CIPHER_free(ciphers);

#endif
    LC(chirp, "SSL context created. ", "ch_connection_t:%p", (void*) conn);
    return CH_SUCCESS;
}
#endif

// .. c:function::
void
ch_cn_read_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
//    :noindex:
//
//    see: :c:func:`ch_cn_read_alloc_cb`
//
// .. code-block:: cpp
//
{
    /* That whole suggested size concept doesn't work, we have to allocated
     * consistent buffers. */
    (void) (suggested_size);
    ch_connection_t* conn  = handle->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    A(!(conn->flags & CH_CN_BUF_UV_USED), "UV buffer still used");
#ifdef CH_ENABLE_ASSERTS
    conn->flags |= CH_CN_BUF_UV_USED;
#endif
    buf->base = conn->buffer_uv;
    buf->len  = conn->buffer_size;
}

#ifndef CH_WITHOUT_TLS
// .. c:function::
void
ch_cn_send_if_pending(ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_cn_send_if_pending`
//
// .. code-block:: cpp
//
{
    A(!(conn->flags & CH_CN_WRITE_PENDING), "Another write is still pending");
    ch_chirp_t* chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    int pending = BIO_pending(conn->bio_app);
    if (pending < 1) {
        if (!(conn->flags & CH_CN_TLS_HANDSHAKE ||
              conn->flags & CH_CN_SHUTTING_DOWN)) {
            int stop;
            ch_rd_read(conn, NULL, 0, &stop); /* Start the reader */
        }
        return;
    }
    A(!(conn->flags & CH_CN_BUF_WTLS_USED), "The wtls buffer is still used");
#ifdef CH_ENABLE_ASSERTS
    conn->flags |= CH_CN_BUF_WTLS_USED;
    conn->flags |= CH_CN_WRITE_PENDING;
#endif
    ssize_t read =
            BIO_read(conn->bio_app, conn->buffer_wtls, conn->buffer_size);
    conn->buffer_wtls_uv.len = read;
    uv_write(
            &conn->write_req,
            (uv_stream_t*) &conn->client,
            &conn->buffer_wtls_uv,
            1,
            _ch_cn_send_pending_cb);
    if (conn->flags & CH_CN_SHUTTING_DOWN) {
        LC(chirp,
           "Sending %d pending shutdown bytes. ",
           "ch_connection_t:%p",
           read,
           (void*) conn);
    } else {
        LC(chirp,
           "Sending %d pending handshake bytes. ",
           "ch_connection_t:%p",
           read,
           (void*) conn);
    }
}
#endif

// .. c:function::
ch_error_t
ch_cn_shutdown(ch_connection_t* conn, int reason)
//    :noindex:
//
//    see: :c:func:`ch_cn_shutdown`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = conn->chirp;
    if (conn->flags & CH_CN_SHUTTING_DOWN) {
        EC(chirp, "Shutdown in progress. ", "ch_connection_t:%p", (void*) conn);
        return CH_IN_PRORESS;
    }
    LC(chirp, "Shutdown connection. ", "ch_connection_t:%p", (void*) conn);
    conn->flags |= CH_CN_SHUTTING_DOWN;
    ch_pr_debounce_connection(conn);
    ch_chirp_int_t*  ichirp = chirp->_;
    ch_writer_t*     writer = &conn->writer;
    ch_remote_t*     remote = conn->remote;
    ch_connection_t* out;
    /* In case this conn is in handshake_conns remove it, since we aborted the
     * handshake */
    ch_cn_delete(&ichirp->protocol.handshake_conns, conn, &out);
    /* In case this conn is in old_connections remove it, since we now cleaned
     * it up*/
    ch_cn_delete(&ichirp->protocol.old_connections, conn, &out);
    conn->remote = NULL; /* Disassociate from remote */
    if (conn->flags & CH_CN_INIT_CLIENT) {
        uv_read_stop((uv_stream_t*) &conn->client);
    }
    ch_message_t* msg = writer->msg;
    ch_message_t* wam = NULL;
    /* In early handshake remote can empty, since we allocate resources after
     * successful handshake. */
    if (remote) {
        /* We finish the message and therefore set wam to NULL. */
        wam                      = remote->wait_ack_message;
        remote->wait_ack_message = NULL;
        /* We could be a connection from old_connections and therefore we do
         * not want to invalidate an active connection. */
        if (remote->conn == conn) {
            remote->conn = NULL;
        }
        /* Abort all ack messsages */
        remote->cntl_msg_queue = NULL;
    }
    if (wam != NULL) {
        wam->_flags |= CH_MSG_FAILURE;
        ch_chirp_finish_message(chirp, conn, wam, reason);
    }
    if (msg != NULL && msg != wam) {
        msg->_flags |= CH_MSG_FAILURE;
        ch_chirp_finish_message(chirp, conn, msg, reason);
    }
    /* finish vs abort - finish: cancel a message on the current connection.
     * abort: means canceling a message that hasn't been queued yet. If
     * possible we don't want to cancel a message that hasn't been queued
     * yet.*/
    if (wam == NULL && msg == NULL && remote != NULL) {
        /* If we have not finished a message we abort one on the remote. */
        ch_cn_abort_one_message(remote, reason);
    }
#ifndef CH_WITHOUT_TLS
    if (conn->flags & CH_CN_ENCRYPTED && conn->flags & CH_CN_INIT_ENCRYPTION) {
        int tmp_err = SSL_get_verify_result(conn->ssl);
        if (tmp_err != X509_V_OK) {
            EC(chirp,
               "Connection has cert verification error: %d. ",
               "ch_connection_t:%p",
               tmp_err,
               (void*) conn);
        }
    }
#endif
    if (ichirp->flags & CH_CHIRP_CLOSING) {
        conn->flags |= CH_CN_DO_CLOSE_ACCOUTING;
        ichirp->closing_tasks += 1;
    }
    _ch_cn_closing(conn);
    return CH_SUCCESS;
}

// .. c:function::
void
ch_cn_write(
        ch_connection_t* conn,
        const uv_buf_t   bufs[],
        unsigned int     nbufs,
        uv_write_cb      callback)
//    :noindex:
//
//    see: :c:func:`ch_cn_write`
//
// .. code-block:: cpp
//
{
    size_t buf_list_size = sizeof(uv_buf_t) * nbufs;
    if (nbufs > conn->bufs_size) {
        conn->bufs      = ch_realloc(conn->bufs, buf_list_size);
        conn->bufs_size = nbufs;
    }
    memcpy(conn->bufs, bufs, buf_list_size);
#ifndef CH_WITHOUT_TLS
    if (conn->flags & CH_CN_ENCRYPTED) {
        conn->write_callback = callback;
        conn->write_written  = 0;
        conn->bufs_index     = 0;
        conn->nbufs          = nbufs;
#ifdef CH_ENABLE_ASSERTS
        A(!(conn->flags & CH_CN_ENCRYPTED_WRITE), "Encrypted write pending");
        conn->flags |= CH_CN_ENCRYPTED_WRITE;
        int pending = BIO_pending(conn->bio_app);
        A(pending == 0, "There is still pending data in SSL BIO");
#endif
        _ch_cn_partial_write(conn);
        return;
    }
#endif
    uv_write(
            &conn->write_req,
            (uv_stream_t*) &conn->client,
            conn->bufs,
            nbufs,
            callback);
#ifdef CH_ENABLE_LOGGING
    ch_chirp_t* chirp = conn->chirp;
    for (unsigned int i = 0; i < nbufs; i++) {
        LC(chirp,
           "Wrote %d bytes. ",
           "ch_connection_t:%p",
           (int) conn->bufs[i].len,
           (void*) conn);
    }
#endif
}
// ==========
// Encryption
// ==========
//

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "encryption.h" */
/* #include "chirp.h" */
/* #include "common.h" */
/* #include "util.h" */

// System includes
// ===============
//
// .. code-block:: cpp
//
#ifndef CH_WITHOUT_TLS
#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/engine.h>
#include <openssl/err.h>

// Declarations
// ============

// .. c:var:: _ch_en_manual_tls
//
//    The user will call ch_en_tls_init() and ch_en_tls_cleanup().
//    Defaults to 0.
//
// .. code-block:: cpp
//
static char _ch_en_manual_tls = 0;

#ifdef CH_OPENSSL_10_API
// .. c:var:: _ch_en_lock_count
//
//    The count of locks created for openssl.
//
// .. code-block:: cpp
//
static int _ch_en_lock_count = 0;

// .. c:var:: _ch_en_lock_list
//
//    List of locks provided for openssl. Openssl will tell us how many locks
//    it needs.
//
// .. code-block:: cpp
//
static uv_rwlock_t* _ch_en_lock_list = NULL;

// .. c:function::
static void
_ch_en_locking_function(int mode, int n, const char* file, int line);
//
//    Called by openssl to lock a mutex.
//
//    :param int mode: Can be CRYPTO_LOCK or CRYPTO_UNLOCK
//    :param int n: The lock to lock/unlock
//    :param const char* file: File the function was called from (deubbing)
//    :param const char* line: Line the function was called from (deubbing)
//

// .. c:function::
static unsigned long
_ch_en_thread_id_function(void);
//
//    Called by openssl to get the current thread it.
//
#endif // CH_OPENSSL_10_API

// Definitions
// ===========
//
// .. c:function::
void
ch_en_init(ch_chirp_t* chirp, ch_encryption_t* enc)
//    :noindex:
//
//    see: :c:func:`ch_en_init`
//
// .. code-block:: cpp
//
{
    memset(enc, 0, sizeof(*enc));
    enc->chirp = chirp;
}

#ifdef CH_OPENSSL_10_API
// .. c:function::
static void
_ch_en_locking_function(int mode, int n, const char* file, int line)
//    :noindex:
//
//    see: :c:func:`_ch_en_locking_function`
//
// .. code-block:: cpp
//
{
    (void) (file);
    (void) (line);
    if (mode & CRYPTO_LOCK) {
        if (!(mode & CRYPTO_READ)) { /* The user requested write */
            uv_rwlock_wrlock(&_ch_en_lock_list[n]);
        } else if (!(mode & CRYPTO_WRITE)) { /* The user requested read */
            uv_rwlock_rdlock(&_ch_en_lock_list[n]);
        } else { /* The user request is bad, do a wrlock for safety */
            uv_rwlock_wrlock(&_ch_en_lock_list[n]);
        }
    } else {
        if (!(mode & CRYPTO_READ)) {
            uv_rwlock_wrunlock(&_ch_en_lock_list[n]);
        } else if (!(mode & CRYPTO_WRITE)) {
            uv_rwlock_rdunlock(&_ch_en_lock_list[n]);
        } else {
            uv_rwlock_wrunlock(&_ch_en_lock_list[n]);
        }
    }
}
#endif // CH_OPENSSL_10_API
#endif // CH_WITHOUT_TLS

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_tls_init(void)
//    :noindex:
//
//    see: :c:func:`ch_en_tls_init`
//
// .. code-block:: cpp
//
{
#ifndef CH_WITHOUT_TLS
    if (_ch_en_manual_tls) {
        return CH_SUCCESS;
    }
    /* Detect if ssl is already initialized by host program */
    if (EVP_get_cipherbyname("AES-256-CBC")) {
        _ch_en_manual_tls = 1;
        return CH_SUCCESS;
    }
#ifdef CH_OPENSSL_10_API
    if (CRYPTO_get_locking_callback() != NULL) {
        _ch_en_manual_tls = 1;
        return CH_SUCCESS;
    }
    OPENSSL_add_all_algorithms_noconf();
    SSL_load_error_strings();
    SSL_library_init();

    return ch_en_tls_threading_setup();
#else
    if (OPENSSL_init_ssl(0, NULL) == 0) {
        return CH_TLS_ERROR;
    }
    return CH_SUCCESS;
#endif
#else  // CH_WITHOUT_TLS
    return CH_SUCCESS;
#endif // CH_WITHOUT_TLS
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_tls_cleanup(void)
//    :noindex:
//
//    see: :c:func:`ch_en_tls_cleanup`
//
// .. code-block:: cpp
//
{
#ifndef CH_WITHOUT_TLS
    if (_ch_en_manual_tls) {
        return CH_SUCCESS;
    }

#ifdef CH_OPENSSL
    FIPS_mode_set(0);
#endif
    ENGINE_cleanup();
    CONF_modules_unload(1);
    ERR_free_strings();
    CONF_modules_free();
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
#ifdef CH_OPENSSL_10_API
    CRYPTO_THREADID id;
    CRYPTO_THREADID_current(&id);
    ERR_remove_thread_state(&id);
    ERR_remove_thread_state(NULL);
#endif
    ASN1_STRING_TABLE_cleanup();

    return ch_en_tls_threading_cleanup();
#else // CH_WITHOUT_TLS
    return CH_SUCCESS;
#endif
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_tls_threading_cleanup(void)
//    :noindex:
//
//    see: :c:func:`ch_en_tls_threading_cleanup`
//
// .. code-block:: cpp
//
{
#ifndef CH_WITHOUT_TLS
#ifdef CH_OPENSSL_10_API
    A(_ch_en_lock_list, "Threading not setup");
    if (!_ch_en_lock_list) {
        fprintf(stderr,
                "%s:%d Fatal: Threading not setup.\n",
                __FILE__,
                __LINE__);
        return CH_VALUE_ERROR;
    }
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    for (int i = 0; i < _ch_en_lock_count; i++) {
        uv_rwlock_destroy(&_ch_en_lock_list[i]);
    }
    ch_free(_ch_en_lock_list);
    _ch_en_lock_list = NULL;
#else
    OPENSSL_thread_stop();
#endif // CH_OPENSSL_10_API
#endif // CH_WITHOUT_TLS
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_en_tls_threading_setup(void)
//    :noindex:
//
//    see: :c:func:`ch_en_tls_threading_setup`
//
// .. code-block:: cpp
//
{
#ifndef CH_WITHOUT_TLS
#ifdef CH_OPENSSL_10_API
    A(!_ch_en_lock_list, "Threading already setup");
    if (_ch_en_lock_list) {
        fprintf(stderr,
                "%s:%d Fatal: Threading already setup.\n",
                __FILE__,
                __LINE__);
        return CH_VALUE_ERROR;
    }
    int lock_count   = CRYPTO_num_locks();
    _ch_en_lock_list = ch_alloc(lock_count * sizeof(uv_rwlock_t));
    if (!_ch_en_lock_list) {
        fprintf(stderr,
                "%s:%d Fatal: Could not allocate memory for locking.\n",
                __FILE__,
                __LINE__);
        return CH_ENOMEM;
    }
    _ch_en_lock_count = lock_count;
    for (int i = 0; i < lock_count; i++) {
        if (uv_rwlock_init(&_ch_en_lock_list[i]) < 0) {
            ch_free(_ch_en_lock_list);
            return CH_INIT_FAIL;
        }
    }
    CRYPTO_set_id_callback(_ch_en_thread_id_function);
    CRYPTO_set_locking_callback(_ch_en_locking_function);
#endif // CH_OPENSSL_10_API
#endif // CH_WITHOUT_TLS
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
void
ch_en_set_manual_tls_init(void)
//    :noindex:
//
//    see: :c:func:`ch_en_set_manual_tls_init`
//
// .. code-block:: cpp
//
{
#ifndef CH_WITHOUT_TLS
    _ch_en_manual_tls = 1;
#endif
}

#ifndef CH_WITHOUT_TLS
// .. c:function::
ch_error_t
ch_en_start(ch_encryption_t* enc)
//    :noindex:
//
//    see: :c:func:`ch_en_start`
//
// .. code-block:: cpp
//
{
    ch_chirp_t*     chirp  = enc->chirp;
    ch_chirp_int_t* ichirp = chirp->_;
#ifdef CH_OPENSSL_10_API
    const SSL_METHOD* method = TLSv1_2_method();
#else
    const SSL_METHOD* method = TLS_method();
#endif
    if (method == NULL) {
        E(chirp, "Could not get the TLSv1_2_method", CH_NO_ARG);
        return CH_TLS_ERROR;
    }
    enc->ssl_ctx = SSL_CTX_new(method);
    if (enc->ssl_ctx == NULL) {
        E(chirp, "Could create the SSL_CTX", CH_NO_ARG);
        return CH_TLS_ERROR;
    }
    SSL_CTX_set_mode(
            enc->ssl_ctx, SSL_MODE_AUTO_RETRY | SSL_MODE_ENABLE_PARTIAL_WRITE);
    SSL_CTX_set_options(enc->ssl_ctx, SSL_OP_NO_COMPRESSION);
    SSL_CTX_set_verify(
            enc->ssl_ctx,
            SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
            NULL);
#ifndef CH_OPENSSL_10_API // NOT DEF!
    SSL_CTX_set_min_proto_version(enc->ssl_ctx, TLS1_2_VERSION);
#endif
    SSL_CTX_set_verify_depth(enc->ssl_ctx, 5);
    if (SSL_CTX_load_verify_locations(
                enc->ssl_ctx, ichirp->config.CERT_CHAIN_PEM, NULL) != 1) {
        E(chirp,
          "Could not set the verification certificate %s",
          ichirp->config.CERT_CHAIN_PEM);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if (SSL_CTX_use_certificate_chain_file(
                enc->ssl_ctx, ichirp->config.CERT_CHAIN_PEM) != 1) {
        E(chirp,
          "Could not set the certificate %s",
          ichirp->config.CERT_CHAIN_PEM);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if (SSL_CTX_use_PrivateKey_file(
                enc->ssl_ctx,
                ichirp->config.CERT_CHAIN_PEM,
                SSL_FILETYPE_PEM) != 1) {
        E(chirp,
          "Could not set the private key %s",
          ichirp->config.CERT_CHAIN_PEM);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if (SSL_CTX_check_private_key(enc->ssl_ctx) != 1) {
        E(chirp, "Private key is not valid %s", ichirp->config.CERT_CHAIN_PEM);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    DH*   dh        = NULL;
    FILE* paramfile = fopen(ichirp->config.DH_PARAMS_PEM, "r");
    if (paramfile == NULL) {
        E(chirp,
          "Could not open the dh-params %s",
          ichirp->config.DH_PARAMS_PEM);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    dh = PEM_read_DHparams(paramfile, NULL, NULL, NULL);
    fclose(paramfile);
    if (dh == NULL) {
        E(chirp,
          "Could not load the dh-params %s",
          ichirp->config.DH_PARAMS_PEM);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if (SSL_CTX_set_tmp_dh(enc->ssl_ctx, dh) != 1) {
        E(chirp,
          "Could not set the dh-params %s",
          ichirp->config.DH_PARAMS_PEM);
        DH_free(dh);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    if (SSL_CTX_set_cipher_list(
                enc->ssl_ctx,
                "-ALL:"
                "DHE-DSS-AES256-GCM-SHA384:"
                "DHE-RSA-AES256-GCM-SHA384:"
                "DHE-RSA-AES256-SHA256:"
                "DHE-DSS-AES256-SHA256:") != 1) {
        E(chirp, "Could not set the cipher list. ch_chirp_t: %p", chirp);
        DH_free(dh);
        SSL_CTX_free(enc->ssl_ctx);
        return CH_TLS_ERROR;
    }
    L(chirp, "Created SSL context for chirp", CH_NO_ARG);
    DH_free(dh);
    return CH_SUCCESS;
}

// .. c:function::
ch_error_t
ch_en_stop(ch_encryption_t* enc)
//    :noindex:
//
//    see: :c:func:`ch_en_stop`
//
// .. code-block:: cpp
//
{
    if (enc->ssl_ctx) {
        SSL_CTX_free(enc->ssl_ctx);
    }
    ERR_clear_error();
#ifdef CH_OPENSSL_10_API
    CRYPTO_THREADID id;
    CRYPTO_THREADID_current(&id);
    ERR_remove_thread_state(&id);
#endif // CH_OPENSSL_10_API
    return CH_SUCCESS;
}

#ifdef CH_OPENSSL_10_API
// .. c:function::
static unsigned long
_ch_en_thread_id_function(void)
//    :noindex:
//
//    see: :c:func:`_ch_en_thread_id_function`
//
// .. code-block:: cpp
//
{
    uv_thread_t self = uv_thread_self();
    return (unsigned long) self;
}
#endif // CH_OPENSSL_10_API
#endif // CH_WITHOUT_TLS
// =======
// Message
// =======
//

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "message.h" */
/* #include "util.h" */

// Definitions
// ===========

// Queue definition
// ----------------
//

qs_queue_bind_impl_cx_m(ch_msg, ch_message_t) CH_ALLOW_NL;

// Interface definitions
// ---------------------

// .. c:function::
CH_EXPORT
void
ch_msg_free_data(ch_message_t* message)
//    :noindex:
//
//    see: :c:func:`ch_msg_get_address`
//
// .. code-block:: cpp
//
{
    if (message->_flags & CH_MSG_FREE_HEADER) {
        ch_free(message->header);
        message->_flags &= ~CH_MSG_FREE_HEADER;
    }
    message->header = NULL;
    if (message->_flags & CH_MSG_FREE_DATA) {
        ch_free(message->data);
        message->_flags &= ~CH_MSG_FREE_DATA;
    }
    message->data = NULL;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_get_address(const ch_message_t* message, ch_text_address_t* address)
//    :noindex:
//
//    see: :c:func:`ch_msg_get_address`
//
// .. code-block:: cpp
//
{
    int ip_protocol = message->ip_protocol;
    if (!(ip_protocol == AF_INET || ip_protocol == AF_INET6)) {
        return CH_VALUE_ERROR;
    }
    int tmp_err = uv_inet_ntop(
            ip_protocol,
            message->address,
            address->data,
            sizeof(address->data));
    /* This error is not dynamic, it means ch_text_address_t is too small, so
     * we do not return it to the user */
    A(tmp_err == 0, "Cannot convert address to text (not enough space)");
    (void) (tmp_err);
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
ch_identity_t
ch_msg_get_identity(ch_message_t* message)
//    :noindex:
//
//    see: :c:func:`ch_msg_get_identity`
//
// .. code-block:: cpp
//
{
    ch_identity_t id;
    memcpy(id.data, message->identity, sizeof(id.data));
    return id;
}

// .. c:function::
CH_EXPORT
ch_identity_t
ch_msg_get_remote_identity(ch_message_t* message)
//    :noindex:
//
//    see: :c:func:`ch_msg_get_remote_identity`
//
// .. code-block:: cpp
//
{
    ch_identity_t id;
    memcpy(id.data, message->remote_identity, sizeof(id.data));
    return id;
}

// .. c:function::
CH_EXPORT
int
ch_msg_has_slot(ch_message_t* message)
//    :noindex:
//
//    see: :c:func:`ch_msg_has_slot`
//
// .. code-block:: cpp
//
{
    return message->_flags & CH_MSG_HAS_SLOT;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_init(ch_message_t* message)
//    :noindex:
//
//    see: :c:func:`ch_msg_init`
//
// .. code-block:: cpp
//
{
    memset(message, 0, sizeof(*message));
    ch_random_ints_as_bytes(message->identity, sizeof(message->identity));
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_msg_set_address(
        ch_message_t*    message,
        ch_ip_protocol_t ip_protocol,
        const char*      address,
        int32_t          port)
//    :noindex:
//
//    see: :c:func:`ch_msg_set_address`
//
// .. code-block:: cpp
//
{
    message->ip_protocol = ip_protocol;
    if (!(ip_protocol == AF_INET || ip_protocol == AF_INET6)) {
        return CH_VALUE_ERROR;
    }
    if (uv_inet_pton(ip_protocol, address, message->address)) {
        return CH_VALUE_ERROR;
    }
    message->port = port;
    return CH_SUCCESS;
}

// .. c:function::
CH_EXPORT
void
ch_msg_set_data(ch_message_t* message, ch_buf* data, uint32_t len)
//    :noindex:
//
//    see: :c:func:`ch_msg_set_data`
//
// .. code-block:: cpp
//
{
    message->data     = data;
    message->data_len = len;
}
// ========
// Protocol
// ========
//

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "protocol.h" */
/* #include "chirp.h" */
/* #include "common.h" */
/* #include "remote.h" */
/* #include "util.h" */

// System includes
// ===============
//
// .. code-block:: cpp
//
#ifndef CH_WITHOUT_TLS
#include <openssl/err.h>
#endif

// Declarations
// ============

// .. c:function::
static void
_ch_pr_abort_all_messages(ch_remote_t* remote, ch_error_t error);
//
//    Abort all messages in queue, because we are closing down.
//
//    :param ch_remote_t* remote: Remote failed to connect.
//    :param ch_error_t error: Status returned by connect.

#ifndef CH_WITHOUT_TLS
// .. c:function::
static int
_ch_pr_decrypt_feed(ch_connection_t* conn, ch_buf* buf, size_t read, int* stop);
//
//    Feeds data into the SSL BIO.
//
//    :param ch_connection_t* conn: Pointer to a connection handle.
//    :param ch_buf* buf:           The buffer containing ``read`` bytes read.
//    :param size_t read:           The number of bytes read.
//    :param int* stop:             (Out) Stop the reading process.
//
#endif

#ifndef CH_WITHOUT_TLS
// .. c:function::
static void
_ch_pr_do_handshake(ch_connection_t* conn);
//
//    Do a handshake on the given connection.
//
//    :param ch_connection_t* conn: Pointer to a connection handle.
//
#endif

// .. c:function::
static void
_ch_pr_gc_connections_cb(uv_timer_t* handle);
//
//    Called to cleanup connections/remotes that aren't use for REUSE_TIME+.
//
//    :param uv_timer_t* handle: uv timer handle, data contains chirp

// .. c:function::
static void
_ch_pr_new_connection_cb(uv_stream_t* server, int status);
//
//    Callback from libuv when a stream server has received an incoming
//    connection.
//
//    :param uv_stream_t* server: Pointer to the stream handle (duplex
//                                communication channel) of the server,
//                                containig a chirp object.

// .. c:function::
static void
_ch_pr_read_data_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
//
//    Callback called from libuv when data was read on a stream.
//    Reads nread bytes on either an encrypted or an unencrypted connection
//    coming from the given stream handle.
//
//    :param uv_stream_t* stream: Pointer to the stream that data was read on.
//    :param ssize_t nread: Number of bytes that were read on the stream.
//    :param uv_buf_t* buf: Pointer to a libuv (data-) buffer. When nread < 0,
//                          the buf parameter might not point to a valid
//                          buffer; in that case buf.len and buf.base are both
//                          set to 0.

// .. c:function::
static int
_ch_pr_read_resume(ch_connection_t* conn, ch_resume_state_t* resume);
//
//    Resumes the ch_rd_read based on a given resume state. If the connection
//    is not encrypted we use conn->read_resume, else conn->tls_resume.
//
//    :param ch_connection_t* conn: Pointer to a connection handle.
//    :param ch_resume_state_t* resume: Pointer to a resume state.

// .. c:function::
static int
_ch_pr_resume(ch_connection_t* conn);
//
//    Resume partial read when the connection was stopped because the last
//    message-slot was used. Returns 1 if it ok to restart the reader.
//
//    :param ch_connection_t* conn: Pointer to a connection handle.

// .. c:function::
static inline ch_error_t
_ch_pr_start_socket(
        ch_chirp_t*      chirp,
        int              af,
        uv_tcp_t*        server,
        uint8_t*         bind,
        struct sockaddr* addr,
        int              v6only,
        char*            needs_uninit);
//
//    Since dual stack sockets don't work on all platforms we start a IPv4 and
//    IPv6 socket.

// .. c:function::
static void
_ch_pr_update_resume(
        ch_resume_state_t* resume,
        ch_buf*            buf,
        size_t             nread,
        ssize_t            bytes_handled);
//
//    Update the resume state. Checks if reading was partial and sets resume
//    state that points to the remaining data. If the last message-slot was
//    used, it is possible that all that has been read and we can just stop or
//    that there is still a message in the buffer.
//
//    :param ch_resume_state_t* resume: Pointer to resume state.
//    :param ch_buf* buf: Pointer to buffer being checked.
//    :param size_t nread: Total bytes available
//    :param ssize_t bytes_handled: Bytes actually handled

// Definitions
// ===========

// .. c:function::
static void
_ch_pr_abort_all_messages(ch_remote_t* remote, ch_error_t error)
//    :noindex:
//
//    see: :c:func:`_ch_pr_abort_all_messages`
//
// .. code-block:: cpp
//
{
    /* The remote is going away, we need to abort all messages */
    ch_message_t* msg;
    ch_msg_dequeue(&remote->msg_queue, &msg);
    while (msg != NULL) {
        ch_send_cb_t cb = msg->_send_cb;
        if (cb != NULL) {
            msg->_send_cb = NULL;
            cb(remote->chirp, msg, error);
        }
        ch_msg_dequeue(&remote->msg_queue, &msg);
    }
    remote->cntl_msg_queue = NULL;
}

#ifndef CH_WITHOUT_TLS
// .. c:function::
static void
_ch_pr_do_handshake(ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`_ch_pr_do_handshake`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp         = conn->chirp;
    conn->tls_handshake_state = SSL_do_handshake(conn->ssl);
    if (SSL_is_init_finished(conn->ssl)) {
        conn->flags &= ~CH_CN_TLS_HANDSHAKE;
        if (conn->tls_handshake_state) {
            LC(chirp,
               "SSL handshake successful. ",
               "ch_connection_t:%p",
               (void*) conn);
        } else {
#ifdef CH_ENABLE_LOGGING
            ERR_print_errors_fp(stderr);
#endif
            EC(chirp,
               "SSL handshake failed. ",
               "ch_connection_t:%p",
               (void*) conn);
            ch_cn_shutdown(conn, CH_TLS_ERROR);
            return;
        }
    }
    ch_cn_send_if_pending(conn);
}
#endif

// .. c:function::
static void
_ch_pr_gc_connections_cb(uv_timer_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_pr_gc_connections_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t*  ichirp       = chirp->_;
    ch_protocol_t*   protocol     = &ichirp->protocol;
    ch_config_t*     config       = &ichirp->config;
    uint64_t         now          = uv_now(ichirp->loop);
    uint64_t         delta        = (1000 * config->REUSE_TIME);
    ch_remote_t*     rm_del_stack = NULL;
    ch_connection_t* cn_del_stack = NULL;

    L(chirp, "Garbage-collecting connections and remotes", CH_NO_ARG);
    rb_iter_decl_cx_m(ch_cn, cn_iter, cn_elem);
    rb_for_m (ch_cn, protocol->old_connections, cn_iter, cn_elem) {
        if (now - cn_elem->timestamp > delta) {
            ch_cn_st_push(&cn_del_stack, cn_elem);
        }
    }
    rb_for_m (ch_cn_st, cn_del_stack, cn_iter, cn_elem) {
        LC(chirp,
           "Garbage-collecting: shutdown.",
           "ch_connection_t:%p",
           cn_elem);
        ch_cn_shutdown(cn_elem, CH_SHUTDOWN);
    }

    rb_iter_decl_cx_m(ch_rm, rm_iter, rm_elem);
    rb_for_m (ch_rm, protocol->remotes, rm_iter, rm_elem) {
        if (!(rm_elem->flags & CH_RM_CONN_BLOCKED) &&
            now - rm_elem->timestamp > delta) {
            A(rm_elem->next == NULL, "Should not be in reconnect_remotes");
            ch_rm_st_push(&rm_del_stack, rm_elem);
        }
    }
    ch_remote_t* remote     = NULL;
    ch_remote_t* tmp_remote = NULL;
    ch_rm_st_pop(&rm_del_stack, &remote);
    while (remote != NULL) {
        _ch_pr_abort_all_messages(remote, CH_SHUTDOWN);
        ch_rm_delete_node(&protocol->remotes, remote);
        ch_connection_t* conn = remote->conn;
        if (conn != NULL) {
            LC(chirp,
               "Garbage-collecting: shutdown.",
               "ch_connection_t:%p",
               remote->conn);
            conn->delete_remote = remote;
            remote->flags       = CH_RM_CONN_BLOCKED;
            ch_cn_shutdown(remote->conn, CH_SHUTDOWN);
        }
        LC(chirp, "Garbage-collecting: deleting.", "ch_remote_t:%p", remote);
        tmp_remote = remote;
        ch_rm_st_pop(&rm_del_stack, &remote);
        if (conn == NULL) {
            ch_rm_free(tmp_remote);
        }
    }
    uint64_t start = (config->REUSE_TIME * 1000 / 2);
    start += rand() % start;
    uv_timer_start(&protocol->gc_timeout, _ch_pr_gc_connections_cb, start, 0);
}

// .. c:function::
static void
_ch_pr_new_connection_cb(uv_stream_t* server, int status)
//    :noindex:
//
//    see: :c:func:`_ch_pr_new_connection_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = server->data;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    /* Do not accept new connections when chirp is closing */
    if (ichirp->flags & CH_CHIRP_CLOSING) {
        return;
    }
    ch_protocol_t* protocol = &ichirp->protocol;
    if (status < 0) {
        L(chirp, "New connection error %s", uv_strerror(status));
        return;
    }

    ch_connection_t* conn = (ch_connection_t*) ch_alloc(sizeof(*conn));
    if (!conn) {
        E(chirp, "Could not allocate memory for connection", CH_NO_ARG);
        return;
    }
    LC(chirp, "Accepted connection. ", "ch_connection_t:%p", (void*) conn);
    memset(conn, 0, sizeof(*conn));
    ch_cn_node_init(conn);
    ch_cn_insert(&protocol->handshake_conns, conn);
    conn->chirp       = chirp;
    conn->client.data = conn;
    uv_tcp_t* client  = &conn->client;
    if (uv_tcp_init(server->loop, client) < 0) {
        EC(chirp,
           "Could not initialize tcp. ",
           "ch_connection_t:%p",
           (void*) conn);
        ch_cn_shutdown(conn, CH_FATAL);
        return;
    }
    conn->flags |= CH_CN_INIT_CLIENT | CH_CN_INCOMING;

    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        struct sockaddr_storage addr;
        int                     addr_len = sizeof(addr);
        ch_text_address_t       taddr;
        if (uv_tcp_getpeername(
                    &conn->client, (struct sockaddr*) &addr, &addr_len) !=
            CH_SUCCESS) {
            EC(chirp,
               "Could not get remote address. ",
               "ch_connection_t:%p",
               (void*) conn);
            ch_cn_shutdown(conn, CH_FATAL);
            return;
        };
        conn->ip_protocol = addr.ss_family;
        if (addr.ss_family == AF_INET6) {
            struct sockaddr_in6* saddr = (struct sockaddr_in6*) &addr;
            memcpy(&conn->address, &saddr->sin6_addr, sizeof(saddr->sin6_addr));
            uv_ip6_name(saddr, taddr.data, sizeof(taddr.data));
        } else {
            struct sockaddr_in* saddr = (struct sockaddr_in*) &addr;
            memcpy(&conn->address, &saddr->sin_addr, sizeof(saddr->sin_addr));
            uv_ip4_name(saddr, taddr.data, sizeof(taddr.data));
        }
#ifndef CH_WITHOUT_TLS
        if (!(ichirp->config.DISABLE_ENCRYPTION || ch_is_local_addr(&taddr))) {
            conn->flags |= CH_CN_ENCRYPTED;
        }
#endif
        ch_pr_conn_start(chirp, conn, client, 1);
    } else {
        ch_cn_shutdown(conn, CH_FATAL);
    }
}

// .. c:function::
static int
_ch_pr_read_resume(ch_connection_t* conn, ch_resume_state_t* resume)
//    :noindex:
//
//    see: :c:func:`_ch_pr_read_resume`
//
// .. code-block:: cpp
//
{
    ch_buf* buf            = resume->rest_of_buffer;
    size_t  nread          = resume->bytes_to_read;
    resume->rest_of_buffer = NULL;
    resume->bytes_to_read  = 0;
    int     stop;
    ssize_t bytes_handled = ch_rd_read(conn, buf, nread, &stop);
    A(resume->bytes_to_read ? resume->rest_of_buffer != NULL : 1,
      "No buffer set");
    if (stop) {
        _ch_pr_update_resume(resume, buf, nread, bytes_handled);
    }
    return !stop;
}

// .. c:function::
static inline ch_error_t
_ch_pr_start_socket(
        ch_chirp_t*      chirp,
        int              af,
        uv_tcp_t*        server,
        uint8_t*         bind,
        struct sockaddr* addr,
        int              v6only,
        char*            needs_uninit)
//    :noindex:
//
//    see: :c:func:`_ch_pr_start_socket`
//
// .. code-block:: cpp
//
{
    ch_text_address_t tmp_addr;
    int               tmp_err;
    ch_chirp_int_t*   ichirp = chirp->_;
    ch_config_t*      config = &ichirp->config;
    *needs_uninit            = 0;
    if (uv_tcp_init(ichirp->loop, server)) {
        return CH_INIT_FAIL;
    }
    *needs_uninit = 1;
    server->data  = chirp;

    tmp_err = uv_inet_ntop(af, bind, tmp_addr.data, sizeof(tmp_addr.data));
    if (tmp_err != CH_SUCCESS) {
        return CH_VALUE_ERROR;
    }

    tmp_err = ch_textaddr_to_sockaddr(
            af, &tmp_addr, config->PORT, (struct sockaddr_storage*) addr);
    if (tmp_err != CH_SUCCESS) {
        return tmp_err;
    }

    tmp_err = uv_tcp_bind(server, addr, v6only);
    if (tmp_err != CH_SUCCESS) {
        fprintf(stderr,
                "%s:%d Fatal: cannot bind port (IPv%d:%d)\n",
                __FILE__,
                __LINE__,
                af == AF_INET6 ? 6 : 4,
                config->PORT);
        return CH_EADDRINUSE;
    }

    tmp_err = uv_tcp_nodelay(server, 1);
    if (tmp_err != CH_SUCCESS) {
        return CH_UV_ERROR;
    }

    tmp_err = uv_listen(
            (uv_stream_t*) server, config->BACKLOG, _ch_pr_new_connection_cb);
    if (tmp_err != CH_SUCCESS) {
        fprintf(stderr,
                "%s:%d Fatal: cannot listen port (IPv%d:%d)\n",
                __FILE__,
                __LINE__,
                af == AF_INET6 ? 6 : 4,
                config->PORT);
        return CH_EADDRINUSE;
    }
    return CH_SUCCESS;
}

// .. c:function::
static void
_ch_pr_update_resume(
        ch_resume_state_t* resume,
        ch_buf*            buf,
        size_t             nread,
        ssize_t            bytes_handled)
//    :noindex:
//
//    see: :c:func:`_ch_pr_update_resume`
//
// .. code-block:: cpp
//
{
    if (bytes_handled != -1 && bytes_handled != (ssize_t) nread) {
        A(resume->rest_of_buffer == NULL || resume->bytes_to_read != 0,
          "Last partial read not completed");
        resume->rest_of_buffer = buf + bytes_handled;
        resume->bytes_to_read  = nread - bytes_handled;
    }
}

// .. c:function::
ch_error_t
ch_pr_conn_start(
        ch_chirp_t* chirp, ch_connection_t* conn, uv_tcp_t* client, int accept)
//    :noindex:
//
//    see: :c:func:`ch_pr_conn_start`
//
// .. code-block:: cpp
//
{
    int tmp_err = ch_cn_init(chirp, conn, conn->flags);
    if (tmp_err != CH_SUCCESS) {
        E(chirp, "Could not initialize connection (%d)", tmp_err);
        ch_cn_shutdown(conn, tmp_err);
        return tmp_err;
    }
    tmp_err = uv_tcp_nodelay(client, 1);
    if (tmp_err != CH_SUCCESS) {
        E(chirp, "Could not set tcp nodelay on connection (%d)", tmp_err);
        ch_cn_shutdown(conn, CH_UV_ERROR);
        return CH_UV_ERROR;
    }

    tmp_err = uv_tcp_keepalive(client, 1, CH_TCP_KEEPALIVE);
    if (tmp_err != CH_SUCCESS) {
        E(chirp, "Could not set tcp keepalive on connection (%d)", tmp_err);
        ch_cn_shutdown(conn, CH_UV_ERROR);
        return CH_UV_ERROR;
    }

    uv_read_start(
            (uv_stream_t*) client, ch_cn_read_alloc_cb, _ch_pr_read_data_cb);
#ifdef CH_WITHOUT_TLS
    (void) (accept);
#else
    if (conn->flags & CH_CN_ENCRYPTED) {
        if (accept) {
            SSL_set_accept_state(conn->ssl);
        } else {
            SSL_set_connect_state(conn->ssl);
            _ch_pr_do_handshake(conn);
        }
        conn->flags |= CH_CN_TLS_HANDSHAKE;
        return CH_SUCCESS;
    }
#endif
    int stop;
    ch_rd_read(conn, NULL, 0, &stop); /* Start reader */
    return CH_SUCCESS;
}

// .. c:function::
void
ch_pr_close_free_remotes(ch_chirp_t* chirp, int only_conns)
//    :noindex:
//
//    see: :c:func:`ch_pr_close_free_remotes`
//
// .. code-block:: cpp
//
{
    ch_chirp_int_t* ichirp   = chirp->_;
    ch_protocol_t*  protocol = &ichirp->protocol;
    while (protocol->old_connections != ch_cn_nil_ptr) {
        ch_cn_shutdown(protocol->old_connections, CH_SHUTDOWN);
    }
    while (protocol->handshake_conns != ch_cn_nil_ptr) {
        ch_cn_shutdown(protocol->handshake_conns, CH_SHUTDOWN);
    }
    if (only_conns) {
        rb_iter_decl_cx_m(ch_rm, rm_iter, rm_elem);
        rb_for_m (ch_rm, protocol->remotes, rm_iter, rm_elem) {
            if (rm_elem->conn != NULL) {
                ch_cn_shutdown(rm_elem->conn, CH_SHUTDOWN);
                ch_wr_process_queues(rm_elem);
            }
        }
    } else {
        while (protocol->remotes != ch_rm_nil_ptr) {
            ch_remote_t* remote = protocol->remotes;
            /* Leaves the queue empty and therefor prevents reconnects. */
            _ch_pr_abort_all_messages(remote, CH_SHUTDOWN);
            ch_rm_delete_node(&protocol->remotes, remote);
            ch_connection_t* conn = remote->conn;
            if (conn != NULL) {
                conn->delete_remote = remote;
                ch_cn_shutdown(conn, CH_SHUTDOWN);
            } else {
                ch_rm_free(remote);
            }
        }
        /* Remove all remotes, sync with reconnect_remotes */
        protocol->reconnect_remotes = NULL;
    }
}

// .. c:function::
void
ch_pr_debounce_connection(ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_pr_conn_start`
//
// .. code-block:: cpp
//
{
    ch_remote_t     key;
    ch_chirp_t*     chirp    = conn->chirp;
    ch_chirp_int_t* ichirp   = chirp->_;
    ch_remote_t*    remote   = NULL;
    ch_protocol_t*  protocol = &ichirp->protocol;
    ch_rm_init_from_conn(chirp, &key, conn, 1);
    if (ch_rm_find(protocol->remotes, &key, &remote) == CH_SUCCESS) {
        if (protocol->reconnect_remotes == NULL) {
            uv_timer_start(
                    &protocol->reconnect_timeout,
                    ch_pr_reconnect_remotes_cb,
                    50 + (rand() % 500),
                    0);
        }
        if (!(remote->flags & CH_RM_CONN_BLOCKED)) {
            remote->flags |= CH_RM_CONN_BLOCKED;
            ch_rm_st_push(&protocol->reconnect_remotes, remote);
        }
    }
}

#ifndef CH_WITHOUT_TLS
// .. c:function::
static int
_ch_pr_decrypt_feed(ch_connection_t* conn, ch_buf* buf, size_t nread, int* stop)
//    :noindex:
//
//    see: :c:func:`ch_pr_decrypt_feed`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp         = conn->chirp;
    size_t      bytes_handled = 0;
    *stop                     = 0;
    do {
        if (nread > 0) {
            ssize_t tmp_err = 0;
            tmp_err         = BIO_write(
                    conn->bio_app, buf + bytes_handled, nread - bytes_handled);
            if (tmp_err < 1) {
                if (!(conn->flags & CH_CN_STOPPED)) {
                    EC(chirp,
                       "SSL error writing to BIO, shutting down connection. ",
                       "ch_connection_t:%p",
                       (void*) conn);
                    ch_cn_shutdown(conn, CH_TLS_ERROR);
                    return -1;
                }
            } else {
                bytes_handled += tmp_err;
            }
        }
        if (conn->flags & CH_CN_TLS_HANDSHAKE) {
            _ch_pr_do_handshake(conn);
        } else {
            ch_pr_decrypt_read(conn, stop);
            if (*stop) {
                return bytes_handled;
            }
        }
    } while (bytes_handled < nread);
    return bytes_handled;
}
#endif

#ifndef CH_WITHOUT_TLS
// .. c:function::
void
ch_pr_decrypt_read(ch_connection_t* conn, int* stop)
//    :noindex:
//
//    see: :c:func:`ch_pr_decrypt_read`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = conn->chirp;
    ssize_t     tmp_err;
    *stop = 0;
    while ((tmp_err = SSL_read(
                    conn->ssl, conn->buffer_rtls, conn->buffer_rtls_size)) >
           0) {
        ;
        LC(chirp,
           "Read %d bytes. (unenc). ",
           "ch_connection_t:%p",
           tmp_err,
           (void*) conn);
        ssize_t bytes_handled =
                ch_rd_read(conn, conn->buffer_rtls, tmp_err, stop);
        if (*stop) {
            _ch_pr_update_resume(
                    &conn->tls_resume,
                    conn->buffer_rtls,
                    tmp_err,
                    bytes_handled);
            return;
        }
    }
    tmp_err = SSL_get_error(conn->ssl, tmp_err);
    if (tmp_err != SSL_ERROR_WANT_READ) {
        if (tmp_err < 0) {
#ifdef CH_ENABLE_LOGGING
            ERR_print_errors_fp(stderr);
#endif
            EC(chirp,
               "SSL operation fatal error. ",
               "ch_connection_t:%p",
               (void*) conn);
        } else {
            LC(chirp,
               "SSL operation failed. ",
               "ch_connection_t:%p",
               (void*) conn);
        }
        ch_cn_shutdown(conn, CH_TLS_ERROR);
    }
}
#endif

// .. c:function::
void
ch_pr_init(ch_chirp_t* chirp, ch_protocol_t* protocol)
//    :noindex:
//
//    see: :c:func:`ch_pr_init`
//
// .. code-block:: cpp
//
{
    memset(protocol, 0, sizeof(*protocol));
    protocol->chirp = chirp;
    ch_cn_tree_init(&protocol->handshake_conns);
    ch_cn_tree_init(&protocol->old_connections);
    ch_rm_tree_init(&protocol->remotes);
    protocol->reconnect_remotes = NULL;
}

// .. c:function::
void
ch_pr_reconnect_remotes_cb(uv_timer_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_pr_reconnect_remotes_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t*    chirp    = handle->data;
    ch_protocol_t* protocol = &chirp->_->protocol;
    ch_chirp_check_m(chirp);
    if (protocol->reconnect_remotes != NULL) {
        ch_remote_t* remote;
        ch_rm_st_pop(&protocol->reconnect_remotes, &remote);
        while (remote != NULL) {
            remote->flags &= ~CH_RM_CONN_BLOCKED;
            ch_wr_process_queues(remote);
            ch_rm_st_pop(&protocol->reconnect_remotes, &remote);
        }
    }
}

// .. c:function::
void
ch_pr_restart_stream(ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_pr_resume`
//
// .. code-block:: cpp
//
{
    if (conn != NULL) {
        LC(conn->chirp, "Resume reading", "ch_connection_t:%p", conn);
        if (_ch_pr_resume(conn)) {
            if (conn->flags & CH_CN_STOPPED) {
                conn->flags &= ~CH_CN_STOPPED;
                LC(conn->chirp, "Restart stream", "ch_connection_t:%p", conn);
                uv_read_start(
                        (uv_stream_t*) &conn->client,
                        ch_cn_read_alloc_cb,
                        _ch_pr_read_data_cb);
            }
        }
    }
}

// .. c:function::
static int
_ch_pr_resume(ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_pr_resume`
//
// .. code-block:: cpp
//
{
#ifndef CH_WITHOUT_TLS
    if (conn->flags & CH_CN_ENCRYPTED) {
        int stop;
        int ret = _ch_pr_read_resume(conn, &conn->tls_resume);
        if (!ret) {
            return ret;
        }
        ch_resume_state_t* resume = &conn->read_resume;

        ssize_t bytes_handled;
        ch_buf* buf            = resume->rest_of_buffer;
        size_t  nread          = resume->bytes_to_read;
        resume->rest_of_buffer = NULL;
        resume->bytes_to_read  = 0;

        bytes_handled = _ch_pr_decrypt_feed(conn, buf, nread, &stop);
        if (stop) {
            _ch_pr_update_resume(resume, buf, nread, bytes_handled);
        }
        return !stop;
    }
#endif
    return _ch_pr_read_resume(conn, &conn->read_resume);
}

// .. c:function::
static void
_ch_pr_read_data_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
//    :noindex:
//
//    see: :c:func:`ch_pr_read_data_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn  = stream->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    /* Ignore reads while shutting down */
    if (conn->flags & CH_CN_SHUTTING_DOWN) {
        return;
    }
    ssize_t bytes_handled = 0;
#ifdef CH_ENABLE_ASSERTS
    conn->flags &= ~CH_CN_BUF_UV_USED;
#endif
    if (nread == UV_EOF) {
        ch_cn_shutdown(conn, CH_PROTOCOL_ERROR);
        return;
    }

    if (nread == 0) {
        LC(chirp,
           "Unexpected emtpy read (%d) from libuv. ",
           "ch_connection_t:%p",
           (int) nread,
           (void*) conn);
        return;
    }
    if (nread < 0) {
        LC(chirp,
           "Reader got error %d -> shutdown. ",
           "ch_connection_t:%p",
           (int) nread,
           (void*) conn);
        ch_cn_shutdown(conn, CH_PROTOCOL_ERROR);
        return;
    }
    LC(chirp,
       "%d available bytes.",
       "ch_connection_t:%p",
       (int) nread,
       (void*) conn);
    int stop;
#ifndef CH_WITHOUT_TLS
    if (conn->flags & CH_CN_ENCRYPTED) {
        bytes_handled = _ch_pr_decrypt_feed(conn, buf->base, nread, &stop);
    } else {
#endif
        bytes_handled = ch_rd_read(conn, buf->base, nread, &stop);
#ifndef CH_WITHOUT_TLS
    }
#endif
    if (stop) {
        _ch_pr_update_resume(
                &conn->read_resume, buf->base, nread, bytes_handled);
    }
}

// .. c:function::
ch_error_t
ch_pr_start(ch_protocol_t* protocol, uint16_t* uninit)
//    :noindex:
//
//    see: :c:func:`ch_pr_start`
//
// .. code-block:: cpp
//
{
    ch_chirp_t*     chirp  = protocol->chirp;
    ch_chirp_int_t* ichirp = chirp->_;
    ch_config_t*    config = &ichirp->config;
    int             tmp_err;
    char            needs_uninit;
    tmp_err = _ch_pr_start_socket(
            chirp,
            AF_INET,
            &protocol->serverv4,
            config->BIND_V4,
            (struct sockaddr*) &protocol->addrv4,
            0,
            &needs_uninit);
    if (needs_uninit) {
        *uninit |= CH_UNINIT_SERVERV4;
    }
    if (tmp_err != CH_SUCCESS) {
        return tmp_err;
    }
    tmp_err = _ch_pr_start_socket(
            chirp,
            AF_INET6,
            &protocol->serverv6,
            config->BIND_V6,
            (struct sockaddr*) &protocol->addrv6,
            UV_TCP_IPV6ONLY,
            &needs_uninit);
    if (needs_uninit) {
        *uninit |= CH_UNINIT_SERVERV6;
    }
    if (tmp_err != CH_SUCCESS) {
        return tmp_err;
    }
    tmp_err = uv_timer_init(ichirp->loop, &protocol->reconnect_timeout);
    if (tmp_err != CH_SUCCESS) {
        return CH_INIT_FAIL;
    }
    *uninit |= CH_UNINIT_TIMER_RECON;
    protocol->reconnect_timeout.data = chirp;
    tmp_err = uv_timer_init(ichirp->loop, &protocol->gc_timeout);
    if (tmp_err != CH_SUCCESS) {
        return CH_INIT_FAIL;
    }
    protocol->gc_timeout.data = chirp;
    *uninit |= CH_UNINIT_TIMER_GC;

    uint64_t start = (config->REUSE_TIME * 1000 / 2);
    start += rand() % start;
    uv_timer_start(&protocol->gc_timeout, _ch_pr_gc_connections_cb, start, 0);
    return CH_SUCCESS;
}

// .. c:function::
ch_error_t
ch_pr_stop(ch_protocol_t* protocol)
//    :noindex:
//
//    see: :c:func:`ch_pr_stop`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = protocol->chirp;
    L(chirp, "Closing protocol", CH_NO_ARG);
    ch_pr_close_free_remotes(chirp, 0);
    uv_close((uv_handle_t*) &protocol->serverv4, ch_chirp_close_cb);
    uv_close((uv_handle_t*) &protocol->serverv6, ch_chirp_close_cb);
    uv_timer_stop(&protocol->reconnect_timeout);
    uv_close((uv_handle_t*) &protocol->reconnect_timeout, ch_chirp_close_cb);
    uv_timer_stop(&protocol->gc_timeout);
    uv_close((uv_handle_t*) &protocol->gc_timeout, ch_chirp_close_cb);
    chirp->_->closing_tasks += 4;
    return CH_SUCCESS;
}
// ======
// Reader
// ======
//

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "reader.h" */
/* #include "chirp.h" */
/* #include "common.h" */
/* #include "connection.h" */
/* #include "remote.h" */
/* #include "util.h" */
/* #include "writer.h" */

// Declarations
// ============

// .. c:function::
static void
_ch_rd_handshake(ch_connection_t* conn, ch_buf* buf, size_t read);
//
//    Handle a handshake on the given connection.
//
//    Ensures, that the byte count which shall be read is at least the same as
//    the handshake size.
//
//    The given buffer gets copied into the handshake structure of the reader.
//    Then the port, the maximum time until a timeout happens and the remote
//    identity are applied to the given connection coming from the readers
//    handshake.
//
//    It is then ensured, that the address of the peer connected to the TCP
//    handle (TCP stream) of the connections client can be resolved.
//
//    If the latter was successful, the IP protocol and address are then applied
//    from the resolved address structure to the connection.
//
//    The given connection gets then searched on the protocols pool of
//    connections. If the connection is already known, the found duplicate gets
//    removed from the pool of connections and gets then added to another pool
//    (of the protocol), holding only such old connections.
//
//    Finally, the given connection is added to the protocols pool of
//    connections.
//
//    :param ch_connection_t* conn: Pointer to a connection instance.
//    :param ch_readert* reader:    Pointer to a reader instance. Its handshake
//                                  data structure is the target of this
//                                  handshake
//    :param ch_buf* buf:           Buffer containing bytes read, acts as data
//                                  source
//    :param size_t read:           Count of bytes read

// .. c:function::
static void
_ch_rd_handshake_cb(uv_write_t* req, int status);
//
//    Called when handshake is sent.
//
//    :param uv_write_t* req: Write request type, holding the
//                            connection handle
//    :param int status: Send status
//

// .. c:function::
static void
_ch_rd_handle_msg(
        ch_connection_t* conn, ch_reader_t* reader, ch_message_t* msg);
//
//    Send ack and call message-handler
//
//    :param ch_connection_t* conn:  Pointer to a connection instance.
//    :param ch_reader_t* reader:    Pointer to a reader instance.
//    :param ch_message_t* msg:      Message that was received
//

// .. c:function::
static ssize_t
_ch_rd_read_buffer(
        ch_connection_t* conn,
        ch_reader_t*     reader,
        ch_message_t*    msg,
        ch_buf*          src_buf,
        size_t           to_read,
        char**           assign_buf,
        ch_buf*          dest_buf,
        size_t           dest_buf_size,
        uint32_t         expected,
        int              free_flag,
        ssize_t*         bytes_handled);
//
//    Read data from the read buffer provided by libuv ``src_buf`` to the field
//    of the message ``assign_buf``. A preallocated buffer will be used if the
//    message is small enough, otherwise a buffer will be allocated. The
//    function also handles partial reads.

// .. c:function::
static inline ssize_t
_ch_rd_read_step(
        ch_connection_t* conn,
        ch_buf*          buf,
        size_t           bytes_read,
        ssize_t          bytes_handled,
        int*             stop,
        int*             cont);
//
//    One step in the reader state machine.
//
//    :param ch_connection_t* conn: Connection the data was read from.
//    :param void* buffer:          The buffer containing ``read`` bytes read.
//    :param size_t bytes_read:     The bytes read.
//    :param size_t bytes_handled:  The bytes handled in the last step
//    :param int* stop:             (Out) Stop the reading process.
//    :param int* cont:             (Out) Request continuation
//

// .. c:function::
static inline ch_error_t
_ch_rd_verify_msg(ch_connection_t* conn, ch_message_t* msg);
//
//    One step in the reader state machine.
//
//    :param ch_connection_t* conn: Connection the message came from
//    :param ch_message_t* msg:     Message to verify

//
// Definitions
// ===========

// .. c:type:: _ch_rd_state_names
//
//    Names of reader states for logging.
//
// .. code-block:: cpp
//
char* _ch_rd_state_names[] = {
        "CH_RD_START",
        "CH_RD_HANDSHAKE",
        "CH_RD_WAIT",
        "CH_RD_SLOT",
        "CH_RD_HEADER",
        "CH_RD_DATA",
};

// .. c:function::
static void
_ch_rd_handshake(ch_connection_t* conn, ch_buf* buf, size_t read)
//    :noindex:
//
//    see: :c:func:`_ch_rd_handshake`
//
// .. code-block:: cpp
//
{
    ch_remote_t       search_remote;
    ch_connection_t*  old_conn = NULL;
    ch_connection_t*  tmp_conn = NULL;
    ch_chirp_t*       chirp    = conn->chirp;
    ch_remote_t*      remote   = NULL;
    ch_chirp_int_t*   ichirp   = chirp->_;
    ch_protocol_t*    protocol = &ichirp->protocol;
    ch_sr_handshake_t hs_tmp;
    uv_timer_stop(&conn->connect_timeout);
    conn->flags |= CH_CN_CONNECTED;
    if (conn->flags & CH_CN_INCOMING) {
        A(ch_cn_delete(&protocol->handshake_conns, conn, &tmp_conn) == 0,
          "Handshake should be tracked");
        A(conn == tmp_conn, "Deleted wrong connection");
    }
    if (read < CH_SR_HANDSHAKE_SIZE) {
        EC(chirp,
           "Illegal handshake size -> shutdown. ",
           "ch_connection_t:%p",
           (void*) conn);
        ch_cn_shutdown(conn, CH_PROTOCOL_ERROR);
        return;
    }
    ch_sr_buf_to_hs(buf, &hs_tmp);
    conn->port = hs_tmp.port;
    memcpy(conn->remote_identity, hs_tmp.identity, CH_ID_SIZE);
    ch_rm_init_from_conn(chirp, &search_remote, conn, 0);
    if (ch_rm_find(protocol->remotes, &search_remote, &remote) != CH_SUCCESS) {
        remote = ch_alloc(sizeof(*remote));
        LC(chirp, "Remote allocated", "ch_remote_t:%p", remote);
        if (remote == NULL) {
            ch_cn_shutdown(conn, CH_ENOMEM);
            return;
        }
        *remote     = search_remote;
        int tmp_err = ch_rm_insert(&protocol->remotes, remote);
        A(tmp_err == 0, "Inserting remote failed");
        (void) (tmp_err);
    }
    conn->remote = remote;
    /* If there is a network race condition we replace the old connection and
     * leave the old one for garbage collection. */
    old_conn     = remote->conn;
    remote->conn = conn;
    /* By definition, the connection that last completes the handshake is the
     * new one, the other the old one. Since we store outgoing connections at
     * the moment we connect, both connections might fight the place in
     * old_conns. It would be possible to prevent this, but removing the new
     * connection from old_conns is easier and just as correct. */
    ch_cn_delete(&protocol->old_connections, conn, &tmp_conn);
    if (old_conn != NULL) {
        /* If we found the current connection everything is ok */
        if (conn != old_conn) {
            L(chirp,
              "ch_connection_t:%p replaced ch_connection_t:%p",
              (void*) conn,
              (void*) old_conn);
            ch_cn_insert(&protocol->old_connections, old_conn);
        }
    }
#ifdef CH_ENABLE_LOGGING
    {
        ch_text_address_t addr;
        uint8_t*          identity = conn->remote_identity;
        char              id[CH_ID_SIZE * 2 + 1];
        uv_inet_ntop(conn->ip_protocol, conn->address, addr.data, sizeof(addr));
        ch_bytes_to_hex(identity, sizeof(identity), id, sizeof(id));
        LC(chirp,
           "Handshake with remote %s:%d (%s) done. ",
           "ch_connection_t:%p",
           addr.data,
           conn->port,
           id,
           (void*) conn);
    }
#endif
    ch_message_t* ack_msg = &conn->ack_msg;
    memcpy(ack_msg->address, conn->address, CH_IP_ADDR_SIZE);
    ack_msg->ip_protocol = conn->ip_protocol;
    ack_msg->port        = conn->port;
    ack_msg->type        = CH_MSG_ACK;
    ack_msg->header_len  = 0;
    ack_msg->data_len    = 0;
    ack_msg->_pool       = conn;
    A(conn->remote != NULL, "The remote has to be set");
    ch_wr_process_queues(conn->remote);
}

// .. c:function::
static void
_ch_rd_handle_msg(ch_connection_t* conn, ch_reader_t* reader, ch_message_t* msg)
//    :noindex:
//
//    see: :c:func:`_ch_rd_handle_msg`
//
// .. code-block:: cpp
//
{
    ch_chirp_t*     chirp  = conn->chirp;
    ch_chirp_int_t* ichirp = chirp->_;
#ifdef CH_ENABLE_LOGGING
    {
        ch_text_address_t addr;
        uint8_t*          identity = conn->remote_identity;
        char              id[CH_ID_SIZE * 2 + 1];
        uv_inet_ntop(conn->ip_protocol, conn->address, addr.data, sizeof(addr));
        ch_bytes_to_hex(identity, sizeof(identity), id, sizeof(id));
        LC(chirp,
           "Read message with id: %s\n"
           "                             "
           "serial:%u\n"
           "                             "
           "from %s:%d type:%d data_len:%u. ",
           "ch_connection_t:%p",
           id,
           msg->serial,
           addr.data,
           conn->port,
           msg->type,
           msg->data_len,
           (void*) conn);
    }
#endif

    reader->state   = CH_RD_WAIT;
    reader->slot    = NULL;
    conn->timestamp = uv_now(ichirp->loop);
    if (conn->remote != NULL) {
        conn->remote->timestamp = conn->timestamp;
    }

    /* Only increase refcnt if we know ch_chirp_release_msg_slot is called */
    reader->pool->refcnt += 1;
    if (ichirp->recv_cb != NULL) {
        ichirp->recv_cb(chirp, msg);
    } else {
        ch_chirp_release_msg_slot(chirp, msg, NULL);
    }
}

// .. c:function::
static void
_ch_rd_handshake_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_rd_handshake_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn  = req->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    if (status < 0) {
        LC(chirp,
           "Sending handshake failed. ",
           "ch_connection_t:%p",
           (void*) conn);
        ch_cn_shutdown(conn, CH_WRITE_ERROR);
        return;
    }
#ifndef CH_WITHOUT_TLS
    /* Check if we already have a message (just after handshake)
     * this is here so we have no overlapping ch_cn_write. If the read causes a
     * ack message to be sent and the write of the handshake is not finished,
     * chirp would assert or be in undefined state. */
    if (conn->flags & CH_CN_ENCRYPTED) {
        int stop;
        ch_pr_decrypt_read(conn, &stop);
    }
#endif
}

static ssize_t
_ch_rd_read_buffer(
        ch_connection_t* conn,
        ch_reader_t*     reader,
        ch_message_t*    msg,
        ch_buf*          src_buf,
        size_t           to_read,
        char**           assign_buf,
        ch_buf*          dest_buf,
        size_t           dest_buf_size,
        uint32_t         expected,
        int              free_flag,
        ssize_t*         bytes_handled)
//    :noindex:
//
//    see: :c:func:`_ch_rd_read_buffer`
//
// .. code-block:: cpp
//
{
    if (reader->bytes_read == 0) {
        if (expected <= dest_buf_size) {
            /* Preallocated buf is large enough */
            *assign_buf = dest_buf;
        } else {
            *assign_buf = ch_alloc(expected);
            if (*assign_buf == NULL) {
                EC(conn->chirp,
                   "Could not allocate memory for message. ",
                   "ch_connection_t:%p",
                   (void*) conn);
                ch_cn_shutdown(conn, CH_ENOMEM);
                return CH_ENOMEM;
            }
            msg->_flags |= free_flag;
        }
    }
    if ((to_read + reader->bytes_read) >= expected) {
        /* We can read everything */
        size_t reading = expected - reader->bytes_read;
        memcpy(*assign_buf + reader->bytes_read, src_buf, reading);
        *bytes_handled += reading;
        reader->bytes_read = 0; /* Reset partial buffer reads */
        return CH_SUCCESS;
    } else {
        /* Only partial read possible */
        memcpy(*assign_buf + reader->bytes_read, src_buf, to_read);
        *bytes_handled += to_read;
        reader->bytes_read += to_read;
        return CH_MORE;
    }
}

// .. c:function::
static inline ssize_t
_ch_rd_read_step(
        ch_connection_t* conn,
        ch_buf*          buf,
        size_t           bytes_read,
        ssize_t          bytes_handled,
        int*             stop,
        int*             cont)
//    :noindex:
//
//    see: :c:func:`ch_rd_free`
//
// .. code-block:: cpp
//
{
    ch_message_t*   msg;
    ch_bf_slot_t*   slot;
    ch_chirp_t*     chirp   = conn->chirp;
    ch_chirp_int_t* ichirp  = chirp->_;
    ch_reader_t*    reader  = &conn->reader;
    int             to_read = bytes_read - bytes_handled;

    LC(chirp,
       "Reader state: %s. ",
       "ch_connection_t:%p",
       _ch_rd_state_names[reader->state],
       (void*) conn);

    switch (reader->state) {
    case CH_RD_START: {
        ch_sr_handshake_t hs_tmp;
        ch_buf            hs_buf[CH_SR_HANDSHAKE_SIZE];
        hs_tmp.port = ichirp->public_port;
        memcpy(hs_tmp.identity, ichirp->identity, CH_ID_SIZE);
        ch_sr_hs_to_buf(&hs_tmp, hs_buf);
        uv_buf_t buf;
        buf.base = hs_buf;
        buf.len  = CH_SR_HANDSHAKE_SIZE;
        ch_cn_write(conn, &buf, 1, _ch_rd_handshake_cb);
        reader->state = CH_RD_HANDSHAKE;
        break;
    }
    case CH_RD_HANDSHAKE: {
        if (bytes_read == 0)
            return -1;
        /* We expect that complete handshake arrives at once,
         * check in _ch_rd_handshake */
        _ch_rd_handshake(conn, buf + bytes_handled, to_read);
        bytes_handled += sizeof(ch_sr_handshake_t);
        reader->state = CH_RD_WAIT;
        break;
    }
    case CH_RD_WAIT: {
        if (bytes_read == 0)
            return -1;
        ch_message_t* wire_msg = &reader->wire_msg;
        ssize_t       reading  = CH_SR_WIRE_MESSAGE_SIZE - reader->bytes_read;
        if (to_read >= reading) {
            /* We can read everything */
            memcpy(reader->net_msg + reader->bytes_read,
                   buf + bytes_handled,
                   reading);
            reader->bytes_read = 0; /* Reset partial buffer reads */
            bytes_handled += reading;
        } else {
            memcpy(reader->net_msg + reader->bytes_read,
                   buf + bytes_handled,
                   to_read);
            reader->bytes_read += to_read;
            bytes_handled += to_read;
            return bytes_handled;
        }
        ch_sr_buf_to_msg(reader->net_msg, wire_msg);
        int tmp_err = _ch_rd_verify_msg(conn, wire_msg);
        if (tmp_err != CH_SUCCESS) {
            ch_cn_shutdown(conn, tmp_err);
            return -1; /* Shutdown */
        }
        if (wire_msg->type & CH_MSG_NOOP) {
            LC(chirp, "Received NOOP.", "ch_connection_t", conn);
            conn->timestamp = uv_now(ichirp->loop);
            if (conn->remote != NULL) {
                conn->remote->timestamp = conn->timestamp;
            }
            break;
        } else if (wire_msg->type & CH_MSG_ACK) {
            ch_message_t* wam = conn->remote->wait_ack_message;
            /* Since we abort wam on shutdown, we can receive acks for a old
             * wam */
            if (wam != NULL) {
                if (memcmp(wam->identity, wire_msg->identity, CH_ID_SIZE) ==
                    0) {
                    wam->_flags |= CH_MSG_ACK_RECEIVED;
                    conn->remote->wait_ack_message = NULL;
                    ch_chirp_finish_message(chirp, conn, wam, CH_SUCCESS);
                }
            }
            break;
        } else {
            reader->state = CH_RD_SLOT;
        }
        /* Since we do not read any data in CH_RD_SLOT, we need to ask for
         * continuation. I wanted to solve this with a fall-though-case, but
         * linters and compilers are complaining. */
        *cont = 1;
        break;
    }
    case CH_RD_SLOT: {
        ch_message_t* wire_msg = &reader->wire_msg;
        if (reader->slot == NULL) {
            reader->slot = ch_bf_acquire(reader->pool);
            if (reader->slot == NULL) {
                LC(chirp, "Stop reading", "ch_connection_t:%p", conn);
                if (!(conn->flags & CH_CN_STOPPED)) {
                    LC(chirp, "Stop stream", "ch_connection_t:%p", conn);
                    uv_read_stop((uv_stream_t*) &conn->client);
                }
                conn->flags |= CH_CN_STOPPED;
                *stop = 1;
                return bytes_handled;
            }
        }
        slot = reader->slot;
        msg  = &slot->msg;
        /* Copy the wire message */
        memcpy(msg, wire_msg, ((char*) &wire_msg->header) - ((char*) wire_msg));
        msg->ip_protocol = conn->ip_protocol;
        msg->port        = conn->port;
        memcpy(msg->remote_identity, conn->remote_identity, CH_ID_SIZE);
        memcpy(msg->address,
               conn->address,
               (msg->ip_protocol == AF_INET6) ? CH_IP_ADDR_SIZE
                                              : CH_IP4_ADDR_SIZE);
        if (msg->type & CH_MSG_REQ_ACK) {
            msg->_flags |= CH_MSG_SEND_ACK;
        }
        /* Direct jump to next read state */
        if (msg->header_len > 0) {
            reader->state = CH_RD_HEADER;
        } else if (msg->data_len > 0) {
            reader->state = CH_RD_DATA;
        } else {
            _ch_rd_handle_msg(conn, reader, msg);
        }
        break;
    }
    case CH_RD_HEADER: {
        if (bytes_read == 0)
            return -1;
        slot = reader->slot;
        msg  = &slot->msg;
        if (_ch_rd_read_buffer(
                    conn,
                    reader,
                    msg,
                    buf + bytes_handled,
                    to_read,
                    &msg->header,
                    slot->header,
                    CH_BF_PREALLOC_HEADER,
                    msg->header_len,
                    CH_MSG_FREE_HEADER,
                    &bytes_handled) != CH_SUCCESS) {
            return -1; /* Shutdown */
        }
        /* Direct jump to next read state */
        if (msg->data_len > 0) {
            reader->state = CH_RD_DATA;
        } else {
            _ch_rd_handle_msg(conn, reader, msg);
        }
        break;
    }
    case CH_RD_DATA: {
        if (bytes_read == 0)
            return -1;
        slot = reader->slot;
        msg  = &slot->msg;
        if (_ch_rd_read_buffer(
                    conn,
                    reader,
                    msg,
                    buf + bytes_handled,
                    to_read,
                    &msg->data,
                    slot->data,
                    CH_BF_PREALLOC_DATA,
                    msg->data_len,
                    CH_MSG_FREE_DATA,
                    &bytes_handled) != CH_SUCCESS) {
            return -1; /* Shutdown */
        }
        _ch_rd_handle_msg(conn, reader, msg);
        break;
    }
    default:
        A(0, "Unknown reader state");
        break;
    }
    return bytes_handled;
}

// .. c:function::
static inline ch_error_t
_ch_rd_verify_msg(ch_connection_t* conn, ch_message_t* msg)
//    :noindex:
//
//    see: :c:func:`_ch_rd_verify_msg`
//
// .. code-block:: cpp
//
{
    ch_chirp_t*     chirp               = conn->chirp;
    ch_chirp_int_t* ichirp              = chirp->_;
    uint32_t        total_wire_msg_size = msg->header_len + msg->data_len;
    if (total_wire_msg_size > ichirp->config.MAX_MSG_SIZE) {
        EC(chirp,
           "Message size exceeds hardlimit. ",
           "ch_connection_t:%p",
           (void*) conn);
        return CH_ENOMEM;
    }
    if ((msg->type & CH_MSG_ACK) || (msg->type & CH_MSG_NOOP)) {
        if (msg->header_len != 0 || msg->data_len != 0) {
            EC(chirp,
               "A ack/noop may not have header or data set. ",
               "ch_connection_t:%p",
               (void*) conn);
            return CH_PROTOCOL_ERROR;
        }
        if (msg->type & CH_MSG_REQ_ACK) {
            EC(chirp,
               "A ack/noop may not require an ack. ",
               "ch_connection_t:%p",
               (void*) conn);
            return CH_PROTOCOL_ERROR;
        }
    }
    return CH_SUCCESS;
}

// .. c:function::
void
ch_rd_free(ch_reader_t* reader)
//    :noindex:
//
//    see: :c:func:`ch_rd_free`
//
// .. code-block:: cpp
//
{
    /* Remove the reference to connection, since it is now invalid */
    reader->pool->conn = NULL;
    ch_bf_free(reader->pool);
}

// .. c:function::
ch_error_t
ch_rd_init(ch_reader_t* reader, ch_connection_t* conn, ch_chirp_int_t* ichirp)
//    :noindex:
//
//    see: :c:func:`ch_rd_init`
//
// .. code-block:: cpp
//
{
    reader->state = CH_RD_START;
    reader->pool  = ch_alloc(sizeof(*reader->pool));
    if (reader->pool == NULL) {
        return CH_ENOMEM;
    }
    return ch_bf_init(reader->pool, conn, ichirp->config.MAX_SLOTS);
}

// .. c:function::
ssize_t
ch_rd_read(ch_connection_t* conn, ch_buf* buf, size_t bytes_read, int* stop)
//    :noindex:
//
//    see: :c:func:`ch_rd_read`
//
// .. code-block:: cpp
//
{
    *stop = 0;

    /* Bytes handled is used when multiple writes (of the remote) come in a
     * single read and the reader switches between various states as for
     * example CH_RD_HANDSHAKE, CH_RD_WAIT or CH_RD_HEADER. */
    ssize_t bytes_handled = 0;
    int     cont;

    /* Ignore reads while shutting down */
    if (conn->flags & CH_CN_SHUTTING_DOWN) {
        return bytes_read;
    }

    do {
        cont          = 0;
        bytes_handled = _ch_rd_read_step(
                conn, buf, bytes_read, bytes_handled, stop, &cont);
        if (*stop || bytes_handled == -1) {
            return bytes_handled;
        }
    } while (bytes_handled < (ssize_t) bytes_read || cont);
    return bytes_handled;
}
// ======
// Remote
// ======
//

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "remote.h" */
/* #include "chirp.h" */
/* #include "util.h" */

// rbtree prototypes
// =================
//
// .. c:function::
static int
ch_remote_cmp(ch_remote_t* x, ch_remote_t* y)
//
//    Compare operator for connections.
//
//    :param ch_remote_t* x: First remote instance to compare
//    :param ch_remote_t* y: Second remote instance to compare
//
//    :return: the comparision between
//                 - the IP protocols, if they are not the same, or
//                 - the addresses, if they are not the same, or
//                 - the ports
//    :rtype: int
//
// .. code-block:: cpp
//
{
    if (x->ip_protocol != y->ip_protocol) {
        return x->ip_protocol - y->ip_protocol;
    } else {
        int tmp_cmp =
                memcmp(x->address,
                       y->address,
                       x->ip_protocol == AF_INET6 ? CH_IP_ADDR_SIZE
                                                  : CH_IP4_ADDR_SIZE);
        if (tmp_cmp != 0) {
            return tmp_cmp;
        } else {
            return x->port - y->port;
        }
    }
}

rb_bind_impl_m(ch_rm, ch_remote_t) CH_ALLOW_NL;

// stack prototypes
// ================
//
// .. code-block:: cpp

qs_stack_bind_impl_m(ch_rm_st, ch_remote_t) CH_ALLOW_NL;

// Definitions
// ===========
//
// .. code-block:: cpp

static void
_ch_rm_init(ch_chirp_t* chirp, ch_remote_t* remote, int key)
//
//    Initialize remote
//
// .. code-block:: cpp
//
{
    memset(remote, 0, sizeof(*remote));
    ch_rm_node_init(remote);
    remote->chirp = chirp;
    if (!key) {
        ch_random_ints_as_bytes(
                (uint8_t*) &remote->serial, sizeof(remote->serial));
        remote->timestamp = uv_now(chirp->_->loop);
    }
}

// .. c:function::
void
ch_rm_init_from_msg(
        ch_chirp_t* chirp, ch_remote_t* remote, ch_message_t* msg, int key)
//    :noindex:
//
//    see: :c:func:`ch_rm_init_from_msg`
//
// .. code-block:: cpp
//
{
    _ch_rm_init(chirp, remote, key);
    remote->ip_protocol = msg->ip_protocol;
    remote->port        = msg->port;
    memcpy(&remote->address, &msg->address, CH_IP_ADDR_SIZE);
    remote->conn = NULL;
}

// .. c:function::
void
ch_rm_init_from_conn(
        ch_chirp_t* chirp, ch_remote_t* remote, ch_connection_t* conn, int key)
//    :noindex:
//
//    see: :c:func:`ch_rm_init_from_conn`
//
// .. code-block:: cpp
//
{
    _ch_rm_init(chirp, remote, key);
    remote->ip_protocol = conn->ip_protocol;
    remote->port        = conn->port;
    memcpy(&remote->address, &conn->address, CH_IP_ADDR_SIZE);
    remote->conn = NULL;
}

// .. c:function::
void
ch_rm_free(ch_remote_t* remote)
//    :noindex:
//
//    see: :c:func:`ch_rm_free`
//
// .. code-block:: cpp
//
{
    LC(remote->chirp, "Remote freed", "ch_remote_t:%p", remote);
    if (remote->noop != NULL) {
        ch_free(remote->noop);
    }
    ch_free(remote);
}
// ==========
// Serializer
// ==========
//

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "serializer.h" */

// Definitions
// ===========
//
// .. code-block:: cpp

// .. c:function::
int
ch_sr_buf_to_msg(ch_buf* buf, ch_message_t* msg)
//    :noindex:
//
//    see: :c:func:`ch_sr_buf_to_msg`
//
// .. code-block:: cpp
//
{
    CH_SR_WIRE_MESSAGE_LAYOUT;

    memcpy(msg->identity, identity, CH_ID_SIZE);

    msg->type       = *type;
    msg->header_len = ntohs(*header_len);
    msg->data_len   = ntohl(*data_len);
    msg->serial     = ntohl(*serial);
    return CH_SR_WIRE_MESSAGE_SIZE;
}

// .. c:function::
int
ch_sr_msg_to_buf(ch_message_t* msg, ch_buf* buf, uint32_t msg_serial)
//    :noindex:
//
//    see: :c:func:`ch_sr_msg_to_buf`
//
// .. code-block:: cpp
//
{
    CH_SR_WIRE_MESSAGE_LAYOUT;

    memcpy(identity, msg->identity, CH_ID_SIZE);

    *type       = msg->type;
    *header_len = htons(msg->header_len);
    *data_len   = htonl(msg->data_len);
    *serial     = htonl(msg_serial);
    return CH_SR_WIRE_MESSAGE_SIZE;
}

// .. c:function::
int
ch_sr_buf_to_hs(ch_buf* buf, ch_sr_handshake_t* hs)
//    :noindex:
//
//    see: :c:func:`ch_sr_buf_to_hs`
//
// .. code-block:: cpp
//
{
    CH_SR_HANDSHAKE_LAYOUT;

    hs->port = ntohs(*port);
    memcpy(hs->identity, identity, CH_ID_SIZE);

    return CH_SR_HANDSHAKE_SIZE;
}

// .. c:function::
int
ch_sr_hs_to_buf(ch_sr_handshake_t* hs, ch_buf* buf)
//    :noindex:
//
//    see: :c:func:`ch_sr_hs_to_buf`
//
// .. code-block:: cpp
//

{
    CH_SR_HANDSHAKE_LAYOUT;

    *port = htons(hs->port);
    memcpy(identity, hs->identity, CH_ID_SIZE);

    return CH_SR_HANDSHAKE_SIZE;
}
// ====
// Util
// ====
//
// Common utility functions.
//

// Project includes
// ================
//
// .. code-block:: cpp

/* #include "util.h" */
/* #include "chirp.h" */
/* #include "rbtree.h" */

// System includes
// ===============
//
// .. code-block:: cpp

#include <stdarg.h>

// Declarations
// ============
//
// .. code-block:: cpp
//

static int             _ch_always_encrypt = 0;
static ch_free_cb_t    _ch_free_cb        = free;
static ch_alloc_cb_t   _ch_alloc_cb       = malloc;
static ch_realloc_cb_t _ch_realloc_cb     = realloc;

// Logging and assert macros
// =========================
//
// Colors
// ------
//
// .. code-block:: cpp

static char* const _ch_lg_reset     = "\x1B[0m";
static char* const _ch_lg_err       = "\x1B[1;31m";
static char* const _ch_lg_colors[8] = {
        "\x1B[0;34m",
        "\x1B[0;32m",
        "\x1B[0;36m",
        "\x1B[0;33m",
        "\x1B[1;34m",
        "\x1B[1;32m",
        "\x1B[1;36m",
        "\x1B[1;33m",
};

#ifdef CH_ENABLE_ASSERTS

// Debug alloc tracking
// ====================
//
// Since we can't use valgrind and rr at the same time and I needed to debug a
// leak with rr, I added this memory leak debugging code. Since we use rr, the
// pointer to the allocation is enough, we don't need any meta information.

// .. c:var:: uv_mutex_t _ch_at_lock
//
//    Lock for alloc tracking
//
// .. code-block:: cpp
//
static uv_mutex_t _ch_at_lock;

struct ch_alloc_track_s;
typedef struct ch_alloc_track_s ch_alloc_track_t;
struct ch_alloc_track_s {
    void*             buf;
    char              color;
    ch_alloc_track_t* parent;
    ch_alloc_track_t* left;
    ch_alloc_track_t* right;
};

#define _ch_at_cmp_m(x, y) rb_safe_cmp_m(x->buf, y->buf)

rb_bind_m(_ch_at, ch_alloc_track_t) CH_ALLOW_NL;

ch_alloc_track_t* _ch_alloc_tree;

// .. c:function::
int
ch_at_allocated(void* buf)
//    :noindex:
//
//    see: :c:func:`ch_at_allocated`
//
// .. code-block:: cpp
//
{
    ch_alloc_track_t  key;
    ch_alloc_track_t* value;
    key.buf = buf;
    uv_mutex_lock(&_ch_at_lock);
    int ret = _ch_at_find(_ch_alloc_tree, &key, &value) == CH_SUCCESS;
    uv_mutex_unlock(&_ch_at_lock);
    return ret;
}

// .. c:function::
static void*
_ch_at_alloc(void* buf)
//
//    Track a memory allocation.
//
//    :param void* buf: Pointer to the buffer to track.
//
// .. code-block:: cpp
//
{
    /* We do not track failed allocations */
    if (buf == NULL) {
        return buf;
    }
    ch_alloc_track_t* track = _ch_alloc_cb(sizeof(*track));
    assert(track);
    /* We treat a failure to track as an alloc failure. This code is here
     * if we ever need to use this in release mode (or just for
     * correctness). */
    if (track == NULL) {
        return NULL;
    }
    _ch_at_node_init(track);
    track->buf = buf;
    uv_mutex_lock(&_ch_at_lock);
    int ret = _ch_at_insert(&_ch_alloc_tree, track);
    uv_mutex_unlock(&_ch_at_lock);
    assert(ret == 0);
    return buf;
}

// .. c:function::
void
ch_at_cleanup(void)
//    :noindex:
//
//    see: :c:func:`ch_at_cleanup`
//
// .. code-block:: cpp
//
{
    uv_mutex_lock(&_ch_at_lock);
    if (_ch_alloc_tree != _ch_at_nil_ptr) {
        fprintf(stderr, "Leaked allocations: \n");
        while (_ch_alloc_tree != _ch_at_nil_ptr) {
            ch_alloc_track_t* item;
            item = _ch_alloc_tree;
            fprintf(stderr, "%p ", item->buf);
            _ch_at_delete_node(&_ch_alloc_tree, item);
            _ch_free_cb(item);
        }
        fprintf(stderr, "\n");
        A(0, "There is a memory leak")
    }
    uv_mutex_unlock(&_ch_at_lock);
    uv_mutex_destroy(&_ch_at_lock);
}

// .. c:function::
void
ch_at_init(void)
//    :noindex:
//
//    see: :c:func:`ch_at_init`
//
// .. code-block:: cpp
//
{
    A(uv_mutex_init(&_ch_at_lock) == 0, "Failed to initialize mutex");
    uv_mutex_lock(&_ch_at_lock);
    _ch_at_tree_init(&_ch_alloc_tree);
    uv_mutex_unlock(&_ch_at_lock);
}

// .. c:function::
static void
_ch_at_free(void* buf)
//
//    Track a freed memory allocation.
//
//    :param void* buf: Pointer to the buffer to track.
//
// .. code-block:: cpp
//
{
    ch_alloc_track_t  key;
    ch_alloc_track_t* track;
    key.buf = buf;
    uv_mutex_lock(&_ch_at_lock);
    int ret = _ch_at_delete(&_ch_alloc_tree, &key, &track);
    uv_mutex_unlock(&_ch_at_lock);
    assert(ret == 0);
    _ch_free_cb(track);
}

// .. c:function::
static void*
_ch_at_realloc(void* buf, void* rbuf)
//
//    Track a memory reallocation.
//
//    :param void* buf: Pointer to the buffer that has been reallocated.
//    :param void* rbuf: Pointer to the new buffer.
//
// .. code-block:: cpp
//
{
    /* We do not track failed reallocations */
    if (rbuf == NULL) {
        return rbuf;
    }
    ch_alloc_track_t  key;
    ch_alloc_track_t* track;
    /* Shortcut if the allocator was able to extend the allocation */
    if (buf == rbuf) {
        return rbuf;
    }
    key.buf = buf;
    uv_mutex_lock(&_ch_at_lock);
    int ret = _ch_at_delete(&_ch_alloc_tree, &key, &track);
    assert(ret == 0);
    track->buf = rbuf;
    ret        = _ch_at_insert(&_ch_alloc_tree, track);
    uv_mutex_unlock(&_ch_at_lock);
    assert(ret == 0);
    return rbuf;
}
#endif

// Definitions
// ===========

// .. c:function::
void*
ch_alloc(size_t size)
//    :noindex:
//
//    see: :c:func:`ch_alloc`
//
// .. code-block:: cpp
//
{
    void* buf = _ch_alloc_cb(size);
    /* Assert memory (do not rely on this, implement it robust: be graceful and
     * return error to user) */
    A(buf, "Allocation failure");
#ifdef CH_ENABLE_ASSERTS
    return _ch_at_alloc(buf);
#else
    return buf;
#endif
}

// .. c:function::
void
ch_bytes_to_hex(uint8_t* bytes, size_t bytes_size, char* str, size_t str_size)
//    :noindex:
//
//    see: :c:func:`ch_bytes_to_hex`
//
// .. code-block:: cpp
//
{
    size_t i;
    A(bytes_size * 2 + 1 <= str_size, "Not enough space for string");
    (void) (str_size);
    for (i = 0; i < bytes_size; i++) {
        snprintf(str, 3, "%02X", bytes[i]);
        str += 2;
    }
    *str = 0;
}

// .. c:function::
void
ch_chirp_set_always_encrypt()
//    :noindex:
//
//    see: :c:func:`ch_chirp_set_always_encrypt`
//
// .. code-block:: cpp
//
{
    _ch_always_encrypt = 1;
}

// .. c:function::
void
ch_free(void* buf)
//    :noindex:
//
//    see: :c:func:`ch_free`
//
// .. code-block:: cpp
//
{
#ifdef CH_ENABLE_ASSERTS
    _ch_at_free(buf);
#endif
    _ch_free_cb(buf);
}

// .. c:function::
int
ch_is_local_addr(ch_text_address_t* addr)
//    :noindex:
//
//    see: :c:func:`ch_is_local_addr`
//
// .. code-block:: cpp
//
{
    if (_ch_always_encrypt) {
        return 0;
    } else {
        return (strncmp("::1", addr->data, sizeof(addr->data)) == 0 ||
                strncmp("127.0.0.1", addr->data, sizeof(addr->data)) == 0);
    }
}

// .. c:function::
void
ch_random_ints_as_bytes(uint8_t* bytes, size_t len)
//    :noindex:
//
//    see: :c:func:`ch_random_ints_as_bytes`
//
// .. code-block:: cpp
//
{
    size_t i;
    int    tmp_rand;
    A(len % 4 == 0, "len must be multiple of four");
#ifdef _WIN32
#if RAND_MAX < 16384 || INT_MAX < 16384 // 2**14
#error Seriously broken compiler or platform
#else  // RAND_MAX < 16384 || INT_MAX < 16384
    for (i = 0; i < len; i += 2) {
        tmp_rand = rand();
        memcpy(bytes + i, &tmp_rand, 2);
    }
#endif // RAND_MAX < 16384 || INT_MAX < 16384
#else  // _WIN32
#if RAND_MAX < 1073741824 || INT_MAX < 1073741824 // 2**30
#ifdef CH_ACCEPT_STRANGE_PLATFORM
    /* WTF, fallback platform */
    (void) (tmp_rand);
    for (i = 0; i < len; i++) {
        bytes[i] = ((unsigned int) rand()) % 256;
    }
#else // ACCEPT_STRANGE_PLATFORM
/* cppcheck-suppress preprocessorErrorDirective */
#error Unexpected RAND_MAX / INT_MAX, define CH_ACCEPT_STRANGE_PLATFORM
#endif // ACCEPT_STRANGE_PLATFORM
#else  // RAND_MAX < 1073741824 || INT_MAX < 1073741824
    /* Tested: this is 4 times faster*/
    for (i = 0; i < len; i += 4) {
        tmp_rand = rand();
        memcpy(bytes + i, &tmp_rand, 4);
    }
#endif // RAND_MAX < 1073741824 || INT_MAX < 1073741824
#endif // _WIN32
}

// .. c:function::
void*
ch_realloc(void* buf, size_t size)
//    :noindex:
//
//    see: :c:func:`ch_realloc`
//
// .. code-block:: cpp
//
{
    void* rbuf = _ch_realloc_cb(buf, size);
    /* Assert memory (do not rely on this, implement it robust: be graceful and
     * return error to user) */
    A(rbuf, "Reallocation failure");
#ifdef CH_ENABLE_ASSERTS
    return _ch_at_realloc(buf, rbuf);
#else
    return rbuf;
#endif
}

// .. c:function::
CH_EXPORT
void
ch_set_alloc_funcs(
        ch_alloc_cb_t alloc, ch_realloc_cb_t realloc, ch_free_cb_t free)
//    :noindex:
//
//    see: :c:func:`ch_set_alloc_funcs`
//
// .. code-block:: cpp
//
{
    _ch_alloc_cb   = alloc;
    _ch_realloc_cb = realloc;
    _ch_free_cb    = free;
}

// .. c:function::
ch_error_t
ch_textaddr_to_sockaddr(
        int                      af,
        ch_text_address_t*       text,
        uint16_t                 port,
        struct sockaddr_storage* addr)
//    :noindex:
//
//    see: :c:func:`ch_textaddr_to_sockaddr`
//
// .. code-block:: cpp
//
{
    if (af == AF_INET6) {
        return uv_ip6_addr(text->data, port, (struct sockaddr_in6*) addr);
    } else {
        A(af == AF_INET, "Unknown IP protocol");
        return uv_ip4_addr(text->data, port, (struct sockaddr_in*) addr);
    }
}

// .. c:function::
CH_EXPORT
void
ch_write_log(
        ch_chirp_t* chirp,
        char*       file,
        int         line,
        char*       message,
        char*       clear,
        int         error,
        ...)
//    :noindex:
//
//    see: :c:func:`ch_write_log`
//
// .. code-block:: cpp
//
{
    va_list args;
    va_start(args, error);
    char* tfile = strrchr(file, '/');
    if (tfile != NULL) {
        file = tfile + 1;
    }
    char buf1[1024];
    if (chirp->_log != NULL) {
        char buf2[1024];
        snprintf(buf1, 1024, "%s:%5d %s %s", file, line, message, clear);
        vsnprintf(buf2, 1024, buf1, args);
        chirp->_log(buf2, error);
    } else {
        uint8_t log_id = ((uint8_t) chirp->_->identity[0]) % 8;
        char*   tmpl;
        char*   first;
        char*   second;
        if (error) {
            tmpl   = "%s%02X%02X%s %17s:%5d Error: %s%s %s%s\n";
            first  = _ch_lg_err;
            second = _ch_lg_err;
        } else {
            tmpl   = "%s%02X%02X%s %17s:%5d %s%s %s%s\n";
            first  = _ch_lg_colors[log_id];
            second = _ch_lg_reset;
        }
        /* From doc: If the format is exhausted while arguments remain, the
         * excess arguments shall be evaluated but are otherwise ignored. */
        snprintf(
                buf1,
                1024,
                tmpl,
                first,
                chirp->_->identity[0],
                chirp->_->identity[1],
                second,
                file,
                line,
                _ch_lg_colors[log_id],
                message,
                _ch_lg_reset,
                clear);
        vfprintf(stderr, buf1, args);
        fflush(stderr);
    }
    va_end(args);
}
// ======
// Writer
// ======
//

// Project includes
// ================
//
// .. code-block:: cpp
//
/* #include "writer.h" */
/* #include "chirp.h" */
/* #include "common.h" */
/* #include "protocol.h" */
/* #include "remote.h" */
/* #include "util.h" */

// Declarations
// ============
//
// .. code-block:: cpp

MINMAX_FUNCS(uint64_t)

// .. c:function::
static int
_ch_wr_check_write_error(
        ch_chirp_t*      chirp,
        ch_writer_t*     writer,
        ch_connection_t* conn,
        int              status);
//
//    Check if the given status is erroneous (that is, there was an error
//    during writing) and cleanup if necessary. Cleaning up means shutting down
//    the given connection instance, stopping the timer for the send-timeout.
//
//    :param ch_chirp_t* chirp:      Pointer to a chirp instance.
//    :param ch_writer_t* writer:    Pointer to a writer instance.
//    :param ch_connection_t* conn:  Pointer to a connection instance.
//    :param int status:             Status of the write, of type
//                                   :c:type:`ch_error_t`.
//
//    :return:                       the status, which is of type
//                                   :c:type:`ch_error_t`.
//    :rtype:                        int
//

// .. c:function::
static ch_error_t
_ch_wr_connect(ch_remote_t* remote);
//
//    Connects to a remote peer.
//
//    :param ch_remote_t* remote: Remote to connect to.

// .. c:function::
static void
_ch_wr_connect_cb(uv_connect_t* req, int status);
//
//    Called by libuv after trying to connect. Contains the connection status.
//
//    :param uv_connect_t* req: Connect request, containing the connection.
//    :param int status:        Status of the connection.
//

// .. c:function::
static void
_ch_wr_connect_timeout_cb(uv_timer_t* handle);
//
//    Callback which is called after the connection reaches its timeout for
//    connecting. The timeout is set by the chirp configuration and is 5 seconds
//    by default. When this callback is called, the connection is shutdown.
//
//    :param uv_timer_t* handle: uv timer handle, data contains chirp

// .. c:function::
static void
_ch_wr_write_data_cb(uv_write_t* req, int status);
//
//    Callback which is called after data was written.
//
//    :param uv_write_t* req:  Write request.
//    :param int status:       Write status.

// .. c:function::
static void
_ch_wr_write_finish(
        ch_chirp_t* chirp, ch_writer_t* writer, ch_connection_t* conn);
//
//    Finishes the current write operation, the message store on the writer is
//    set to NULL and :c:func:`ch_chirp_finish_message` is called.
//
//    :param ch_chirp_t* chirp:      Pointer to a chirp instance.
//    :param ch_writer_t* writer:    Pointer to a writer instance.
//    :param ch_connection_t* conn:  Pointer to a connection instance.

// .. c:function::
static void
_ch_wr_write_timeout_cb(uv_timer_t* handle);
//
//    Callback which is called after the writer reaches its timeout for
//    sending. The timeout is set by the chirp configuration and is 5 seconds
//    by default. When this callback is called, the connection is being shut
//    down.
//
//    :param uv_timer_t* handle: uv timer handle, data contains chirp
//
// .. c:function::
static void
_ch_wr_enqeue_probe_if_needed(ch_remote_t* remote);
//
//    If remote wasn't use for 3/4 REUSE_TIME, we send a probe message before
//    the actual message. A probe message will probe the connection before
//    sending the actual message. If there is a garbage-collection race (the
//    remote is already closing the connection), the probe would fail and not
//    the actual message, then the closing of the connection is detected and it
//    will be reestablished on sending the actual message. Probes are a way of
//    preventing gc-races.
//
//    :param ch_remote_t* remote: Remote to send the probe to.

// Definitions
// ===========

// .. c:function::
static int
_ch_wr_check_write_error(
        ch_chirp_t*      chirp,
        ch_writer_t*     writer,
        ch_connection_t* conn,
        int              status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_check_write_error`
//
// .. code-block:: cpp
//
{
    A(writer->msg != NULL, "ⱳriter->msg should be set on callback");
    (void) (writer);
    if (status != CH_SUCCESS) {
        LC(chirp,
           "Write failed with uv status: %d. ",
           "ch_connection_t:%p",
           status,
           (void*) conn);
        ch_cn_shutdown(conn, CH_PROTOCOL_ERROR);
        return CH_PROTOCOL_ERROR;
    }
    return CH_SUCCESS;
}

// .. c:function::
static ch_error_t
_ch_wr_connect(ch_remote_t* remote)
//    :noindex:
//
//    see: :c:func:`_ch_wr_connect`
//
// .. code-block:: cpp
//
{
    ch_chirp_t*      chirp  = remote->chirp;
    ch_chirp_int_t*  ichirp = chirp->_;
    ch_connection_t* conn   = ch_alloc(sizeof(*conn));
    if (!conn) {
        return CH_ENOMEM;
    }
    remote->conn = conn;
    memset(conn, 0, sizeof(*conn));
    ch_cn_node_init(conn);
    conn->chirp        = chirp;
    conn->port         = remote->port;
    conn->ip_protocol  = remote->ip_protocol;
    conn->connect.data = conn;
    conn->remote       = remote;
    conn->client.data  = conn;
    int tmp_err        = uv_timer_init(ichirp->loop, &conn->connect_timeout);
    if (tmp_err != CH_SUCCESS) {
        EC(chirp,
           "Initializing connect timeout failed: %d. ",
           "ch_connection_t:%p",
           tmp_err,
           (void*) conn);
        ch_cn_shutdown(conn, CH_UV_ERROR);
        return CH_INIT_FAIL;
    }
    conn->connect_timeout.data = conn;
    conn->flags |= CH_CN_INIT_CONNECT_TIMEOUT;
    tmp_err = uv_timer_start(
            &conn->connect_timeout,
            _ch_wr_connect_timeout_cb,
            ch_min_uint64_t(ichirp->config.TIMEOUT * 2000, 60000),
            0);
    if (tmp_err != CH_SUCCESS) {
        EC(chirp,
           "Starting connect timeout failed: %d. ",
           "ch_connection_t:%p",
           tmp_err,
           (void*) conn);
        ch_cn_shutdown(conn, CH_UV_ERROR);
        return CH_UV_ERROR;
    }

    ch_text_address_t taddr;
    uv_inet_ntop(
            remote->ip_protocol,
            remote->address,
            taddr.data,
            sizeof(taddr.data));
#ifndef CH_WITHOUT_TLS
    if (!(ichirp->config.DISABLE_ENCRYPTION || ch_is_local_addr(&taddr))) {
        conn->flags |= CH_CN_ENCRYPTED;
    }
#endif
    memcpy(&conn->address, &remote->address, CH_IP_ADDR_SIZE);
    if (uv_tcp_init(ichirp->loop, &conn->client) < 0) {
        EC(chirp,
           "Could not initialize tcp. ",
           "ch_connection_t:%p",
           (void*) conn);
        ch_cn_shutdown(conn, CH_CANNOT_CONNECT);
        return CH_INIT_FAIL;
    }
    conn->flags |= CH_CN_INIT_CLIENT;
    struct sockaddr_storage addr;
    /* No error can happen, the address was taken from a binary format */
    ch_textaddr_to_sockaddr(remote->ip_protocol, &taddr, remote->port, &addr);
    tmp_err = uv_tcp_connect(
            &conn->connect,
            &conn->client,
            (struct sockaddr*) &addr,
            _ch_wr_connect_cb);
    if (tmp_err != CH_SUCCESS) {
        E(chirp,
          "Failed to connect to host: %s:%d (%d)",
          taddr.data,
          remote->port,
          tmp_err);
        ch_cn_shutdown(conn, CH_CANNOT_CONNECT);
        return CH_CANNOT_CONNECT;
    }
    LC(chirp,
       "Connecting to remote %s:%d. ",
       "ch_connection_t:%p",
       taddr.data,
       remote->port,
       (void*) conn);
    return CH_SUCCESS;
}

// .. c:function::
void
_ch_wr_connect_cb(uv_connect_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_connect_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn  = req->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    ch_text_address_t taddr;
    A(chirp == conn->chirp, "Chirp on connection should match");
    uv_inet_ntop(
            conn->ip_protocol, conn->address, taddr.data, sizeof(taddr.data));
    if (status == CH_SUCCESS) {
        LC(chirp,
           "Connected to remote %s:%d. ",
           "ch_connection_t:%p",
           taddr.data,
           conn->port,
           (void*) conn);
        /* A connection is created at either _ch_pr_new_connection_cb
         * (incoming) or _ch_wr_connect (outgoing). After connect
         * (_ch_wr_connection_cb) both code-paths will continue at
         * ch_pr_conn_start. From there on incoming and outgoing connections
         * are handled the same way. */
        ch_pr_conn_start(chirp, conn, &conn->client, 0);
    } else {
        EC(chirp,
           "Connection to remote failed %s:%d (%d). ",
           "ch_connection_t:%p",
           taddr.data,
           conn->port,
           status,
           (void*) conn);
        ch_cn_shutdown(conn, CH_CANNOT_CONNECT);
    }
}

// .. c:function::
static void
_ch_wr_connect_timeout_cb(uv_timer_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_wr_connect_timeout_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn  = handle->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    LC(chirp, "Connect timed out. ", "ch_connection_t:%p", (void*) conn);
    ch_cn_shutdown(conn, CH_TIMEOUT);
    uv_timer_stop(&conn->connect_timeout);
    /* We have waited long enough, we send the next message */
    ch_remote_t  key;
    ch_remote_t* remote = NULL;
    ch_rm_init_from_conn(chirp, &key, conn, 1);
    if (ch_rm_find(chirp->_->protocol.remotes, &key, &remote) == CH_SUCCESS) {
        ch_wr_process_queues(remote);
    }
}

// .. c:function::
static void
_ch_wr_enqeue_probe_if_needed(ch_remote_t* remote)
//    :noindex:
//
//    see: :c:func:`_ch_wr_enqeue_probe_if_needed`
//
// .. code-block:: cpp
//
{
    ch_chirp_t*     chirp  = remote->chirp;
    ch_chirp_int_t* ichirp = chirp->_;
    ch_config_t*    config = &ichirp->config;
    ch_message_t*   noop   = remote->noop;
    uint64_t        now    = uv_now(ichirp->loop);
    uint64_t        delta  = (1000 * config->REUSE_TIME / 4 * 3);
    if (now - remote->timestamp > delta) {
        if (noop == NULL) {
            remote->noop = ch_alloc(sizeof(*remote->noop));
            if (remote->noop == NULL) {
                return; /* ENOMEM: Noop are not important, we don't send it. */
            }
            noop = remote->noop;
            memset(noop, 0, sizeof(*noop));
            memcpy(noop->address, remote->address, CH_IP_ADDR_SIZE);
            noop->ip_protocol = remote->ip_protocol;
            noop->port        = remote->port;
            noop->type        = CH_MSG_NOOP;
        }
        /* The noop is not enqueued yet, enqueue it */
        if (!(noop->_flags & CH_MSG_USED) && noop->_next == NULL) {
            LC(chirp, "Sending NOOP.", "ch_remote_t:%p", remote);
            ch_msg_enqueue(&remote->cntl_msg_queue, noop);
        }
    }
}

// .. c:function::
static void
_ch_wr_write_data_cb(uv_write_t* req, int status)
//    :noindex:
//
//    see: :c:func:`_ch_wr_write_data_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn  = req->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    ch_writer_t* writer = &conn->writer;
    if (_ch_wr_check_write_error(chirp, writer, conn, status)) {
        return;
    }
    _ch_wr_write_finish(chirp, writer, conn);
}

// .. c:function::
static void
_ch_wr_write_finish(
        ch_chirp_t* chirp, ch_writer_t* writer, ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`_ch_wr_write_finish`
//
// .. code-block:: cpp
//
{
    ch_message_t* msg = writer->msg;
    A(msg != NULL, "Writer has no message");
    if (!(msg->type & CH_MSG_REQ_ACK)) {
        msg->_flags |= CH_MSG_ACK_RECEIVED; /* Emulate ACK */
    }
    msg->_flags |= CH_MSG_WRITE_DONE;
    writer->msg     = NULL;
    conn->timestamp = uv_now(chirp->_->loop);
    if (conn->remote != NULL) {
        conn->remote->timestamp = conn->timestamp;
    }
    ch_chirp_finish_message(chirp, conn, msg, CH_SUCCESS);
}

// .. c:function::
static void
_ch_wr_write_timeout_cb(uv_timer_t* handle)
//    :noindex:
//
//    see: :c:func:`_ch_wr_write_timeout_cb`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn  = handle->data;
    ch_chirp_t*      chirp = conn->chirp;
    ch_chirp_check_m(chirp);
    LC(chirp, "Write timed out. ", "ch_connection_t:%p", (void*) conn);
    ch_cn_shutdown(conn, CH_TIMEOUT);
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_send(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb)
//    :noindex:
//
//    see: :c:func:`ch_wr_send`
//
// .. code-block:: cpp
//
{
    ch_chirp_check_m(chirp);
    if (chirp->_->config.SYNCHRONOUS != 0) {
        msg->type = CH_MSG_REQ_ACK;
    } else {
        msg->type = 0;
    }
    return ch_wr_send(chirp, msg, send_cb);
}

// .. c:function::
CH_EXPORT
ch_error_t
ch_chirp_send_ts(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb)
//    :noindex:
//
//    see: :c:func:`ch_chirp_send_ts`
//
// .. code-block:: cpp
//
{
    A(chirp->_init == CH_CHIRP_MAGIC, "Not a ch_chirp_t*");
    ch_chirp_int_t* ichirp = chirp->_;
    if (msg->_flags & CH_MSG_USED || msg->_send_cb != NULL) {
        EC(chirp, "Message already used. ", "ch_message_t:%p", (void*) msg);
        return CH_USED;
    }
    msg->_send_cb = send_cb;
    uv_mutex_lock(&ichirp->send_ts_queue_lock);
    ch_msg_enqueue(&ichirp->send_ts_queue, msg);
    uv_mutex_unlock(&ichirp->send_ts_queue_lock);
    if (uv_async_send(&ichirp->send_ts) < 0) {
        E(chirp, "Could not call send_ts callback", CH_NO_ARG);
        return CH_UV_ERROR;
    }
    return CH_SUCCESS;
}

// .. c:function::
void
ch_wr_free(ch_writer_t* writer)
//    :noindex:
//
//    see: :c:func:`ch_wr_free`
//
// .. code-block:: cpp
//
{
    ch_connection_t* conn = writer->send_timeout.data;
    uv_timer_stop(&writer->send_timeout);
    uv_close((uv_handle_t*) &writer->send_timeout, ch_cn_close_cb);
    conn->shutdown_tasks += 1;
}

// .. c:function::
ch_error_t
ch_wr_init(ch_writer_t* writer, ch_connection_t* conn)
//    :noindex:
//
//    see: :c:func:`ch_wr_init`
//
// .. code-block:: cpp
//
{

    ch_chirp_t*     chirp  = conn->chirp;
    ch_chirp_int_t* ichirp = chirp->_;
    int             tmp_err;
    tmp_err = uv_timer_init(ichirp->loop, &writer->send_timeout);
    if (tmp_err != CH_SUCCESS) {
        EC(chirp,
           "Initializing send timeout failed: %d. ",
           "ch_connection_t:%p",
           tmp_err,
           (void*) conn);
        return CH_INIT_FAIL;
    }
    writer->send_timeout.data = conn;
    return CH_SUCCESS;
}

// .. c:function::
ch_error_t
ch_wr_process_queues(ch_remote_t* remote)
//    :noindex:
//
//    see: :c:func:`ch_wr_process_queues`
//
// .. code-block:: cpp
//
{
    AP(ch_at_allocated(remote), "Remote (%p) not allocated", (void*) remote);
    ch_chirp_t* chirp = remote->chirp;
    ch_chirp_check_m(chirp);
    ch_connection_t* conn = remote->conn;
    ch_message_t*    msg  = NULL;
    if (conn == NULL) {
        if (remote->flags & CH_RM_CONN_BLOCKED) {
            return CH_BUSY;
        } else {
            /* Only connect of the queue is not empty */
            if (remote->msg_queue != NULL || remote->cntl_msg_queue != NULL) {
                ch_error_t tmp_err;
                tmp_err = _ch_wr_connect(remote);
                if (tmp_err == CH_ENOMEM) {
                    ch_cn_abort_one_message(remote, CH_ENOMEM);
                }
                return tmp_err;
            }
        }
    } else {
        AP(ch_at_allocated(conn), "Conn (%p) not allocated", (void*) conn);
        if (!(conn->flags & CH_CN_CONNECTED) ||
            conn->flags & CH_CN_SHUTTING_DOWN) {
            return CH_BUSY;
        } else if (conn->writer.msg != NULL) {
            return CH_BUSY;
        } else if (remote->cntl_msg_queue != NULL) {
            ch_msg_dequeue(&remote->cntl_msg_queue, &msg);
            A(msg->type & CH_MSG_ACK || msg->type & CH_MSG_NOOP,
              "ACK/NOOP expected");
            ch_wr_write(conn, msg);
            return CH_SUCCESS;
        } else if (remote->msg_queue != NULL) {
            if (chirp->_->config.SYNCHRONOUS) {
                if (remote->wait_ack_message == NULL) {
                    ch_msg_dequeue(&remote->msg_queue, &msg);
                    remote->wait_ack_message = msg;
                    ch_wr_write(conn, msg);
                    return CH_SUCCESS;
                } else {
                    return CH_BUSY;
                }
            } else {
                ch_msg_dequeue(&remote->msg_queue, &msg);
                A(!(msg->type & CH_MSG_REQ_ACK), "REQ_ACK unexpected");
                ch_wr_write(conn, msg);
                return CH_SUCCESS;
            }
        }
    }
    return CH_EMPTY;
}

// .. c:function::
ch_error_t
ch_wr_send(ch_chirp_t* chirp, ch_message_t* msg, ch_send_cb_t send_cb)
//    :noindex:
//
//    see: :c:func:`ch_wr_send`
//
// .. code-block:: cpp
//
{
    ch_chirp_int_t* ichirp = chirp->_;
    if (ichirp->flags & CH_CHIRP_CLOSING || ichirp->flags & CH_CHIRP_CLOSED) {
        if (send_cb != NULL) {
            send_cb(chirp, msg, CH_SHUTDOWN);
        }
        return CH_SHUTDOWN;
    }
    ch_remote_t  search_remote;
    ch_remote_t* remote;
    msg->_send_cb = send_cb;
    A(!(msg->_flags & CH_MSG_USED), "Message should not be used");
    A(!((msg->_flags & CH_MSG_ACK_RECEIVED) ||
        (msg->_flags & CH_MSG_WRITE_DONE)),
      "No write state should be set");
    msg->_flags |= CH_MSG_USED;
    ch_protocol_t* protocol = &ichirp->protocol;

    ch_rm_init_from_msg(chirp, &search_remote, msg, 0);
    if (ch_rm_find(protocol->remotes, &search_remote, &remote) != CH_SUCCESS) {
        remote = ch_alloc(sizeof(*remote));
        LC(chirp, "Remote allocated", "ch_remote_t:%p", remote);
        if (remote == NULL) {
            if (send_cb != NULL) {
                send_cb(chirp, msg, CH_ENOMEM);
            }
            return CH_ENOMEM;
        }
        *remote     = search_remote;
        int tmp_err = ch_rm_insert(&protocol->remotes, remote);
        A(tmp_err == 0, "Inserting remote failed");
        (void) (tmp_err);
    }
    /* Remote isn't used for 3/4 REUSE_TIME we send a probe, before the
     * acutal message */
    _ch_wr_enqeue_probe_if_needed(remote);

    int queued = 0;
    if (msg->type & CH_MSG_ACK || msg->type & CH_MSG_NOOP) {
        queued = remote->cntl_msg_queue != NULL;
        ch_msg_enqueue(&remote->cntl_msg_queue, msg);
    } else {
        queued = remote->msg_queue != NULL;
        ch_msg_enqueue(&remote->msg_queue, msg);
    }

    ch_wr_process_queues(remote);
    if (queued)
        return CH_QUEUED;
    else
        return CH_SUCCESS;
}

// .. c:function::
void
ch_wr_send_ts_cb(uv_async_t* handle)
//    :noindex:
//
//    see: :c:func:`ch_wr_send_ts_cb`
//
// .. code-block:: cpp
//
{
    ch_chirp_t* chirp = handle->data;
    ch_chirp_check_m(chirp);
    ch_chirp_int_t* ichirp = chirp->_;
    if (ichirp->flags & CH_CHIRP_CLOSING) {
        return;
    }
    uv_mutex_lock(&ichirp->send_ts_queue_lock);

    ch_message_t* cur;
    ch_msg_dequeue(&ichirp->send_ts_queue, &cur);
    while (cur != NULL) {
        uv_mutex_unlock(&ichirp->send_ts_queue_lock);
        ch_chirp_send(chirp, cur, cur->_send_cb);
        uv_mutex_lock(&ichirp->send_ts_queue_lock);
        ch_msg_dequeue(&ichirp->send_ts_queue, &cur);
    }
    uv_mutex_unlock(&ichirp->send_ts_queue_lock);
}

// .. c:function::
void
ch_wr_write(ch_connection_t* conn, ch_message_t* msg)
//    :noindex:
//
//    see: :c:func:`ch_wr_write`
//
// .. code-block:: cpp
//
{
    ch_chirp_t*     chirp  = conn->chirp;
    ch_writer_t*    writer = &conn->writer;
    ch_remote_t*    remote = conn->remote;
    ch_chirp_int_t* ichirp = chirp->_;
    A(writer->msg == NULL, "Message should be null on new write");
    writer->msg = msg;
    int tmp_err = uv_timer_start(
            &writer->send_timeout,
            _ch_wr_write_timeout_cb,
            ichirp->config.TIMEOUT * 1000,
            0);
    if (tmp_err != CH_SUCCESS) {
        EC(chirp,
           "Starting send timeout failed: %d. ",
           "ch_connection_t:%p",
           tmp_err,
           (void*) conn);
    }

    remote->serial += 1;
    ch_sr_msg_to_buf(msg, writer->net_msg, remote->serial);
    uv_buf_t buf[3];
    buf[0].base = writer->net_msg;
    buf[0].len  = CH_SR_WIRE_MESSAGE_SIZE;
    buf[1].base = msg->header;
    buf[1].len  = msg->header_len;
    buf[2].base = msg->data;
    buf[2].len  = msg->data_len;
    ch_cn_write(conn, buf, 3, _ch_wr_write_data_cb);
}
