/// Whether `year` is a leap year in the Gregorian calendar: one divisible by 4,
/// except for centuries, which must be divisible by 400.
pub fn is_leap_year(year: i32) -> bool {
    year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)
}

// The tests are a module of this crate, compiled only by `cargo test`. They are
// often written in this file, as `mod tests { ... }`; here they are in tests.rs.
#[cfg(test)]
mod tests;
