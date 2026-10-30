fn describe(code: u16) -> &'static str {
    match code {
        200 => "ok",
        301 | 302 | 307 | 308 => "redirect",
        // Arms are tried from top to bottom, so 404 must come before 400..=499.
        404 => "not found",
        400..=499 => "client error",
        500..=599 => "server error",
        _ => "unknown", // a match must cover every possible value
    }
}

fn main() {
    for code in [200, 301, 308, 404, 418, 500, 503, 99] {
        println!("{code}: {}", describe(code));
    }
}
