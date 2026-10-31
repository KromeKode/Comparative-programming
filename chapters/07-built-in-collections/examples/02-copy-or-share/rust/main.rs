// &mut [i32] borrows the caller's items and may change them.
fn set_first(items: &mut [i32]) {
    items[0] = -1;
}

fn main() {
    let mut a = vec![1, 2, 3];

    // `let b = a;` would move the Vec into b, and using `a` afterwards would not
    // compile. To share it, borrow it.
    let b = &mut a;
    b[0] = 99;
    println!("b = a, then b[0] = 99: a is {a:?}");

    let mut c = a.clone(); // an explicit copy
    c[0] = 0;
    println!("c = copy of a, then c[0] = 0: a is {a:?}");

    set_first(&mut a);
    println!("set_first(a) sets a[0] = -1: a is {a:?}");

    let grid = vec![vec![0, 0], vec![0, 0]];
    let mut g = grid.clone(); // clone() copies every row too
    g[0][0] = 1;
    println!("g = copy of grid, then g[0][0] = 1: grid is {grid:?}");
}
