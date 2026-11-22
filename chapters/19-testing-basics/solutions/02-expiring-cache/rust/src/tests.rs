use std::cell::Cell;

use super::*;

/// A clock that says whatever time the test sets. The cache borrows it, so the time is
/// in a Cell, which can be set through a shared reference.
#[derive(Default)]
struct FakeClock {
    time: Cell<u64>,
}

impl Clock for FakeClock {
    fn now(&self) -> u64 {
        self.time.get()
    }
}

#[test]
fn put_then_get() {
    let clock = FakeClock::default();
    let mut cache = Cache::new(10, &clock);
    cache.put("a", "1");
    assert_eq!(cache.get("a"), Some("1"));
    assert_eq!(cache.get("b"), None);
    assert_eq!(cache.len(), 1);
}

// An entry put at time t is there until t + ttl, and gone from then on.
#[test]
fn expires_after_ttl() {
    let clock = FakeClock::default();
    clock.time.set(100);
    let mut cache = Cache::new(10, &clock);
    cache.put("a", "1");
    clock.time.set(109);
    assert_eq!(cache.get("a"), Some("1"));
    clock.time.set(110);
    assert_eq!(cache.get("a"), None);
    assert_eq!(cache.len(), 0);
    assert!(cache.is_empty());
}

// Getting an entry doesn't make it last longer.
#[test]
fn get_does_not_refresh() {
    let clock = FakeClock::default();
    let mut cache = Cache::new(10, &clock);
    cache.put("a", "1");
    clock.time.set(5);
    assert_eq!(cache.get("a"), Some("1"));
    clock.time.set(10);
    assert_eq!(cache.get("a"), None);
}

// Putting a key again replaces its value, and its ttl starts again.
#[test]
fn put_replaces() {
    let clock = FakeClock::default();
    let mut cache = Cache::new(10, &clock);
    cache.put("a", "1");
    clock.time.set(5);
    cache.put("a", "2");
    assert_eq!(cache.len(), 1);
    clock.time.set(14);
    assert_eq!(cache.get("a"), Some("2"));
    clock.time.set(15);
    assert_eq!(cache.get("a"), None);
}

// Each key has its own time: one expiring, or a put of another, leaves the rest.
#[test]
fn keys_are_separate() {
    let clock = FakeClock::default();
    let mut cache = Cache::new(10, &clock);
    cache.put("a", "1");
    clock.time.set(5);
    cache.put("b", "2");
    assert_eq!(cache.get("a"), Some("1"));
    assert_eq!(cache.get("b"), Some("2"));
    assert_eq!(cache.len(), 2);
    clock.time.set(10);
    assert_eq!(cache.get("a"), None);
    assert_eq!(cache.get("b"), Some("2"));
    assert_eq!(cache.len(), 1);
}
