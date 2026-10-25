fn main() {
    let a = 0.1;
    let b = 0.2;
    let sum: f64 = a + b;

    println!("0.1 + 0.2 = {sum}");
    println!("equal to 0.3? {}", if sum == 0.3 { "yes" } else { "no" });
    println!("close to 0.3? {}", if (sum - 0.3).abs() < 1e-9 { "yes" } else { "no" });
}
