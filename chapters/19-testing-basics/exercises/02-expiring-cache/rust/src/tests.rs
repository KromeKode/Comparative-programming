// Write tests for the cache here, as #[test] functions. One is here to start with,
// on the computer's clock, which can't show what happens as time passes without the
// test waiting for it. The README lists what the cache promises.
use super::*;

#[test]
fn put_then_get() {
    let mut cache = Cache::new(60, &SystemClock);
    cache.put("a", "1");
    assert_eq!(cache.get("a"), Some("1"));
}
