=====
Queue
=====

Please import all classes from namespace :py:mod:`libchirp.queue`, to make sure
you get the right implementation.

.. code-block:: python

   from libchirp.queue import Chirp, Config, Loop, Message

.. autosummary::

   libchirp.queue.Chirp
   libchirp.queue.Message

.. _sender:

Example
=======

queue_sender.py

.. code-block:: python

   from libchirp.queue import Chirp, Config, Loop, Message

   loop = Loop(); config = Config(); message = Message()
   config.DISABLE_ENCRYPTION = True
   # The queue interface is a bit tricky with ACKNOWLEDGE
   config.ACKNOWLEDGE = False
   config.PORT = 2992
   message.data = b'hello'
   message.address = "127.0.0.1"
   message.port = 2998
   try:
       chirp = Chirp(loop, config)
       chirp.send(message).result()
       msg = chirp.get(timeout=1)
       print(msg.data)
       msg.release().result()
   finally:
       chirp.stop()
       loop.stop()

For a echo-server see :ref:`echo-server`.

Chirp
=====

.. autoclass:: libchirp.queue.Chirp
    :members:
    :inherited-members:
    :undoc-members:
    :show-inheritance:

Message
=======

.. autoclass:: libchirp.queue.Message
    :members:
    :inherited-members:
    :undoc-members:
    :show-inheritance:

Config/Loop
===========

* :py:class:`libchirp.Config`

* :py:class:`libchirp.Loop`
