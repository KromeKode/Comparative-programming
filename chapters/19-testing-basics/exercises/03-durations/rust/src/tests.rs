// Write tests for parse_duration and format_duration here, as #[test] functions. One
// is here to start with, and the README lists what they promise.
use super::*;

#[test]
fn parses() {
    assert_eq!(parse_duration("1h30m"), Ok(5400));
}
