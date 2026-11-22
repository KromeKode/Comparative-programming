// A failed check returns from the test at once, with the check's text.
macro_rules! check {
    ($cond:expr) => {
        if !$cond {
            return Err(stringify!($cond));
        }
    };
}

type TestFn = fn() -> Result<(), &'static str>;

// tests! takes ordinary-looking functions, gives each one a Result to
// return, and builds the list of tests from their names.
macro_rules! tests {
    ($(fn $name:ident() $body:block)*) => {
        $(
            fn $name() -> Result<(), &'static str> {
                $body
                Ok(())
            }
        )*

        const TESTS: &[(&str, TestFn)] = &[$((stringify!($name), $name)),*];
    };
}

tests! {
    fn addition() {
        check!(2 + 2 == 4);
        check!(-3 + 3 == 0);
    }

    fn strings() {
        let word = "abc".to_uppercase();
        check!(word == "ABC");
        check!(word.len() == 3);
    }

    fn rounding() {
        check!(0.5 + 0.25 == 0.75);
        check!(0.1 + 0.2 == 0.3);
    }

    fn sorting() {
        let mut numbers = vec![3, 1, 2];
        numbers.sort();
        check!(numbers == [1, 2, 3]);
    }
}

fn main() {
    let mut failed = 0;
    println!("running {} tests", TESTS.len());
    for (name, run) in TESTS {
        match run() {
            Ok(()) => println!("PASS {name}"),
            Err(text) => {
                println!("FAIL {name}: {text}");
                failed += 1;
            }
        }
    }
    println!("{} passed, {failed} failed", TESTS.len() - failed);
    // a real runner would exit with 1 when a test fails
}
