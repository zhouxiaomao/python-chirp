"""Configure pytest."""
import pytest
from libchirp import Config, Message


@pytest.fixture
def config():
    """Return a libchirp config."""
    return Config()


@pytest.fixture
def message():
    """Return a libchirp message."""
    return Message()
