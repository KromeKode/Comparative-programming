use super::*;

#[test]
fn parses() {
    let cases = [
        ("45s", 45),
        ("1h30m", 5400),
        ("2d", 172800),
        ("1d2h3m4s", 93784),
        // A number can be more than the next unit up holds, and it can be 0.
        ("90m", 5400),
        ("0s", 0),
        ("1h0m", 3600),
    ];
    for (text, seconds) in cases {
        assert_eq!(
            parse_duration(text),
            Ok(seconds),
            "parse_duration({text:?})"
        );
    }
}

#[test]
fn rejects() {
    let cases = [
        "",       // no parts
        "90",     // a number with no unit
        "1h30",   // the same, after a part
        "h",      // a unit with no number
        "1x",     // not a unit
        "1h1h",   // a unit twice
        "30m1h",  // units out of order
        "1h 30m", // anything else
        "-1h",
    ];
    for text in cases {
        assert!(parse_duration(text).is_err(), "parse_duration({text:?})");
    }
}

#[test]
fn formats() {
    let cases = [
        (5400, "1h30m"),
        (93784, "1d2h3m4s"),
        (59, "59s"),
        // Units that are 0 are left out, wherever they are, unless everything is.
        (3600, "1h"),
        (86401, "1d1s"),
        (0, "0s"),
    ];
    for (seconds, text) in cases {
        assert_eq!(format_duration(seconds), text, "format_duration({seconds})");
    }
}

#[test]
fn round_trips() {
    for seconds in 0..100_000 {
        assert_eq!(parse_duration(&format_duration(seconds)), Ok(seconds));
    }
}
