/// The error for a number that has no Roman numeral. `PartialEq` lets tests compare
/// it with `assert_eq!`, and `Debug` lets `assert_eq!` print it when they differ.
#[derive(Debug, PartialEq)]
pub struct OutOfRange(pub u32);

const NUMERALS: &[(u32, &str)] = &[
    (1000, "M"),
    (900, "CM"),
    (500, "D"),
    (400, "CD"),
    (100, "C"),
    (90, "XC"),
    (50, "L"),
    (40, "XL"),
    (10, "X"),
    (9, "IX"),
    (5, "V"),
    (4, "IV"),
    (1, "I"),
];

/// `n` in Roman numerals, or an error if `n` is outside 1 to 3999.
pub fn to_roman(mut n: u32) -> Result<String, OutOfRange> {
    if !(1..=3999).contains(&n) {
        return Err(OutOfRange(n));
    }
    let mut numeral = String::new();
    for &(value, letters) in NUMERALS {
        while n >= value {
            numeral.push_str(letters);
            n -= value;
        }
    }
    Ok(numeral)
}

#[cfg(test)]
mod tests;
