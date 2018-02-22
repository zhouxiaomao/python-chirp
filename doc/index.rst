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

.. _modes-of-operation:

Modes of operation
==================

Connection-synchronous
----------------------

:py:attr:`libchirp.Config.ACKNOWLEDGE` = `True`

* Sending a message only returns a success when the remote has released the message.

* No message can be lost by chirp

* For concurrency the application has to disable :py:attr:`libchirp.Config.AUTO_RELEASE`.
  Then the application has to take care that the message is not lost.

* If the application completes the operation inside the message-handler,
  messages will automatically be throttled. Be aware of the timeout: if the
  applications operation takes longer either increase the timeout or copy the
  message (with copying you lose the throttling)

* Slower

Connection-asynchronous
-----------------------

:py:attr:`libchirp.Config.ACKNOWLEDGE` = `False`

* Sending a message returns a success when the message is successfully written to
  the operating system

* If unexpected errors (ie. remote dies) happen, the message can be lost in the
  TCP-buffer

* Automatic concurrency, by default chirp uses 16 concurrent message-slots

* The application needs a scheduler that periodically checks that operations
  have completed

* Faster

What should I use?
------------------

For simple message transmission, for example sending events to a time-series
database we recommend :py:attr:`libchirp.Config.ACKNOWLEDGE` = `True`, since
chirp will cover this process out of the box.

For more complex application where you have to schedule your operations anyway,
use :py:attr:`libchirp.Config.ACKNOWLEDGE` = `False`, do periodic bookkeeping
and resend failed operations.

API Reference
=============

.. toctree::
   :maxdepth: 2

   api.rst

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
