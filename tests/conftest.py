"""Configure pytest."""
import pytest
from libchirp import Config, Message, Loop


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
