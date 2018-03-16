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
   config.PORT = 2992
   message.data = b'hello'
   message.address = "127.0.0.1"
   message.port = 2998
   try:
       chirp = Chirp(loop, config)
       chirp.send(message).result()
       msg = chirp.get()
       msg.release().result()
       print(msg.data)
   finally:
       chirp.stop()
       loop.stop()

For a echo-server see :ref:`echo-server`.

The usual pattern for the queue interface is:

.. code-block:: python

   config.ACKNOWLEDGE = False
   while True:
      msg = chirp.get()
      # Do heavy work
      chirp.send(msg)  # Send the result
      msg.release()  # Notify the remote that the work is done

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
