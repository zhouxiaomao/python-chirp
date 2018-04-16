#!/usr/bin/env python3
from concurrent.futures import Future
from libchirp.pool import Chirp, Config, Loop, Message

loop = Loop(); config = Config(); message = Message()
config.DISABLE_ENCRYPTION = True
config.PORT = 2992
message.data = b'hello'
message.address = "127.0.0.1"
message.port = 2998
try:
    try:
        chirp = Chirp(loop, config)
        res = chirp.request(message).result()
        print(res.data)
    finally:
        chirp.stop()
finally:
    loop.stop()
