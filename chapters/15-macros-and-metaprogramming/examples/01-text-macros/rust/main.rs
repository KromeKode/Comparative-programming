use std::cell::Cell;

// macro_rules! defines a macro by example. Each rule matches the tokens the
// macro is given and says what code to put in their place. $x:expr matches
// one whole expression, and it stays one expression when it is pasted in.
macro_rules! square {
    ($x:expr) => {
        $x * $x
    };
}

// This one runs its argument once, in a block of its own.
macro_rules! square_once {
    ($x:expr) => {{
        let x = $x;
        x * x
    }};
}

// stringify! turns tokens into a string, as # does in C, and line! is the
// line the macro was used on.
macro_rules! show {
    ($e:expr) => {
        println!("line {}: {} = {}", line!(), stringify!($e), $e)
    };
}

// Macros are hygienic: the tmp below belongs to the macro, so it can't clash
// with a tmp of the caller's, even one passed in as $a.
macro_rules! swap {
    ($a:ident, $b:ident) => {
        let tmp = $a;
        $a = $b;
        $b = tmp;
    };
}

const LIMIT: i32 = 3;

fn main() {
    show!(LIMIT * 2);
    show!(square!(1 + 2));

    let clicks = Cell::new(0);
    let bump = || {
        clicks.set(clicks.get() + 1);
        clicks.get()
    };
    // The argument is pasted in twice, so the call in it runs twice.
    show!(square!(bump()));
    show!(square_once!(bump()));
    show!(clicks.get());

    let mut tmp = 1;
    let mut other = 2;
    swap!(tmp, other);
    println!("after swap!(tmp, other): tmp = {tmp}, other = {other}");

    // cfg! asks how the program is being compiled. rustc -O turns
    // debug_assertions off, and with it every debug_assert!.
    if cfg!(debug_assertions) {
        println!("debug_assertions is on, so debug_assert! checks run");
    } else {
        println!("debug_assertions is off, so debug_assert! compiles to nothing");
    }
}
