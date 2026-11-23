//! Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
//! Kelvin. clap reads the options from the struct `Args`, writes --help from its doc
//! comments, and exits with status 2, after a message on stderr, when the command line
//! is wrong.
use clap::{Parser, ValueEnum};
use std::process::ExitCode;

#[derive(Clone, Copy, ValueEnum)]
enum Unit {
    C,
    F,
    K,
}

impl Unit {
    fn letter(self) -> char {
        match self {
            Unit::C => 'c',
            Unit::F => 'f',
            Unit::K => 'k',
        }
    }
}

fn to_celsius(t: f64, unit: Unit) -> f64 {
    match unit {
        Unit::C => t,
        Unit::F => (t - 32.0) * 5.0 / 9.0,
        Unit::K => t - 273.15,
    }
}

fn from_celsius(t: f64, unit: Unit) -> f64 {
    match unit {
        Unit::C => t,
        Unit::F => t * 9.0 / 5.0 + 32.0,
        Unit::K => t + 273.15,
    }
}

/// Converts temperatures, such as 21.5c, -40f, or 300k.
#[derive(Parser)]
#[command(name = "temp", bin_name = "temp")]
struct Args {
    /// The unit to convert to
    #[arg(short, long, value_enum, default_value = "c")]
    to: Unit,

    /// Digits after the point, 0 to 9
    #[arg(short, long, default_value_t = 1, value_parser = clap::value_parser!(u8).range(0..=9))]
    digits: u8,

    #[arg(required = true, value_name = "TEMPERATURE")]
    temperatures: Vec<String>,
}

/// A number followed by c, f, or k.
fn parse_temperature(text: &str) -> Option<(f64, Unit)> {
    let unit = match text.chars().last()? {
        'c' => Unit::C,
        'f' => Unit::F,
        'k' => Unit::K,
        _ => return None,
    };
    let value: f64 = text[..text.len() - 1].parse().ok()?;
    value.is_finite().then_some((value, unit))
}

fn main() -> ExitCode {
    let args = Args::parse(); // exits with status 2 if the arguments are wrong

    // Check every temperature before printing any, so that a mistake prints nothing.
    let mut temperatures = Vec::new();
    for text in &args.temperatures {
        match parse_temperature(text) {
            Some((value, unit)) => temperatures.push((text, value, unit)),
            None => {
                eprintln!("temp: not a temperature: {text}");
                return ExitCode::from(2);
            }
        }
    }

    for (text, value, unit) in temperatures {
        let converted = from_celsius(to_celsius(value, unit), args.to);
        let digits = usize::from(args.digits);
        println!("{text} = {converted:.digits$}{}", args.to.letter());
    }
    ExitCode::SUCCESS
}
