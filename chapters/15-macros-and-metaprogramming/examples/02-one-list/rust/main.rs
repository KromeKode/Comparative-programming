// A macro that takes the whole list. $( ... );* matches the entries one by
// one, and each $( ... )* in the output repeats once for every entry.
macro_rules! statuses {
    ($($name:ident = $code:literal, $text:literal;)*) => {
        #[derive(Clone, Copy)]
        enum Status {
            $($name = $code,)*
        }

        impl Status {
            const ALL: &[Status] = &[$(Status::$name,)*];

            fn code(self) -> u16 {
                self as u16
            }

            fn name(self) -> &'static str {
                match self {
                    $(Status::$name => stringify!($name),)*
                }
            }

            fn text(self) -> &'static str {
                match self {
                    $(Status::$name => $text,)*
                }
            }

            fn from_code(code: u16) -> Option<Status> {
                match code {
                    $($code => Some(Status::$name),)*
                    _ => None,
                }
            }

            fn from_name(name: &str) -> Option<Status> {
                match name {
                    $(stringify!($name) => Some(Status::$name),)*
                    _ => None,
                }
            }
        }
    };
}

statuses! {
    Ok = 200, "OK";
    Moved = 301, "Moved Permanently";
    NotFound = 404, "Not Found";
    Teapot = 418, "I'm a teapot";
    ServerError = 500, "Internal Server Error";
}

fn main() {
    println!("{} statuses", Status::ALL.len());
    for status in Status::ALL {
        println!("{} {}: {}", status.code(), status.name(), status.text());
    }
    for code in [404, 999] {
        let text = Status::from_code(code).map_or("unknown", Status::text);
        println!("lookup {code}: {text}");
    }
    for name in ["Teapot", "Brewing"] {
        match Status::from_name(name) {
            Some(status) => println!("lookup {name}: {}", status.code()),
            None => println!("lookup {name}: unknown"),
        }
    }
}
