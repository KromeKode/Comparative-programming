# Write tests for the cache here, as test_ functions. One is here to start with, on
# the computer's clock, which can't show what happens as time passes without the
# test waiting for it. The README lists what the cache promises.
from cache import Cache, SystemClock


def test_put_then_get() -> None:
    cache = Cache(60, SystemClock())
    cache.put("a", "1")
    assert cache.get("a") == "1"
