#!/usr/bin/env python3
import asyncio
from libchirp.asyncio import Chirp, Config, Loop

class MyChirp(Chirp):
    async def handler(self, msg):
        print(msg.data)
        await self.send(msg)
        aio_loop.stop()

loop = Loop(); config = Config()
config.DISABLE_ENCRYPTION = True
# Workers are usually asynchronous
config.SYNCHRONOUS = False
aio_loop = asyncio.get_event_loop()
try:
    chirp = MyChirp(loop, config, aio_loop)
    try:
        aio_loop.run_forever()
    finally:
        chirp.stop()
finally:
    loop.stop()
