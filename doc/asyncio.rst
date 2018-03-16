=======
Asyncio
=======

Please import all classes from namespace :py:mod:`libchirp.asyncio`, to make
sure you get the right implementation.

.. code-block:: python

   from libchirp.asyncio import Chirp, Config, Loop, Message

.. autosummary::

   libchirp.asyncio.Chirp
   libchirp.asyncio.Message

.. _echo-server:

Example
=======

async_echo.py

.. code-block:: python

   import asyncio
   from libchirp.asyncio import Chirp, Config, Loop

   class MyChirp(Chirp):
       async def handler(self, msg):
           print(msg.data)
           await self.send(msg)
           aio_loop.stop()

   loop = Loop(); config = Config()
   config.DISABLE_ENCRYPTION = True
   # Workers usually do not acknowledge
   config.ACKNOWLEDGE = False
   aio_loop = asyncio.get_event_loop()
   try:
       chirp = MyChirp(loop, config, aio_loop)
       try:
           aio_loop.run_forever()
       finally:
           chirp.stop()
   finally:
       loop.stop()

For a sender see :ref:`sender`.

Chirp
=====

.. autoclass:: libchirp.asyncio.Chirp
    :members:
    :inherited-members:
    :undoc-members:
    :show-inheritance:

Message
=======

.. autoclass:: libchirp.asyncio.Message
    :members:
    :inherited-members:
    :undoc-members:
    :show-inheritance:

Config/Loop
===========

* :py:class:`libchirp.Config`

* :py:class:`libchirp.Loop`
