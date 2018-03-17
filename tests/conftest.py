"""Configure pytest."""
from libchirp import ChirpBase, Config, Loop, MessageThread
from libchirp.queue import Chirp
import os
import pytest
import signal
from subprocess import Popen, PIPE, TimeoutExpired
import time


@pytest.fixture
def config():
    """Return a libchirp config."""
    return Config()


@pytest.fixture
def message():
    """Return a libchirp message."""
    return MessageThread()


@pytest.fixture
def loop():
    """Return a libchirp loop."""
    loop = Loop()
    yield loop
    loop.stop()


@pytest.fixture
def sender(loop, config):
    """Return a libchirp sender."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.PORT = 2992
    a = ChirpBase(loop, config)
    yield a
    a.stop()


@pytest.fixture
def queue(loop, config):
    """Return a libchirp.queue sender."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.PORT = 2992
    a = Chirp(loop, config)
    yield a
    a.stop()


@pytest.fixture
def fast_sender(loop, config):
    """Return a libchirp sender."""
    config.DISABLE_ENCRYPTION = True
    config.SYNCHRONOUS = False
    config.PORT = 2992
    a = ChirpBase(loop, config)
    yield a
    a.stop()


@pytest.fixture
def echo():
    """Run a echo_test."""
    args = [
        "./echo_test", "2993", "0"
    ]
    echo = Popen(args, stdin=PIPE, preexec_fn=os.setsid)
    time.sleep(0.6)
    yield echo
    close(echo)


def close(proc):
    """Close the subprocess."""
    try:
        proc.stdin.write(b"\n")
        proc.stdin.flush()
        proc.wait(4)
    except TimeoutExpired:
        print("Doing kill")
        os.killpg(os.getpgid(proc.pid), signal.SIGTERM)
        proc.poll()
        raise  # Its a bug when the process doesn't complete
