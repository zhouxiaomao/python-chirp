from libchirp.queue import Chirp, Config, Loop, Message

loop = Loop(); config = Config()
config.DISABLE_ENCRYPTION = True
# Workers usually do not acknowledge
config.ACKNOWLEDGE = False
try:
    chirp = Chirp(loop, config)
    msg = chirp.get()
    print(msg.data)
    chirp.send(msg).result()
    msg.release().result()
finally:
    chirp.stop()
    loop.stop()
