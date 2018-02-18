"""Configure pytest."""
import pytest
from libchirp import Config


@pytest.fixture
def config():
    """Return a libchirp config."""
    return Config()
