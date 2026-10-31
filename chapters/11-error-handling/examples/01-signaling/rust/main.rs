fn main() {
    let inputs = ["42", "4x2", "", "99999999999999999999"];

    // parse returns a Result: Ok with the number, or Err with the reason. The
    // compiler will not let you use the number without dealing with the Err.
    println!("parse::<i32>:");
    for text in inputs {
        match text.parse::<i32>() {
            Ok(value) => println!("  {text:?} -> Ok({value})"),
            Err(error) => println!("  {text:?} -> Err({:?}): {error}", error.kind()),
        }
    }

    // When you don't need the reason, turn the Result into an Option or a
    // default.
    println!("ok and unwrap_or:");
    for text in inputs {
        let maybe = text.parse::<i32>().ok();
        let or_zero = text.parse::<i32>().unwrap_or(0);
        println!("  {text:?} -> {maybe:?}, or {or_zero}");
    }
}
