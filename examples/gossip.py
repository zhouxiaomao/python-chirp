import asyncio
import uuid
import ipaddress
import json
from libchirp.asyncio import Chirp, Config, Loop, Message
import random
from subprocess import check_output
import sys
import time

update_delay = 60

peers = set()
infos = dict()
myip = None
myid = uuid.getnode()

def cmd(*args):
    return check_output(args).strip().decode("UTF-8")

def get_info():
    return [
        myid,
        time.time(),
        myip,
        cmd("uname", "-r"),
        cmd("uptime")
    ]

def print_info():
    print("\033[2J\033[1;1H")
    for info in sorted(infos.values()):
        print(", ".join([str(x) for x in info[2:]]))

class MyChirp(Chirp):
    async def handler(self, msg):
        global myip
        try:
            peers.add(msg.address)
            info = json.loads(msg.data, encoding="UTF-8")
            if info[0] == myid:
                myip = info[2]
            if info[2] is None:
                info[2] = msg.address
            old_info = infos.get(info[0])
            if old_info is None or info[1] > old_info[1]:
                infos[info[0]] = info
                print_info()
        except Exception as e:
            print(e)

    async def update(self):
        while True:
            try:
                info = get_info()
                infos[info[0]] = info
                for peer in peers:
                    info = random.choice(list(infos.values()))
                    msg = Message()
                    msg.port = config.PORT
                    msg.address = peer
                    msg.data = json.dumps(info).encode("UTF-8")
                    try:
                        await self.send(msg)
                    except Exception:
                        peers.remove(peer)
                await asyncio.sleep(update_delay)
            except Exception as e:
                print(e)

for arg in sys.argv[1:]:
    peers.add(ipaddress.ip_address(arg).compressed)

loop = Loop(); config = Config()
config.DISABLE_ENCRYPTION = True
config.SYNCHRONOUS = False
aio_loop = asyncio.get_event_loop()
try:
    chirp = MyChirp(loop, config, aio_loop)
    try:
        asyncio.run_coroutine_threadsafe(chirp.update(), aio_loop)
        aio_loop.run_forever()
    finally:
        chirp.stop()
finally:
    loop.stop()
