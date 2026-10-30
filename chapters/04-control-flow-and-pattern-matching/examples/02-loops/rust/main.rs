fn main() {
    print!("up:");
    for i in 0..5 {
        print!(" {i}"); // 0..5 stops before 5
    }
    println!();

    print!("down:");
    for i in (1..=5).rev() {
        print!(" {i}"); // 1..=5 includes 5
    }
    println!();

    print!("by threes:");
    for i in (0..10).step_by(3) {
        print!(" {i}");
    }
    println!();

    print!("halving:");
    let mut n = 100;
    while n > 0 {
        print!(" {n}");
        n /= 2;
    }
    println!();

    let colors = ["red", "green", "blue"];
    print!("colors:");
    for color in colors {
        print!(" {color}");
    }
    println!();

    print!("numbered:");
    for (i, color) in colors.iter().enumerate() {
        print!(" {}={color}", i + 1);
    }
    println!();

    // Rust has no do-while. A loop with the test at the end does the same job.
    let mut m = 100;
    loop {
        m *= 2;
        if m >= 50 {
            break;
        }
    }
    println!("do-while: {m}");
}
