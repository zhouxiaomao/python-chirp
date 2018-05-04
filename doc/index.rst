============
python-chirp
============

.. image:: https://raw.githubusercontent.com/concretecloud/chirp/master/doc/_static/chirp.png
   :width: 30%

Supported by |adsy|

.. |adsy| image:: https://1042.ch/ganwell/adsy-logo.svg
   :target: https://adfinis-sygroup.ch/

Message-passing for everyone
============================

Features
========

* Fully automatic connection setup

* TLS support

  * Connections to 127.0.0.1 and ::1 aren't encrypted

* Easy message routing

* Robust

  * No message can be lost without an error (in sync mode)

* Very thin API

* Minimal code-base, all additional features will be implemented as modules in
  an upper layer

* Fast

API Reference
=============

.. toctree::
   :maxdepth: 2

   std_api.rst


.. _modes-of-operation:

Modes of operation
==================

The internal operation is always asynchronous. libchirp is asynchronous across
multiple connections. Often you want one peer to be synchronous and the other
asynchronous, depending on what pattern you implement.

Connection-synchronous
----------------------

:py:attr:`libchirp.Config.SYNCHRONOUS` = `True`

* Sending a message only returns a success (`result()` or `await`) when the remote
  has released the message.

* No message can be lost by chirp

* If the application completes the operation inside the message-handler,
  messages will automatically be throttled. Be aware of the timeout: if the
  applications operation takes longer either increase the timeout or copy the
  message (with copying you lose the throttling)

* Slower

Connection-asynchronous
-----------------------

:py:attr:`libchirp.Config.SYNCHRONOUS` = `False`

* Sending a message returns a success (`result()` or `await`) when the message
  is successfully written to the operating system

* If unexpected errors (ie. remote dies) happen, the message can be lost in the
  TCP-buffer

* Automatic concurrency, by default chirp uses 16 concurrent message-slots

* The application needs a scheduler that periodically checks that operations
  have completed

* Faster

What should I use?
------------------

Rule of thumb:

* Consumers (workers) are not synchronous (SYNCHRONOUS = False)

* Producers are synchronous if they don't do bookkeeping (SYNCHRONOUS = True)

* If you route messages from a synchronous producer, you want to be synchronous
  too: Timeouts get propagated to the producer.

For simple message transmission, for example sending events to a time-series
database we recommend :py:attr:`libchirp.Config.SYNCHRONOUS` = `True`, since
chirp will cover this process out of the box.

For more complex application where you have to schedule your operations anyway,
use :py:attr:`libchirp.Config.SYNCHRONOUS` = `False`, do periodic bookkeeping
and resend failed operations.

Message-slots
=============

libchirp supports up to 32 message-slots configured by
:py:attr:`libchirp.Config.MAX_SLOTS`. By default 16 (connection-asynchronous) or
1 (synchronous) message-slots are used. Message-slots ensure that a chirp-node
is not overloaded. If there where no bounds the system could go out-of-memory or
not be able to do the computations requested by the messages. Because of this
message-slots have to be released after the work is done. In order to be more
pythonic, libchirp has the :py:attr:`libchirp.Config.AUTO_RELEASE` feature,
which will automatically release the message-slot after the handler (your code)
returns. If the work is not done after the message-slot returns we recommend to
release the message-slot manually.

In connection-synchronous mode the throttling will also propagate through
message-routers if routed message is released after

.. code-block:: python

   send().result()

returns.

In :py:class:`libchirp.queue.Chirp` the message will be release when you call
:py:meth:`libchirp.queue.Chirp.get`, which is NOT after the work is done. Which
is fine if you process one message at time. If you process messages concurrent,
we recommend to disable auto-release.

.. _concurrency:

Concurrency
===========

For the

* :py:class:`libchirp.asyncio.Chirp`

* :py:class:`libchirp.queue.Chirp`

* :py:class:`libchirp.pool.Chirp`

implementations the libuv evnet-loop runs in a separate thread.
send()/release_slot() return Futures, which will finish once libchirp.c has
finished the operation.  Concurrency can be achieved by issuing multiple send()
commands and waiting for all the resulting futures later. In a low-latency
environment up to 64 for futures can be kept open at time for optimal
performance. Beyond 64, managing the open requests in chirp and maybe also the
operating-system seems to kick in.  If you need to improve the performance
please experiment in your particular environment.

See also

* :py:func:`concurrent.futures.wait`

* :py:func:`concurrent.futures.as_completed`

* :py:func:`asyncio.gather`

Performance
===========

Glossary
--------

* Process pass: Execute something in another process and get the result back
* Thread pass: Execute something in another thread and get the result back
* Pooled: 16 operations can be active at the same time
* Single: 1 operation can be active at the same time

Python measurements
-------------------

* 20'000 thread passes in plain python (single)
* 3'500 process passes in plain python (single)
* 42'000 thread passes in plain python (pooled)
* 3'500 process passes in plain python (pooled)
  * Actually 3'000 but you wouldn't pool it in that case

libchirp C measurements
-----------------------

* 55'000 process passes in C (single)
* 240'000 process passes in C (pooled)
* Thread passes are not relevant since we always use sockets (syscalls)

libchirp python measurements
----------------------------

* 18'000 process passes with CFFI bindings (single)
  * Compared to the 3'500 process passes of pure python this is 5 times faster
* 23'000 process passes with CFFI bindings (pooled)
  * Compared to the 3'500 process passes of pure python this is 6.5 times faster

The limiting factor are the thread passes in pure python, since the libuv
event-loop runs in a separate thread. https://github.com/MagicStack/uvloop could
eliminate that extra thread, but that is currently beyond the scope of our
project.

This also means: Using multiple chirp instances in a mesh-topology will increase
performance.

Notebook: Intel(R) Core(TM) i7-6500U CPU @ 2.50GHz
Kernel: 4.14.18-0-vanilla

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
