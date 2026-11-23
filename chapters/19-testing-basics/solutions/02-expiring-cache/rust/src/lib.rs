use std::collections::HashMap;
use std::time::{SystemTime, UNIX_EPOCH};

/// Something that tells the time, in whole seconds.
pub trait Clock {
    fn now(&self) -> u64;
}

/// The computer's clock, which counts the seconds since 1970.
pub struct SystemClock;

impl Clock for SystemClock {
    fn now(&self) -> u64 {
        SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map_or(0, |since_1970| since_1970.as_secs())
    }
}

struct Entry {
    value: String,
    expires: u64, // the time the entry stops being live
}

impl Entry {
    fn is_live(&self, now: u64) -> bool {
        now < self.expires
    }
}

/// Keeps each value for `ttl` seconds after it was put, and then forgets it.
pub struct Cache<'a> {
    ttl: u64,
    clock: &'a dyn Clock,
    entries: HashMap<String, Entry>,
}

impl<'a> Cache<'a> {
    pub fn new(ttl: u64, clock: &'a dyn Clock) -> Self {
        Cache {
            ttl,
            clock,
            entries: HashMap::new(),
        }
    }

    /// Stores `value` under `key` for the next `ttl` seconds, replacing any value there.
    pub fn put(&mut self, key: &str, value: &str) {
        let now = self.clock.now();
        self.entries.retain(|_, entry| entry.is_live(now));
        let entry = Entry {
            value: value.to_string(),
            expires: now + self.ttl,
        };
        self.entries.insert(key.to_string(), entry);
    }

    /// The value under `key`, or `None` if there is none or it has expired.
    pub fn get(&self, key: &str) -> Option<&str> {
        let entry = self.entries.get(key)?;
        if !entry.is_live(self.clock.now()) {
            return None;
        }
        Some(&entry.value)
    }

    /// How many entries haven't expired.
    pub fn len(&self) -> usize {
        let now = self.clock.now();
        self.entries
            .values()
            .filter(|entry| entry.is_live(now))
            .count()
    }

    /// Whether every entry has expired.
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

#[cfg(test)]
mod tests;
