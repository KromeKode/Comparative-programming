use std::thread;

const LIMIT: u64 = 200_000;
const WORKERS: u64 = 4;

fn is_prime(n: u64) -> bool {
    n >= 2 && (2..).take_while(|d| d * d <= n).all(|d| n % d != 0)
}

fn main() {
    println!("counting primes below {LIMIT} with {WORKERS} workers");

    // thread::spawn runs a closure on a new thread. move gives the closure
    // its own copies of start and end, since the thread may outlive them.
    let handles: Vec<_> = (0..WORKERS)
        .map(|i| {
            let (start, end) = (LIMIT * i / WORKERS, LIMIT * (i + 1) / WORKERS);
            let handle = thread::spawn(move || (start..end).filter(|&n| is_prime(n)).count());
            (start, end, handle)
        })
        .collect();

    // join waits for the thread and returns its result, or Err if it panicked.
    let mut total = 0;
    for (i, (start, end, handle)) in handles.into_iter().enumerate() {
        let count = handle.join().expect("a worker panicked");
        println!("worker {i}: [{start}, {end}) has {count} primes");
        total += count;
    }
    println!("total: {total} primes");
}
