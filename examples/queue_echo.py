#!/usr/bin/env python3
from libchirp.queue import Chirp, Config, Loop, Message

loop = Loop(); config = Config()
config.DISABLE_ENCRYPTION = True
# Workers are usually asynchronous
config.SYNCHRONOUS = False
try:
    chirp = Chirp(loop, config)
    msg = chirp.get()
    print(msg.data)
    chirp.send(msg).result()
    msg.release().result()
finally:
    chirp.stop()
    loop.stop()
