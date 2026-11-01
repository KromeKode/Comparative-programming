// Privacy in Rust is per module: code outside `mod fraction` can't see the
// fields, so it can't make a fraction that is not in lowest terms.
mod fraction {
    use std::cmp::Ordering;
    use std::fmt;
    use std::ops::Add;

    // derive(PartialEq) compares the fields, which is right in lowest terms.
    #[derive(Clone, Copy, Debug, PartialEq, Eq)]
    pub struct Fraction {
        numerator: i64,
        denominator: i64, // always positive
    }

    fn gcd(a: i64, b: i64) -> i64 {
        if b == 0 { a.abs() } else { gcd(b, a % b) }
    }

    impl Fraction {
        pub fn new(numerator: i64, denominator: i64) -> Result<Fraction, String> {
            if denominator == 0 {
                return Err("denominator is zero".to_string());
            }
            let sign = denominator.signum();
            let divisor = gcd(numerator, denominator);
            Ok(Fraction {
                numerator: sign * numerator / divisor,
                denominator: sign * denominator / divisor,
            })
        }
    }

    impl Add for Fraction {
        type Output = Fraction;

        fn add(self, other: Fraction) -> Fraction {
            Fraction::new(
                self.numerator * other.denominator + other.numerator * self.denominator,
                self.denominator * other.denominator,
            )
            .expect("the product of two denominators is never zero")
        }
    }

    // derive(PartialOrd) would compare the numerators first, which is wrong
    // for 1/3 and 1/2, so Ord is written by hand.
    impl Ord for Fraction {
        fn cmp(&self, other: &Fraction) -> Ordering {
            (self.numerator * other.denominator).cmp(&(other.numerator * self.denominator))
        }
    }

    impl PartialOrd for Fraction {
        fn partial_cmp(&self, other: &Fraction) -> Option<Ordering> {
            Some(self.cmp(other))
        }
    }

    impl fmt::Display for Fraction {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            if self.denominator == 1 {
                write!(f, "{}", self.numerator)
            } else {
                write!(f, "{}/{}", self.numerator, self.denominator)
            }
        }
    }
}

use fraction::Fraction;

fn yes_no(value: bool) -> &'static str {
    if value { "yes" } else { "no" }
}

fn main() -> Result<(), String> {
    let (half, third) = (Fraction::new(1, 2)?, Fraction::new(1, 3)?);
    println!("1/2 + 1/3 = {}", half + third);
    println!("2/-4 is stored as {}", Fraction::new(2, -4)?);
    println!("6/3 is stored as {}", Fraction::new(6, 3)?);
    if let Err(error) = Fraction::new(1, 0) {
        println!("1/0: error: {error}");
    }
    println!("1/2 == 2/4: {}", yes_no(half == Fraction::new(2, 4)?));
    println!("1/3 < 1/2: {}", yes_no(third < half));
    let mut fractions = vec![
        Fraction::new(3, 4)?,
        third,
        Fraction::new(-1, 2)?,
        Fraction::new(2, 4)?,
    ];
    fractions.sort();
    let text: Vec<String> = fractions.iter().map(|f| f.to_string()).collect();
    println!("sorted: {}", text.join(", "));
    Ok(())
}
