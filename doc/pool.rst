====
Pool
====

Please import all classes from namespace :py:mod:`libchirp.pool`, to make sure
you get the right implementation.

.. code-block:: python

   from libchirp.pool import Chirp, Config, Loop, Message

.. autosummary::

   libchirp.pool.Chirp
   libchirp.pool.Message

Example
=======

pool_echo.py

.. code-block:: python

   from concurrent.futures import Future
   from libchirp.pool import Chirp, Config, Loop

   res = Future()

   class MyChirp(Chirp):
       def handler(self, msg):
           print(msg.data)
           res.set_result(self.send(msg).result())

   loop = Loop(); config = Config()
   config.DISABLE_ENCRYPTION = True
   # Workers usually do not acknowledge
   config.ACKNOWLEDGE = False
   try:
       chirp = MyChirp(loop, config)
       try:
           res.result()
       finally:
           chirp.stop()
   finally:
       loop.stop()

For a sender see :ref:`sender`.

Chirp
=====

.. autoclass:: libchirp.pool.Chirp
    :members:
    :inherited-members:
    :undoc-members:
    :show-inheritance:

Message
=======

.. autoclass:: libchirp.pool.Message
    :members:
    :inherited-members:
    :undoc-members:
    :show-inheritance:

Config/Loop
===========

* :py:class:`libchirp.Config`

* :py:class:`libchirp.Loop`
