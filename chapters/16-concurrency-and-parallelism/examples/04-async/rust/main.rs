use std::cell::Cell;
use std::collections::VecDeque;
use std::future::Future;
use std::pin::Pin;
use std::sync::{Arc, Mutex};
use std::task::{Context, Poll, Wake, Waker};

// Rust has async, await, and the Future trait, but its standard library has no
// event loop to run futures on. Programs use a crate such as tokio for that.
// This is a small one, to show what such a loop does.

// A future that isn't ready the first time it is polled. It asks to be polled
// again, which puts its task at the back of the line.
struct Pause {
    paused: bool,
}

impl Future for Pause {
    type Output = ();

    fn poll(mut self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<()> {
        if self.paused {
            return Poll::Ready(());
        }
        self.paused = true;
        cx.waker().wake_by_ref();
        Poll::Pending
    }
}

fn pause() -> Pause {
    Pause { paused: false }
}

// Waking a task puts its number at the back of the queue of tasks that are
// ready to run. A Waker has to be Send and Sync, even on one thread, so the
// queue is behind an Arc and a Mutex.
struct TaskWaker {
    id: usize,
    ready: Arc<Mutex<VecDeque<usize>>>,
}

impl Wake for TaskWaker {
    fn wake(self: Arc<Self>) {
        self.ready.lock().unwrap().push_back(self.id);
    }
}

// A future on the heap, pinned there because it can hold references into
// itself. 'a is how long the things it borrows live.
type Task<'a, T> = Pin<Box<dyn Future<Output = T> + 'a>>;

fn spawn<'a, T>(future: impl Future<Output = T> + 'a) -> Task<'a, T> {
    Box::pin(future)
}

// Runs the tasks on this one thread until all of them finish, and returns
// their results in order.
fn run_all<T>(mut tasks: Vec<Task<'_, T>>) -> Vec<T> {
    let ready = Arc::new(Mutex::new((0..tasks.len()).collect::<VecDeque<_>>()));
    let wakers: Vec<Waker> = (0..tasks.len())
        .map(|id| {
            let ready = Arc::clone(&ready);
            Waker::from(Arc::new(TaskWaker { id, ready }))
        })
        .collect();
    let mut results: Vec<Option<T>> = tasks.iter().map(|_| None).collect();
    loop {
        // The lock is released at the end of this statement, before the poll,
        // which can wake the task and lock the queue again.
        let next = ready.lock().unwrap().pop_front();
        let Some(id) = next else { break };
        // poll runs the task up to its next await that isn't ready.
        let mut cx = Context::from_waker(&wakers[id]);
        if let Poll::Ready(result) = tasks[id].as_mut().poll(&mut cx) {
            results[id] = Some(result);
        }
    }
    results
        .into_iter()
        .map(|result| result.expect("a task never finished"))
        .collect()
}

async fn task(name: &str, steps: u32) -> u32 {
    for step in 1..=steps {
        println!("{name}: step {step}");
        // await is the only place a task can stop and let another one run.
        pause().await;
    }
    println!("{name}: done");
    steps
}

async fn add_at_once(count: &Cell<u32>) {
    count.set(count.get() + 1); // no await, so no other task can run in the middle
}

async fn add_with_await(count: &Cell<u32>) {
    let value = count.get();
    pause().await; // every other task reads the old value here
    count.set(value + 1);
}

fn main() {
    println!("three tasks, each pausing at every await");
    // Calling an async fn runs none of it. It returns a future, which runs
    // only when something polls it.
    let results = run_all(vec![
        spawn(task("a", 2)),
        spawn(task("b", 3)),
        spawn(task("c", 1)),
    ]);
    let results: Vec<String> = results.iter().map(u32::to_string).collect();
    println!("results: {}", results.join(", "));

    println!("4 tasks add 1 to a shared count");
    // The tasks share count through a Cell, which can be changed through a
    // shared reference, and can't be sent to another thread.
    let count = Cell::new(0);
    run_all((0..4).map(|_| spawn(add_at_once(&count))).collect());
    println!("with no await between reading and writing: {}", count.get());
    let count = Cell::new(0);
    run_all((0..4).map(|_| spawn(add_with_await(&count))).collect());
    println!("with an await between reading and writing: {}", count.get());
}
