// A pretend file that closes when it is dropped.
struct File {
    name: &'static str,
}

impl File {
    fn open(name: &'static str, fails: bool) -> Result<File, String> {
        if fails {
            return Err(format!("cannot open {name}"));
        }
        println!("  open {name}");
        Ok(File { name })
    }
}

impl Drop for File {
    fn drop(&mut self) {
        println!("  close {}", self.name);
    }
}

// No cleanup code at all. When the function returns, whether at the end or
// early through `?`, its owned values are dropped in the reverse order.
fn run_job(output_fails: bool, copy_fails: bool) -> Result<(), String> {
    let _input = File::open("input", false)?;
    let _output = File::open("output", output_fails)?;
    if copy_fails {
        return Err("disk full".to_string());
    }
    println!("  copy input to output");
    Ok(())
}

fn main() {
    let jobs = [
        ("everything works", false, false),
        ("opening output fails", true, false),
        ("copying fails", false, true),
    ];
    for (i, (title, output_fails, copy_fails)) in jobs.into_iter().enumerate() {
        let number = i + 1;
        println!("job {number}: {title}");
        match run_job(output_fails, copy_fails) {
            Ok(()) => println!("  job {number} done"),
            Err(message) => println!("  job {number} failed: {message}"),
        }
    }
}
