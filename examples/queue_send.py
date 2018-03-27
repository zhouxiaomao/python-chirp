#!/usr/bin/env python3
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
