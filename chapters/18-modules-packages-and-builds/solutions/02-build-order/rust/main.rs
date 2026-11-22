// Exercise 18.2: build order. The order to build modules in, and what to rebuild.
use std::cmp::Reverse;
use std::collections::{BTreeMap, BinaryHeap, HashMap, HashSet};
use std::io::{self, Read};

struct Module<'a> {
    line: usize,
    imports: Vec<&'a str>, // as written, with repeats dropped
}

fn is_name(text: &str) -> bool {
    !text.is_empty()
        && text
            .bytes()
            .all(|b| b.is_ascii_alphanumeric() || b"_./-".contains(&b))
}

fn run(input: &str) -> Vec<String> {
    let mut out = Vec::new();
    // A BTreeMap keeps the modules in order by name, and defined in the order they
    // were defined.
    let mut modules: BTreeMap<&str, Module> = BTreeMap::new();
    let mut defined: Vec<&str> = Vec::new();
    let mut queries: Vec<(usize, &str)> = Vec::new();
    for (number, line) in (1..).zip(input.lines()) {
        if let Some((name, rest)) = line.split_once(':') {
            let name = name.trim();
            let imports: Vec<&str> = rest.split_whitespace().collect();
            if !is_name(name) || !imports.iter().all(|import| is_name(import)) {
                out.push(format!("error: line {number}: a module is NAME: IMPORTS"));
                return out;
            }
            if let Some(earlier) = modules.get(name) {
                let earlier = earlier.line;
                out.push(format!(
                    "error: line {number}: {name} is already defined on line {earlier}"
                ));
                return out;
            }
            let mut unique = Vec::new();
            for import in imports {
                if !unique.contains(&import) {
                    unique.push(import);
                }
            }
            let module = Module {
                line: number,
                imports: unique,
            };
            modules.insert(name, module);
            defined.push(name);
            continue;
        }
        match line.split_whitespace().collect::<Vec<_>>()[..] {
            [] => {}
            ["rebuild", name] => queries.push((number, name)),
            _ => {
                out.push(format!(
                    "error: line {number}: expected NAME: IMPORTS or rebuild NAME"
                ));
                return out;
            }
        }
    }

    let mut users: HashMap<&str, Vec<&str>> = HashMap::new();
    let mut waiting: BTreeMap<&str, usize> = BTreeMap::new();
    for &name in &defined {
        let module = &modules[name];
        for &import in &module.imports {
            if !modules.contains_key(import) {
                let line = module.line;
                out.push(format!(
                    "error: line {line}: {name} imports {import}, which isn't defined"
                ));
                return out;
            }
            users.entry(import).or_default().push(name);
        }
        waiting.insert(name, module.imports.len());
    }

    // Kahn's algorithm: a module is ready once everything it imports is built, and
    // of the ready modules, the first in alphabetical order goes next. BinaryHeap
    // gives the largest first, so Reverse turns it around.
    let mut ready: BinaryHeap<Reverse<&str>> = waiting
        .iter()
        .filter(|&(_, &count)| count == 0)
        .map(|(&name, _)| Reverse(name))
        .collect();
    let mut order: Vec<&str> = Vec::new();
    while let Some(Reverse(name)) = ready.pop() {
        order.push(name);
        for &user in users.get(name).into_iter().flatten() {
            let count = waiting.get_mut(user).unwrap();
            *count -= 1;
            if *count == 0 {
                ready.push(Reverse(user));
            }
        }
    }

    if order.len() < modules.len() {
        // Every module left over imports another one left over, so following first
        // imports from the first module left over must come back to a module.
        let is_left = |name: &str| waiting[name] > 0;
        let start = *waiting.keys().find(|&&name| is_left(name)).unwrap();
        let mut path = vec![start];
        let mut seen = HashMap::from([(start, 0)]);
        loop {
            let last = path[path.len() - 1];
            let next = *modules[last]
                .imports
                .iter()
                .find(|&&import| is_left(import))
                .unwrap();
            if let Some(&at) = seen.get(next) {
                let cycle = path[at..].join(" -> ");
                out.push(format!("error: import cycle: {cycle} -> {next}"));
                return out;
            }
            seen.insert(next, path.len());
            path.push(next);
        }
    }

    let text = if order.is_empty() {
        "nothing to build".to_string()
    } else {
        order.join(", ")
    };
    out.push(format!("build order: {text}"));
    for (number, name) in queries {
        if !modules.contains_key(name) {
            out.push(format!("error: line {number}: no module called {name}"));
            continue;
        }
        // Everything that imports the changed module, directly or not, comes after
        // it in the build order, so one pass through the order finds them all.
        let mut affected = HashSet::from([name]);
        let mut rebuilt = vec![name];
        for &module in &order {
            if !affected.contains(module)
                && modules[module]
                    .imports
                    .iter()
                    .any(|import| affected.contains(import))
            {
                affected.insert(module);
                rebuilt.push(module);
            }
        }
        out.push(format!("rebuild {name}: {}", rebuilt.join(", ")));
    }
    out
}

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("can't read stdin");
    for line in run(&input) {
        println!("{line}");
    }
}
