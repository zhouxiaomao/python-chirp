#!/usr/bin/env python3
import asyncio
from libchirp.asyncio import Chirp, Config, Loop, Message

async def send():
    await chirp.send(message)

class MyChirp(Chirp):
    async def handler(self, msg):
        print(msg.data)

loop = Loop(); config = Config(); message = Message()
config.DISABLE_ENCRYPTION = True
config.PORT = 2992
message.data = b'hello'
message.address = "127.0.0.1"
message.port = 2998
aio_loop = asyncio.get_event_loop()
try:
    try:
        chirp = MyChirp(loop, config, aio_loop)
        fut = asyncio.ensure_future(send())
        aio_loop.run_until_complete(fut)
    finally:
        chirp.stop()
finally:
    loop.stop()
