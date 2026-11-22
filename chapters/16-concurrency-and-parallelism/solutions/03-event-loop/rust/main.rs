use std::cmp::Reverse;
use std::collections::{BinaryHeap, HashMap, VecDeque};
use std::io::{self, Read};

enum Step {
    Print(String),
    Sleep(u64),
    Wait(usize), // the number of the task to wait for
}

struct Task {
    name: String,
    steps: Vec<Step>,
    next: usize, // the index of the step to run when it runs again
    done: bool,
    waiting_for: usize,
    waiters: Vec<usize>, // in the order they began waiting
}

// Returns the tasks, and prints an error for each line with a problem.
fn parse(input: &str) -> Vec<Task> {
    let split = |line: &str| {
        let line = line.trim();
        let (command, rest) = line.split_once(' ').unwrap_or((line, ""));
        (command.to_string(), rest.trim().to_string())
    };
    // A wait can name a task defined further down, so the names come first.
    let mut numbers = HashMap::new();
    for line in input.lines() {
        let (command, name) = split(line);
        if command == "task" {
            let count = numbers.len();
            numbers.entry(name).or_insert(count);
        }
    }
    let mut tasks: Vec<Task> = Vec::new();
    for (i, line) in input.lines().enumerate() {
        let number = i + 1;
        let (command, rest) = split(line);
        if command.is_empty() {
            continue;
        }
        if command == "task" {
            let steps = Vec::new();
            tasks.push(Task {
                name: rest,
                steps,
                next: 0,
                done: false,
                waiting_for: 0,
                waiters: vec![],
            });
            continue;
        }
        let Some(task) = tasks.last_mut() else {
            println!("error: line {number}: a step before any task");
            continue;
        };
        match command.as_str() {
            "print" => task.steps.push(Step::Print(rest)),
            "sleep" => match rest.parse() {
                Ok(duration) if rest.bytes().all(|b| b.is_ascii_digit()) => {
                    task.steps.push(Step::Sleep(duration))
                }
                _ => println!("error: line {number}: sleep takes a whole number"),
            },
            "wait" => match numbers.get(&rest) {
                Some(&other) => task.steps.push(Step::Wait(other)),
                None => println!("error: line {number}: no task named {rest}"),
            },
            _ => println!("error: line {number}: unknown step: {command}"),
        }
    }
    tasks
}

struct Loop {
    tasks: Vec<Task>,
    time: u64,
    ready: VecDeque<usize>, // the tasks that can run now, in order
    // The sleeping tasks, as (time to wake, order of the sleep, task).
    // BinaryHeap pops its largest item, so Reverse makes it pop the smallest.
    sleeping: BinaryHeap<Reverse<(u64, u64, usize)>>,
    sleeps: u64,
}

impl Loop {
    // Runs a task until it sleeps, waits, or ends.
    fn run_until_stopped(&mut self, id: usize) {
        while self.tasks[id].next < self.tasks[id].steps.len() {
            let next = self.tasks[id].next;
            self.tasks[id].next += 1;
            // A task is named by its number, not borrowed, so that this can
            // change the task and the one it waits for.
            match self.tasks[id].steps[next] {
                Step::Print(ref text) => {
                    println!("[{}] {}: {text}", self.time, self.tasks[id].name)
                }
                Step::Sleep(0) => {
                    self.ready.push_back(id);
                    return;
                }
                Step::Sleep(duration) => {
                    self.sleeping
                        .push(Reverse((self.time + duration, self.sleeps, id)));
                    self.sleeps += 1;
                    return;
                }
                Step::Wait(other) if !self.tasks[other].done => {
                    self.tasks[id].waiting_for = other;
                    self.tasks[other].waiters.push(id);
                    return;
                }
                Step::Wait(_) => {} // it has finished already, so carry on
            }
        }
        let task = &mut self.tasks[id];
        task.done = true;
        println!("[{}] {}: done", self.time, task.name);
        self.ready.extend(task.waiters.drain(..));
    }

    fn run(&mut self) {
        loop {
            while let Some(id) = self.ready.pop_front() {
                self.run_until_stopped(id);
            }
            let Some(&Reverse((wake, _, _))) = self.sleeping.peek() else {
                break;
            };
            // Nothing can run now, so the clock jumps to the next wake.
            self.time = wake;
            while let Some(&Reverse((wake, _, id))) = self.sleeping.peek()
                && wake == self.time
            {
                self.sleeping.pop();
                self.ready.push_back(id);
            }
        }
        let stuck: Vec<String> = self
            .tasks
            .iter()
            .filter(|task| !task.done)
            .map(|task| {
                format!(
                    "{} waits for {}",
                    task.name, self.tasks[task.waiting_for].name
                )
            })
            .collect();
        if stuck.is_empty() {
            println!("[{}] all done", self.time);
        } else {
            println!("[{}] deadlock: {}", self.time, stuck.join(", "));
        }
    }
}

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("can't read stdin");
    let tasks = parse(&input);
    let ready = (0..tasks.len()).collect();
    let mut event_loop = Loop {
        tasks,
        time: 0,
        ready,
        sleeping: BinaryHeap::new(),
        sleeps: 0,
    };
    event_loop.run();
}
