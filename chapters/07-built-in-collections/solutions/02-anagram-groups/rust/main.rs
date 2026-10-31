use std::collections::HashMap;
use std::io::{self, Read};

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("failed to read stdin");

    // A HashMap forgets the order keys arrived in, so it maps each key to a group
    // number, and the groups live in a Vec in the order they were created.
    let mut group_of: HashMap<String, usize> = HashMap::new();
    let mut groups: Vec<Vec<String>> = Vec::new();
    for word in input.split_whitespace() {
        let word = word.to_ascii_lowercase();
        let mut letters: Vec<char> = word.chars().collect();
        letters.sort_unstable(); // anagrams have the same letters once sorted
        let key: String = letters.into_iter().collect();
        let index = *group_of.entry(key).or_insert_with(|| {
            groups.push(Vec::new());
            groups.len() - 1
        });
        groups[index].push(word);
    }

    for group in &groups {
        println!("{}", group.join(" "));
    }
}
