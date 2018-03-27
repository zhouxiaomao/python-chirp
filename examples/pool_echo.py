#!/usr/bin/env python3
from concurrent.futures import Future
from libchirp.pool import Chirp, Config, Loop

res = Future()

class MyChirp(Chirp):
    def handler(self, msg):
        print(msg.data)
        res.set_result(self.send(msg).result())

loop = Loop(); config = Config()
config.DISABLE_ENCRYPTION = True
# Workers are usually asynchronous
config.SYNCHRONOUS = False
try:
    chirp = MyChirp(loop, config)
    try:
        res.result()
    finally:
        chirp.stop()
finally:
    loop.stop()
