import pytest

from cache import Cache


class FakeClock:
    """A clock that says whatever time the test sets."""

    def __init__(self) -> None:
        self.time = 0

    def now(self) -> int:
        return self.time


# A fixture: pytest calls it for each test that has a parameter named clock, and
# passes what it returns, so every test gets a new clock.
@pytest.fixture
def clock() -> FakeClock:
    return FakeClock()


def test_put_then_get(clock: FakeClock) -> None:
    cache = Cache(10, clock)
    cache.put("a", "1")
    assert cache.get("a") == "1"
    assert cache.get("b") is None
    assert len(cache) == 1


def test_expires_after_ttl(clock: FakeClock) -> None:
    """An entry put at time t is there until t + ttl, and gone from then on."""
    clock.time = 100
    cache = Cache(10, clock)
    cache.put("a", "1")
    clock.time = 109
    assert cache.get("a") == "1"
    clock.time = 110
    assert cache.get("a") is None
    assert len(cache) == 0


def test_get_does_not_refresh(clock: FakeClock) -> None:
    """Getting an entry doesn't make it last longer."""
    cache = Cache(10, clock)
    cache.put("a", "1")
    clock.time = 5
    assert cache.get("a") == "1"
    clock.time = 10
    assert cache.get("a") is None


def test_put_replaces(clock: FakeClock) -> None:
    """Putting a key again replaces its value, and its ttl starts again."""
    cache = Cache(10, clock)
    cache.put("a", "1")
    clock.time = 5
    cache.put("a", "2")
    assert len(cache) == 1
    clock.time = 14
    assert cache.get("a") == "2"
    clock.time = 15
    assert cache.get("a") is None


def test_keys_are_separate(clock: FakeClock) -> None:
    """Each key has its own time: one expiring, or a put of another, leaves the rest."""
    cache = Cache(10, clock)
    cache.put("a", "1")
    clock.time = 5
    cache.put("b", "2")
    assert cache.get("a") == "1"
    assert cache.get("b") == "2"
    assert len(cache) == 2
    clock.time = 10
    assert cache.get("a") is None
    assert cache.get("b") == "2"
    assert len(cache) == 1
