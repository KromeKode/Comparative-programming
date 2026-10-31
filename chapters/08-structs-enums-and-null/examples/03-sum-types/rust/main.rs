// Each variant of an enum can carry its own fields.
enum Payment {
    Cash,
    Card { last4: String },
    Voucher { code: String, cents: i64 },
}

fn money(cents: i64) -> String {
    format!("{}.{:02}", cents / 100, cents % 100)
}

fn describe(payment: &Payment, total: i64) -> String {
    // match must cover every variant. The `if` on the third arm is a guard.
    match payment {
        Payment::Cash => format!("cash: pay {} in cash", money(total)),
        Payment::Card { last4 } => {
            format!("card ending {last4}: charge {} to the card", money(total))
        }
        Payment::Voucher { code, cents } if *cents >= total => {
            format!("voucher {code} ({}): covers the order", money(*cents))
        }
        Payment::Voucher { code, cents } => {
            let left = money(total - cents);
            format!("voucher {code} ({}): {left} left to pay", money(*cents))
        }
    }
}

fn main() {
    let total = 1250;
    let payments = [
        Payment::Cash,
        Payment::Card {
            last4: "4242".to_string(),
        },
        Payment::Voucher {
            code: "SPRING".to_string(),
            cents: 500,
        },
        Payment::Voucher {
            code: "BIG".to_string(),
            cents: 2000,
        },
    ];
    println!("order total: {}", money(total));
    for payment in &payments {
        println!("{}", describe(payment, total));
    }
}
