use std::cmp::Reverse;
use std::collections::BTreeMap;
use std::io::{self, Read};

struct Grade<'a> {
    name: &'a str,
    subject: &'a str,
    score: u32,
}

// The standard library has no group_by for iterators, so this folds the grades
// into a map of lists. A BTreeMap keeps its keys sorted.
fn group_by<'a, 'g>(
    grades: &'g [Grade<'a>],
    key: impl Fn(&Grade<'a>) -> &'a str,
) -> BTreeMap<&'a str, Vec<&'g Grade<'a>>> {
    grades.iter().fold(BTreeMap::new(), |mut groups, grade| {
        groups
            .entry(key(grade))
            .or_insert_with(Vec::new)
            .push(grade);
        groups
    })
}

fn report(grades: &[Grade]) -> Vec<String> {
    let mut lines = vec!["subjects:".to_string()];
    for (subject, group) in group_by(grades, |g| g.subject) {
        let best = group.iter().map(|g| g.score).max().expect("a grade");
        let total: u32 = group.iter().map(|g| g.score).sum();
        let mut names: Vec<&str> = group
            .iter()
            .filter(|g| g.score == best)
            .map(|g| g.name)
            .collect();
        names.sort_unstable();
        let average = total / group.len() as u32;
        lines.push(format!(
            "  {subject}: average {average}, best {best} ({})",
            names.join(", ")
        ));
    }

    let students = group_by(grades, |g| g.name);
    let mut ranked: Vec<(&str, u32)> = students
        .iter()
        .map(|(&name, group)| (name, group.iter().map(|g| g.score).sum()))
        .collect();
    ranked.sort_by_key(|&(name, total)| (Reverse(total), name));
    lines.push("ranking:".to_string());
    lines.extend(
        ranked
            .iter()
            .enumerate()
            .map(|(i, (name, total))| format!("  {}. {name} {total}", i + 1)),
    );

    // The map's keys are sorted already, so the names come out in order.
    let passed: Vec<&str> = students
        .iter()
        .filter(|(_, group)| group.iter().all(|g| g.score >= 50))
        .map(|(&name, _)| name)
        .collect();
    let passed = if passed.is_empty() {
        "nobody".to_string()
    } else {
        passed.join(", ")
    };
    lines.push(format!("passed everything: {passed}"));
    lines
}

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("readable input");
    let grades: Vec<Grade> = input
        .lines()
        .map(|line| line.split_whitespace().collect::<Vec<_>>())
        .filter(|words| !words.is_empty())
        .map(|words| Grade {
            name: words[0],
            subject: words[1],
            score: words[2].parse().expect("a score"),
        })
        .collect();
    println!("{}", report(&grades).join("\n"));
}
