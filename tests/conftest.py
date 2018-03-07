"""Configure pytest."""
from libchirp import Config, Message, Loop
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
    return Message()


@pytest.fixture
def loop():
    """Return a libchirp loop."""
    loop = Loop()
    loop.run()
    yield loop
    loop.stop()


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
