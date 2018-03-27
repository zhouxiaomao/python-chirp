#!/usr/bin/env python3
from concurrent.futures import Future
from libchirp.pool import Chirp, Config, Loop, Message

res = Future()

class MyChirp(Chirp):
    def handler(self, msg):
        print(msg.data)
        res.set_result(0)

loop = Loop(); config = Config(); message = Message()
config.DISABLE_ENCRYPTION = True
config.PORT = 2992
message.data = b'hello'
message.address = "127.0.0.1"
message.port = 2998
try:
    try:
        chirp = MyChirp(loop, config)
        chirp.send(message).result()
        res.result()
    finally:
        chirp.stop()
finally:
    loop.stop()
