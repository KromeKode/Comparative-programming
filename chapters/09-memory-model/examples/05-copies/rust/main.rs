#[derive(Clone)]
struct Player {
    name: String,
    scores: Vec<i32>,
}

#[derive(Clone, Copy)]
struct Point {
    x: i32,
    y: i32,
}

fn main() {
    // clone() copies every field, and String and Vec clone their heap memory,
    // so the copy is deep.
    let a = Player {
        name: String::from("Ada"),
        scores: vec![90, 85],
    };
    let mut b = a.clone();
    b.name.replace_range(0..1, "E");
    b.scores.push(70);
    println!(
        "a.clone() copies everything: a = {} {:?}, b = {} {:?}",
        a.name, a.scores, b.name, b.scores
    );

    // Plain assignment moves a Player instead of copying it. The compiler then
    // refuses any use of `a` (chapter 10).
    let c = a;
    println!("let c = a moves it: c.name = {}", c.name);

    // A Copy type is copied bit for bit on assignment, like a C struct.
    let p = Point { x: 1, y: 2 };
    let mut q = p;
    q.x = 5;
    println!(
        "let q = p copies a Copy type: p = ({}, {}), q = ({}, {})",
        p.x, p.y, q.x, q.y
    );

    let mut team = vec![c, b];
    for player in team.iter_mut() {
        player.name.push('!');
    }
    let names: Vec<&str> = team.iter().map(|player| player.name.as_str()).collect();
    println!("iter_mut changes the players: {names:?}");
}
