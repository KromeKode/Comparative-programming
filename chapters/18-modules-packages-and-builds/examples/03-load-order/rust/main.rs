mod settings;

fn main() {
    println!("main: start");
    println!("main: the greeting is {}", *settings::GREETING);
    println!("main: the greeting is still {}", *settings::GREETING);
}
