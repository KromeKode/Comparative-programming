const GRID: [[i32; 3]; 3] = [[3, 8, 1], [9, 4, 7], [2, 6, 5]];

fn main() {
    'targets: for target in [7, 10] {
        let mut checked = 0;
        for (r, row) in GRID.iter().enumerate() {
            for (c, &value) in row.iter().enumerate() {
                checked += 1;
                if value == target {
                    println!("{target}: row {r}, column {c} (checked {checked} cells)");
                    continue 'targets; // leaves both inner loops at once
                }
            }
        }
        println!("{target}: not found (checked {checked} cells)");
    }
}
