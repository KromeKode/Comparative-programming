use std::fmt;
use std::io::{self, Read};

const MAX_ATTEMPTS: usize = 3;

#[derive(Debug, Clone, Copy)]
enum Failure {
    TimedOut,
    ServerBusy,
    NotFound,
    LoginRefused,
}

impl Failure {
    // Try the same file again?
    fn is_retryable(self) -> bool {
        matches!(self, Failure::TimedOut | Failure::ServerBusy)
    }

    // Stop the whole batch?
    fn is_fatal(self) -> bool {
        matches!(self, Failure::LoginRefused)
    }
}

impl fmt::Display for Failure {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let text = match self {
            Failure::TimedOut => "timed out",
            Failure::ServerBusy => "server busy",
            Failure::NotFound => "not found",
            Failure::LoginRefused => "login refused",
        };
        write!(f, "{text}")
    }
}

// How download_with_retries can fail: one failure, or too many in a row.
enum DownloadError {
    Failed(Failure),
    GaveUp { attempts: usize, last: Failure },
}

impl fmt::Display for DownloadError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            DownloadError::Failed(failure) => write!(f, "{failure}"),
            DownloadError::GaveUp { attempts, last } => {
                write!(f, "gave up after {attempts} attempts: {last}")
            }
        }
    }
}

// The pretend server: the answer for this attempt. The last answer repeats.
fn download(answers: &[&str], attempt: usize) -> Result<(), Failure> {
    match answers[attempt.min(answers.len()) - 1] {
        "timeout" => Err(Failure::TimedOut),
        "busy" => Err(Failure::ServerBusy),
        "missing" => Err(Failure::NotFound),
        "auth" => Err(Failure::LoginRefused),
        _ => Ok(()),
    }
}

// Return the number of attempts it took, or the error that stopped it.
fn download_with_retries(name: &str, answers: &[&str]) -> Result<usize, DownloadError> {
    let mut wait = 1;
    for attempt in 1..=MAX_ATTEMPTS {
        match download(answers, attempt) {
            Ok(()) => return Ok(attempt),
            Err(failure) if !failure.is_retryable() => {
                return Err(DownloadError::Failed(failure));
            }
            Err(last) if attempt == MAX_ATTEMPTS => {
                return Err(DownloadError::GaveUp {
                    attempts: attempt,
                    last,
                });
            }
            Err(failure) => {
                println!("{name}: {failure}, retrying in {wait}s");
                wait *= 2;
            }
        }
    }
    unreachable!("the last attempt always returns")
}

fn main() {
    let mut input = String::new();
    let mut stdin = io::stdin();
    stdin.read_to_string(&mut input).expect("stdin is readable");
    let jobs: Vec<Vec<&str>> = input
        .lines()
        .map(|line| line.split_whitespace().collect::<Vec<_>>())
        .filter(|words| !words.is_empty())
        .collect();

    let (mut downloaded, mut skipped, mut failed, mut not_tried) = (0, 0, 0, 0);
    for (index, job) in jobs.iter().enumerate() {
        let (name, answers) = (job[0], &job[1..]);
        match download_with_retries(name, answers) {
            Ok(attempts) => {
                let plural = if attempts == 1 { "" } else { "s" };
                println!("{name}: ok after {attempts} attempt{plural}");
                downloaded += 1;
            }
            Err(DownloadError::Failed(failure)) if failure.is_fatal() => {
                println!("{name}: stopped: {failure}");
                failed += 1;
                not_tried = jobs.len() - index - 1;
                break;
            }
            Err(DownloadError::Failed(failure)) => {
                println!("{name}: skipped: {failure}");
                skipped += 1;
            }
            Err(error @ DownloadError::GaveUp { .. }) => {
                println!("{name}: failed: {error}");
                failed += 1;
            }
        }
    }
    println!("downloaded {downloaded}, skipped {skipped}, failed {failed}, not tried {not_tried}");
}
