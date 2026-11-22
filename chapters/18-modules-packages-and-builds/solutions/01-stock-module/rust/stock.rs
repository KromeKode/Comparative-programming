//! The stock module: how many of each item there are.

use std::collections::BTreeMap;

/// Counts of items. The struct is `pub`, so main.rs can name it, but its field
/// isn't, so main.rs can't read or change the counts except through the methods.
#[derive(Default)]
pub struct Stock {
    counts: BTreeMap<String, u64>,
}

impl Stock {
    pub fn new() -> Stock {
        Stock::default()
    }

    pub fn add(&mut self, item: &str, amount: u64) {
        *self.counts.entry(item.to_string()).or_insert(0) += amount;
    }

    /// Takes amount of item away, and returns true, if there are that many.
    pub fn take(&mut self, item: &str, amount: u64) -> bool {
        match self.counts.get_mut(item) {
            Some(count) if *count >= amount => {
                *count -= amount;
                true
            }
            _ => false,
        }
    }

    /// How many of item there are: 0 for an item never added.
    pub fn count(&self, item: &str) -> u64 {
        self.counts.get(item).copied().unwrap_or(0)
    }

    /// The items there are any of, with their counts, in alphabetical order, which
    /// is the order a BTreeMap keeps its keys in.
    pub fn items(&self) -> Vec<(&str, u64)> {
        self.counts
            .iter()
            .filter(|(_, count)| **count > 0)
            .map(|(item, count)| (item.as_str(), *count))
            .collect()
    }
}
