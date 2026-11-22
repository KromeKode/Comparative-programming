// A child module of lib.rs, so `super::*` brings in everything it defines,
// private items included.
use super::*;

#[test]
fn divisible_by_4() {
    assert!(is_leap_year(2024));
    assert!(!is_leap_year(2023));
}

#[test]
fn centuries() {
    assert!(!is_leap_year(1900));
    assert!(is_leap_year(2000));
}
