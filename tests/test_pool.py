"""Queue tests."""

from queue import Queue
import time
import pytest

from libchirp.pool import Chirp, Config


def test_initialize(loop, config):
    """test_initialize."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = Chirp(loop, config)
    a.stop()


def test_recv_msg(config, sender, message):
    """test_recv_msg."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"

    class MyChirp(Chirp):
        def handler(self, msg):
            self.queue.put(msg)

    a = MyChirp(sender.loop, config)
    a.queue = Queue()
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    sender.send(message)
    msg = a.queue.get()
    time.sleep(0.1)
    assert msg._msg_t is None
    a.stop()


def test_pool_reqquest_timeout(config, sender, message):
    """test_pool_request_timeout."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"

    a = Chirp(sender.loop, config)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    with pytest.raises(TimeoutError):
        sender.request(message).result()
    a.stop()


def test_pool_request(config, sender, message):
    """test_pool_request."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"

    class MyChirp(Chirp):
        def handler(self, msg):
            self.send(msg).result()

    a = MyChirp(sender.loop, config)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    msg = sender.request(message).result()
    assert msg.data == b'hello'
    a.stop()


def test_ignore_msg(config, sender, message):
    """test_ignore_msg."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.AUTO_RELEASE = False

    class MyChirp(Chirp):
        pass

    a = MyChirp(sender.loop, config)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    sender.send(message).result()
    a.stop()


def test_recv_msg_no_auto(config, sender, message):
    """test_recv_msg_no_auto."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.AUTO_RELEASE = False

    class MyChirp(Chirp):
        def handler(self, msg):
            self.queue.put(msg)

    a = MyChirp(sender.loop, config)
    a.queue = Queue()
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    sender.send(message)
    msg = a.queue.get()
    time.sleep(0.1)
    assert msg._msg_t is not None
    msg.release().result()
    assert msg._msg_t is None
    a.stop()
