// Reads a trace of function calls and prints a flat profile: for each function, how
// many calls it got, its total time, and its self time. See ../README.md.
use std::collections::HashMap;
use std::io::Read;

#[derive(Default)]
struct Function {
    calls: u64,
    total: u64,
    self_time: u64,
    running: u64,         // how many of its calls are running now
    outermost_start: u64, // when the outermost of them started
}

struct Call<'a> {
    name: &'a str,
    start: u64,
    in_callees: u64, // time spent in the functions it called
}

fn is_time(s: &str) -> bool {
    !s.is_empty() && s.len() <= 12 && s.bytes().all(|b| b.is_ascii_digit())
}

fn is_name(s: &str) -> bool {
    s.bytes().all(|b| (b'!'..=b'~').contains(&b))
}

fn profile(text: &str) -> Result<HashMap<&str, Function>, String> {
    let mut functions: HashMap<&str, Function> = HashMap::new();
    let mut stack: Vec<Call> = Vec::new();
    let mut now = 0;
    for (number, line) in (1..).zip(text.lines()) {
        let fields: Vec<&str> = line.split([' ', '\t']).filter(|f| !f.is_empty()).collect();
        if fields.is_empty() {
            continue;
        }
        let [time, event @ ("enter" | "exit"), name] = fields[..] else {
            return Err(format!(
                "line {number}: expected TIME enter NAME or TIME exit NAME"
            ));
        };
        if !is_time(time) || !is_name(name) {
            return Err(format!(
                "line {number}: expected TIME enter NAME or TIME exit NAME"
            ));
        }
        let time: u64 = time.parse().unwrap();
        if time < now {
            return Err(format!(
                "line {number}: time goes back from {now} to {time}"
            ));
        }
        now = time;

        if event == "enter" {
            let function = functions.entry(name).or_default();
            function.calls += 1;
            if function.running == 0 {
                function.outermost_start = time;
            }
            function.running += 1;
            stack.push(Call {
                name,
                start: time,
                in_callees: 0,
            });
            continue;
        }

        let Some(call) = stack.pop() else {
            return Err(format!(
                "line {number}: exit from {name}, but nothing is running"
            ));
        };
        if call.name != name {
            return Err(format!(
                "line {number}: exit from {name}, but the innermost call is to {}",
                call.name
            ));
        }
        let duration = time - call.start;
        let function = functions.get_mut(name).unwrap();
        function.self_time += duration - call.in_callees;
        function.running -= 1;
        // A recursive call's time is already in the total of the call around it.
        if function.running == 0 {
            function.total += time - function.outermost_start;
        }
        if let Some(caller) = stack.last_mut() {
            caller.in_callees += duration;
        }
    }
    if let Some(call) = stack.last() {
        return Err(format!("end of trace: {} is still running", call.name));
    }
    Ok(functions)
}

fn main() {
    let mut text = String::new();
    std::io::stdin().read_to_string(&mut text).unwrap();
    let functions = match profile(&text) {
        Ok(functions) => functions,
        Err(message) => {
            println!("error: {message}");
            return;
        }
    };
    if functions.is_empty() {
        println!("no calls");
    }
    let mut rows: Vec<_> = functions.into_iter().collect();
    rows.sort_by(|(a, f), (b, g)| g.self_time.cmp(&f.self_time).then(a.cmp(b)));
    for (name, f) in rows {
        println!("{name} {} {} {}", f.calls, f.total, f.self_time);
    }
}
