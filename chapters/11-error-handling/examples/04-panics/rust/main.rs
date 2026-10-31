use std::panic;

struct User {
    id: u32,
    name: String,
}

fn find_user(users: &[User], id: u32) -> Option<&User> {
    users.iter().find(|user| user.id == id)
}

// Run action and report what it returned or the message it panicked with.
fn attempt(label: &str, action: impl FnOnce() -> String + panic::UnwindSafe) {
    match panic::catch_unwind(action) {
        Ok(value) => println!("{label}: {value}"),
        Err(payload) => {
            // A panic's payload is usually a String or a &str.
            let message = payload
                .downcast_ref::<String>()
                .map(String::as_str)
                .or_else(|| payload.downcast_ref::<&str>().copied())
                .unwrap_or("(no message)");
            println!("{label}: panicked: {message}");
        }
    }
}

fn main() {
    // By default a panic prints its message to stderr. This hook prints nothing,
    // so attempt can print the message itself.
    panic::set_hook(Box::new(|_| {}));

    let numbers = vec![10, 20, 30];
    let users = vec![User {
        id: 1,
        name: "ada".to_string(),
    }];
    let (index, zero) = (5, 0);

    // Each of these is a bug, and each one panics.
    attempt("numbers[5]", || numbers[index].to_string());
    attempt("10 / zero", || (10 / zero).to_string());
    attempt("find_user(7).unwrap().name", || {
        find_user(&users, 7).unwrap().name.clone()
    });
    attempt("assert!(numbers.len() == 4)", || {
        assert!(numbers.len() == 4);
        "passed".to_string()
    });

    // The checked versions return an Option instead.
    attempt("numbers.get(5)", || format!("{:?}", numbers.get(index)));
    attempt("10.checked_div(zero)", || {
        format!("{:?}", 10_i32.checked_div(zero))
    });
}
