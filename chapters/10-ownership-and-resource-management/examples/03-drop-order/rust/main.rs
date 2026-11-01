// Prints when it is made and when it is dropped.
struct Noisy(&'static str);

fn make(name: &'static str) -> Noisy {
    println!("  make {name}");
    Noisy(name)
}

impl Drop for Noisy {
    fn drop(&mut self) {
        println!("  drop {}", self.0);
    }
}

// drop runs first. Then the fields are dropped, in the order they are
// declared in: the opposite of C++. (The underscores tell the compiler that
// nothing reads the fields.)
struct Pair {
    _first: Noisy,
    _second: Noisy,
}

impl Drop for Pair {
    fn drop(&mut self) {
        println!("  drop pair");
    }
}

fn main() {
    println!("locals go in the reverse order:");
    {
        let _a = make("a");
        let _b = make("b");
        let _c = make("c");
    }

    println!("an inner scope ends first:");
    {
        let _outer = make("outer");
        {
            let _inner = make("inner");
        }
        println!("  inner is gone, outer is not");
    }

    println!("a struct, then its fields:");
    {
        let _pair = Pair {
            _first: make("pair.first"),
            _second: make("pair.second"),
        };
    }

    println!("a temporary lasts until the end of its statement:");
    make("temporary");
    println!("  the temporary is gone");

    // `let _` binds nothing, so the value is a temporary too. `let _name`
    // binds a variable, which lives to the end of the scope.
    println!("let _ = drops at once, let _name = does not:");
    {
        let _ = make("unnamed");
        let _named = make("named");
        println!("  the scope ends");
    }

    println!("ending a lifetime early:");
    {
        let early = make("early");
        drop(early);
        println!("  early is gone before its scope ends");
    }
}
