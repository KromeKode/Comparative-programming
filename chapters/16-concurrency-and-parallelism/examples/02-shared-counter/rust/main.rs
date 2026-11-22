use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, Mutex};
use std::thread;

const THREADS: usize = 4;
const TIMES: u64 = 100_000;

fn main() {
    println!("{THREADS} threads add 1, {TIMES} times each");

    // A Mutex owns the value it protects, and lock() is the only way to reach
    // it. Arc is a reference count that threads can share, so each thread
    // gets its own handle to the one Mutex.
    let locked_count = Arc::new(Mutex::new(0));
    let handles: Vec<_> = (0..THREADS)
        .map(|_| {
            let count = Arc::clone(&locked_count);
            thread::spawn(move || {
                for _ in 0..TIMES {
                    // The guard unlocks the mutex when it goes out of scope.
                    *count.lock().unwrap() += 1;
                }
            })
        })
        .collect();
    for handle in handles {
        handle.join().unwrap();
    }
    println!("with a mutex: {}", locked_count.lock().unwrap());

    // thread::scope waits for every thread spawned in it before it returns,
    // so they can borrow local variables, and no Arc is needed.
    let atomic_count = AtomicU64::new(0);
    thread::scope(|scope| {
        for _ in 0..THREADS {
            scope.spawn(|| {
                for _ in 0..TIMES {
                    atomic_count.fetch_add(1, Ordering::Relaxed);
                }
            });
        }
    });
    println!("with an atomic: {}", atomic_count.load(Ordering::Relaxed));
}
