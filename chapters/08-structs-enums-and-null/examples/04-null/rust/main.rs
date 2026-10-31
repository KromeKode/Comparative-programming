struct User {
    name: String,
    nickname: Option<String>, // None when the user has no nickname
}

fn find<'a>(users: &'a [User], name: &str) -> Option<&'a User> {
    users.iter().find(|user| user.name == name)
}

fn main() {
    let users = vec![
        User {
            name: "ada".to_string(),
            nickname: Some("Countess".to_string()),
        },
        User {
            name: "alan".to_string(),
            nickname: None,
        },
        User {
            name: "grace".to_string(),
            nickname: Some("Amazing Grace".to_string()),
        },
    ];

    for user in &users {
        // unwrap_or gives a default when the Option is None.
        let shown = user.nickname.as_deref().unwrap_or(&user.name);
        println!("{} is shown as {shown}", user.name);
    }

    // map_or applies a function to the value inside, or gives a default for None.
    let lengths: Vec<String> = users
        .iter()
        .map(|user| user.nickname.as_deref().map_or(0, str::len).to_string())
        .collect();
    println!("nickname lengths: {}", lengths.join(" "));

    for name in ["alan", "linus"] {
        match find(&users, name) {
            Some(user) => println!("find \"{name}\": {}", user.name),
            None => println!("find \"{name}\": nobody"),
        }
    }

    for name in ["ada", "alan", "linus"] {
        // and_then chains a second step that may also give None.
        let nickname = find(&users, name).and_then(|user| user.nickname.as_deref());
        println!("nickname of \"{name}\": {}", nickname.unwrap_or("none"));
    }
}
