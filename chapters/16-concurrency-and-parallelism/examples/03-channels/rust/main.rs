use std::sync::mpsc;
use std::sync::{Arc, Mutex};
use std::thread;

const COUNT: u64 = 1000;
const WORKERS: usize = 3;

fn main() {
    // mpsc means many producers, one consumer: a Sender can be cloned, and
    // the Receiver can't. sync_channel(16) holds up to 16 values, and a send
    // to a full channel waits for room.
    let (job_sender, job_receiver) = mpsc::sync_channel::<u64>(16);
    let (result_sender, result_receiver) = mpsc::sync_channel::<u64>(16);

    // Several workers take jobs from one Receiver, so they share it behind a
    // Mutex, and each one locks it to take a job.
    let job_receiver = Arc::new(Mutex::new(job_receiver));
    for _ in 0..WORKERS {
        let jobs = Arc::clone(&job_receiver);
        let results = result_sender.clone();
        thread::spawn(move || {
            loop {
                // The lock is released at the end of this statement, before
                // the work starts. recv returns Err once every Sender is gone.
                let job = jobs.lock().unwrap().recv();
                let Ok(n) = job else { break };
                results.send(n * n).unwrap();
            }
        });
    }
    // Only the workers' clones should keep the results channel open.
    drop(result_sender);

    thread::spawn(move || {
        for n in 1..=COUNT {
            job_sender.send(n).unwrap();
        }
        // job_sender is dropped here, which closes the channel.
    });

    // The loop ends when every Sender of results has been dropped.
    let (mut received, mut sum) = (0, 0);
    for square in result_receiver {
        received += 1;
        sum += square;
    }
    println!("{WORKERS} workers squared 1 to {COUNT}");
    println!("results: {received}");
    println!("sum: {sum}");
}
