// Write tests for wrap here, as #[test] functions. One is here to start with, and
// the README lists what wrap promises.
use super::*;

#[test]
fn one_line() {
    assert_eq!(wrap("one two", 10), ["one two"]);
}
