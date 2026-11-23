use super::*;

// A table of cases, and one loop that checks them all. The message after the two
// values says which case failed.
#[test]
fn numerals() {
    let cases = [
        (1, "I"),
        (4, "IV"),
        (9, "IX"),
        (14, "XIV"),
        (40, "XL"),
        (90, "XC"),
        (400, "CD"),
        (1994, "MCMXCIV"),
        (2024, "MMXXIV"),
        (3999, "MMMCMXCIX"),
    ];
    for (n, want) in cases {
        assert_eq!(to_roman(n), Ok(want.to_string()), "to_roman({n})");
    }
}

// The errors are part of what to_roman promises, so they get tests too.
#[test]
fn out_of_range() {
    assert_eq!(to_roman(0), Err(OutOfRange(0)));
    assert_eq!(to_roman(4000), Err(OutOfRange(4000)));
}
