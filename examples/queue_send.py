from libchirp.queue import Chirp, Config, Loop, Message

loop = Loop(); config = Config(); message = Message()
config.DISABLE_ENCRYPTION = True
# The queue interface is a bit tricky with ACKNOWLEDGE
config.ACKNOWLEDGE = False
config.PORT = 2992
message.data = b'hello'
message.address = "127.0.0.1"
message.port = 2998
try:
    chirp = Chirp(loop, config)
    chirp.send(message).result()
    msg = chirp.get(timeout=1)
    print(msg.data)
    msg.release().result()
finally:
    chirp.stop()
    loop.stop()
