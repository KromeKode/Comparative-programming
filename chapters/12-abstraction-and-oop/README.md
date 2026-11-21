# Chapter 12: Abstraction and OOP

Once a program has more than a handful of types, two questions keep coming up. How do
you stop other code from putting a value into a broken state? And how do you write code
that works with many types without knowing which one it has? Object-oriented programming
answers both with classes, interfaces, and inheritance, and C++, Python, JavaScript,
TypeScript, Dart, and Kotlin are built around it. C, Rust, and Go are not, and each
answers the same questions with smaller parts: structs with functions, traits or
interfaces, and composition. This chapter puts all nine side by side on four tasks: a
type that guards its own data, many types behind one interface, reuse by inheritance and
by composition, and behavior shared between classes that are not related.

**Languages:** all 9 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Classes, constructors that check their input, and private fields, in nine forms | Every chapter from here on |
| Defining equality, ordering, and printing for your own types | Generics and polymorphism (chapter 13), sorting and searching (chapter 29) |
| Interfaces and traits, and how a call finds its method at run time | Generics and polymorphism (chapter 13), iterators (chapter 14), testing basics (chapter 19) |
| When inheritance helps, when it breaks, and how composition replaces it | Design problems (chapter 31), game engines and ECS (chapter 47) |
| Default methods and mixins: sharing code without a common base class | Iterators and functional style (chapter 14), GUI and desktop apps (chapter 49) |

---

## 12.1 The concept: hide the data, share the behavior

An object is data together with the operations on it. A fraction is two numbers, but not
any two numbers. The denominator can't be zero, and if every fraction is kept in lowest
terms, 1/2 and 2/4 are stored the same way, so comparing them is easy. Those rules are
the type's **invariants**. If any code can write to the fields, any code can break them.
So the type keeps its fields **private**, and offers a **constructor** that checks its
input and **methods** that keep the rules. That is **encapsulation**: code outside the
type sees what it does, not how.

The second idea is **polymorphism**: one piece of code that works with many types. A
function that writes out a report should not care whether the report is CSV, JSON, or
Markdown. It needs only "something with an export method". That something is an
**interface**, called a trait in Rust and a protocol in Python: a list of methods, with
no code behind them. Any type that has those methods can be passed in, and the call
finds the right code at run time. That is **dynamic dispatch**.

The third idea is **reuse**. There are two ways to build a new type from an old one:

- **Inheritance.** The new class *is* an old one, with changes. It gets all of the old
  class's fields and methods, and can **override** some of them. Code written for the
  base class works on the subclass too.
- **Composition.** The new type *has* an old one as a field, and passes work on to it.
  It chooses which methods to offer, and the old type never calls back into the new one.

```text
 Inheritance: a CountingBag is a Bag      Composition: a CountingBag has a Bag

   Bag                                      CountingBag
     add, add_all                             count
     ^                                        bag -----------> Bag
     | extends                                add, add_all       add, add_all
   CountingBag
     count
     add, add_all  (replacing Bag's)
```

Inheritance does two jobs at once. It shares code, and it lets a subclass be used
wherever the base class is. An interface does only the second job, and composition only
the first, and much of the difference between the nine languages is in how freely they
let you pick one job without the other. A fourth tool, the **mixin** or **default
method**, is a bundle of methods that classes with nothing else in common can all take
in.

---

## 12.2 What each language gives you

| Language | Types with methods | Privacy | Interfaces | Inheritance | Sharing code without inheritance |
| --- | --- | --- | --- | --- | --- |
| C | A `struct`, and functions that take a pointer to it | None in the language; a header can declare `struct thing;` and hide the fields | A struct of function pointers, by hand | None; put the shared struct first and cast | Functions that take the shared struct |
| C++ | `class` and `struct`, the same except that `class` members are private by default | `public`, `protected`, and `private`, checked by the compiler | Abstract classes, with pure virtual functions marked `= 0` | Single and multiple; only `virtual` functions can be overridden | Templates, including CRTP mixins |
| Rust | `struct` and `enum`, with `impl` blocks | Private to the module unless marked `pub` | Traits, used as `dyn Trait` at run time or as generics (chapter 13) | None; a supertrait only requires another trait | Default methods in traits; composition |
| Go | Any named type can have methods | Private to the package unless the name starts with a capital letter | Interfaces, satisfied without saying so | None | Embedding, which promotes the inner type's methods |
| Python | `class` | Convention: `_name` means private, and `__name` is renamed to `_Class__name` | `typing.Protocol`, checked by mypy, and `abc.ABC`, checked when an object is made | Single and multiple, searched in a fixed order (the MRO) | Mixin classes |
| JavaScript | `class`, built on prototypes | `#name` fields and methods, enforced at run time (ES2022) | None; any object with the right methods works | Single, with `extends` | Functions that return subclasses; `Object.assign` onto a prototype |
| TypeScript | As JavaScript, with types | `private` and `protected`, checked by the compiler only; or JavaScript's `#` | Structural `interface`s; `implements` is optional, and only checks | As JavaScript, plus `abstract` classes | Mixin functions, as in JavaScript |
| Dart | `class` | `_name` is private to the file (its library) | Every class is also an interface; an `abstract interface class` is only one | Single, with `extends` | `mixin`, added with `with`; `on` limits where it may go |
| Kotlin | `class`, `data class`, and `object` | `private`, `protected`, `internal` (the module), and `public`, the default | `interface`, with default methods and abstract properties | Single, from `open` classes only; members are final unless `open` | Default methods; delegation with `by` |

Read the Inheritance column first. Six languages have it, and Kotlin makes you ask for
it with `open`. C, Rust, and Go do without, and get by with interfaces and composition.
The Interfaces column is where all nine meet: each has a way to say "any type with these
methods", even if in C you build it by hand.

---

## 12.3 Classes: data that keeps its own rules

Here is a fraction in each language. Each one keeps the fraction in lowest terms with
the sign on the numerator, refuses a zero denominator, adds two fractions, and defines
equality, ordering, and printing. Chapter 3 promised this: how to teach `==` and `<`
about a type of your own.

**C**

```c
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// C has no private fields. A library would put only "struct fraction;" in its
// header, so other files could hold a pointer but not see inside. In one
// file, the rule is a comment: use the functions, not the fields.
struct fraction {
    long numerator;
    long denominator; // always positive, and in lowest terms
};

long gcd(long a, long b) {
    a = labs(a);
    while (b != 0) {
        long rest = a % b;
        a = b;
        b = rest;
    }
    return a;
}

// The "constructor": returns false instead of making a broken fraction.
bool fraction_make(long numerator, long denominator, struct fraction *out) {
    if (denominator == 0) {
        return false;
    }
    if (denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }
    long divisor = gcd(numerator, denominator);
    *out = (struct fraction){numerator / divisor, denominator / divisor};
    return true;
}

struct fraction fraction_add(struct fraction a, struct fraction b) {
    struct fraction sum;
    fraction_make(a.numerator * b.denominator + b.numerator * a.denominator,
                  a.denominator * b.denominator, &sum);
    return sum;
}

bool fraction_equal(struct fraction a, struct fraction b) {
    return a.numerator == b.numerator && a.denominator == b.denominator;
}

// Negative, zero, or positive, the shape that qsort wants.
int fraction_compare(const void *left, const void *right) {
    const struct fraction *a = left, *b = right;
    long x = a->numerator * b->denominator, y = b->numerator * a->denominator;
    return (x > y) - (x < y);
}

void fraction_print(struct fraction f) {
    if (f.denominator == 1) {
        printf("%ld", f.numerator);
    } else {
        printf("%ld/%ld", f.numerator, f.denominator);
    }
}

const char *yes_no(bool value) { return value ? "yes" : "no"; }

int main(void) {
    struct fraction half, third, other;
    fraction_make(1, 2, &half);
    fraction_make(1, 3, &third);

    printf("1/2 + 1/3 = ");
    fraction_print(fraction_add(half, third));
    printf("\n2/-4 is stored as ");
    fraction_make(2, -4, &other);
    fraction_print(other);
    printf("\n6/3 is stored as ");
    fraction_make(6, 3, &other);
    fraction_print(other);
    printf("\n");
    if (!fraction_make(1, 0, &other)) {
        printf("1/0: error: denominator is zero\n");
    }
    fraction_make(2, 4, &other);
    printf("1/2 == 2/4: %s\n", yes_no(fraction_equal(half, other)));
    printf("1/3 < 1/2: %s\n", yes_no(fraction_compare(&third, &half) < 0));

    struct fraction list[4];
    fraction_make(3, 4, &list[0]);
    list[1] = third;
    fraction_make(-1, 2, &list[2]);
    fraction_make(2, 4, &list[3]);
    qsort(list, 4, sizeof list[0], fraction_compare);
    printf("sorted: ");
    for (int i = 0; i < 4; i++) {
        printf("%s", i > 0 ? ", " : "");
        fraction_print(list[i]);
    }
    printf("\n");
    return 0;
}
```

**C++**

```cpp
#include <algorithm>
#include <compare>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>

// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction {
  public:
    Fraction(long numerator, long denominator) {
        if (denominator == 0) {
            throw std::invalid_argument("denominator is zero");
        }
        if (denominator < 0) {
            numerator = -numerator;
            denominator = -denominator;
        }
        long divisor = std::gcd(numerator, denominator);
        numerator_ = numerator / divisor;
        denominator_ = denominator / divisor;
    }

    Fraction operator+(const Fraction &other) const {
        return Fraction(numerator_ * other.denominator_ +
                            other.numerator_ * denominator_,
                        denominator_ * other.denominator_);
    }

    // Lowest terms make equal fractions look the same, so the default ==,
    // which compares the fields one by one, is right.
    bool operator==(const Fraction &) const = default;

    // The default <=> would compare the numerators first, which is wrong for
    // 1/3 and 1/2. This one compares a/b with c/d as a*d with c*b. The
    // compiler writes <, <=, >, and >= from it.
    std::strong_ordering operator<=>(const Fraction &other) const {
        return numerator_ * other.denominator_ <=>
               other.numerator_ * denominator_;
    }

    friend std::ostream &operator<<(std::ostream &out, const Fraction &f) {
        out << f.numerator_;
        if (f.denominator_ != 1) {
            out << '/' << f.denominator_;
        }
        return out;
    }

  private:
    long numerator_;
    long denominator_; // always positive
};

const char *yes_no(bool value) { return value ? "yes" : "no"; }

int main() {
    Fraction half(1, 2), third(1, 3);
    std::cout << "1/2 + 1/3 = " << half + third << '\n';
    std::cout << "2/-4 is stored as " << Fraction(2, -4) << '\n';
    std::cout << "6/3 is stored as " << Fraction(6, 3) << '\n';
    try {
        Fraction(1, 0);
    } catch (const std::invalid_argument &error) {
        std::cout << "1/0: error: " << error.what() << '\n';
    }
    std::cout << "1/2 == 2/4: " << yes_no(half == Fraction(2, 4)) << '\n';
    std::cout << "1/3 < 1/2: " << yes_no(third < half) << '\n';
    std::vector<Fraction> fractions{Fraction(3, 4), third, Fraction(-1, 2),
                                    Fraction(2, 4)};
    std::sort(fractions.begin(), fractions.end());
    std::cout << "sorted: ";
    for (std::size_t i = 0; i < fractions.size(); i++) {
        std::cout << (i > 0 ? ", " : "") << fractions[i];
    }
    std::cout << '\n';
}
```

**Rust**

```rust
// Privacy in Rust is per module: code outside `mod fraction` can't see the
// fields, so it can't make a fraction that is not in lowest terms.
mod fraction {
    use std::cmp::Ordering;
    use std::fmt;
    use std::ops::Add;

    // derive(PartialEq) compares the fields, which is right in lowest terms.
    #[derive(Clone, Copy, Debug, PartialEq, Eq)]
    pub struct Fraction {
        numerator: i64,
        denominator: i64, // always positive
    }

    fn gcd(a: i64, b: i64) -> i64 {
        if b == 0 { a.abs() } else { gcd(b, a % b) }
    }

    impl Fraction {
        pub fn new(numerator: i64, denominator: i64) -> Result<Fraction, String> {
            if denominator == 0 {
                return Err("denominator is zero".to_string());
            }
            let sign = denominator.signum();
            let divisor = gcd(numerator, denominator);
            Ok(Fraction {
                numerator: sign * numerator / divisor,
                denominator: sign * denominator / divisor,
            })
        }
    }

    impl Add for Fraction {
        type Output = Fraction;

        fn add(self, other: Fraction) -> Fraction {
            Fraction::new(
                self.numerator * other.denominator + other.numerator * self.denominator,
                self.denominator * other.denominator,
            )
            .expect("the product of two denominators is never zero")
        }
    }

    // derive(PartialOrd) would compare the numerators first, which is wrong
    // for 1/3 and 1/2, so Ord is written by hand.
    impl Ord for Fraction {
        fn cmp(&self, other: &Fraction) -> Ordering {
            (self.numerator * other.denominator).cmp(&(other.numerator * self.denominator))
        }
    }

    impl PartialOrd for Fraction {
        fn partial_cmp(&self, other: &Fraction) -> Option<Ordering> {
            Some(self.cmp(other))
        }
    }

    impl fmt::Display for Fraction {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            if self.denominator == 1 {
                write!(f, "{}", self.numerator)
            } else {
                write!(f, "{}/{}", self.numerator, self.denominator)
            }
        }
    }
}

use fraction::Fraction;

fn yes_no(value: bool) -> &'static str {
    if value { "yes" } else { "no" }
}

fn main() -> Result<(), String> {
    let (half, third) = (Fraction::new(1, 2)?, Fraction::new(1, 3)?);
    println!("1/2 + 1/3 = {}", half + third);
    println!("2/-4 is stored as {}", Fraction::new(2, -4)?);
    println!("6/3 is stored as {}", Fraction::new(6, 3)?);
    if let Err(error) = Fraction::new(1, 0) {
        println!("1/0: error: {error}");
    }
    println!("1/2 == 2/4: {}", yes_no(half == Fraction::new(2, 4)?));
    println!("1/3 < 1/2: {}", yes_no(third < half));
    let mut fractions = vec![
        Fraction::new(3, 4)?,
        third,
        Fraction::new(-1, 2)?,
        Fraction::new(2, 4)?,
    ];
    fractions.sort();
    let text: Vec<String> = fractions.iter().map(|f| f.to_string()).collect();
    println!("sorted: {}", text.join(", "));
    Ok(())
}
```

**Go**

```go
package main

import (
	"errors"
	"fmt"
	"slices"
	"strings"
)

// Fraction is kept in lowest terms, with the sign on the numerator. Its fields
// start with a lower-case letter, so code outside this package can't see
// them. This example is all one package, so here that is only a promise.
type Fraction struct {
	numerator   int
	denominator int // always positive
}

func gcd(a, b int) int {
	for b != 0 {
		a, b = b, a%b
	}
	return max(a, -a)
}

// NewFraction is the constructor: Go has none built in, only this convention.
func NewFraction(numerator, denominator int) (Fraction, error) {
	if denominator == 0 {
		return Fraction{}, errors.New("denominator is zero")
	}
	if denominator < 0 {
		numerator, denominator = -numerator, -denominator
	}
	divisor := gcd(numerator, denominator)
	return Fraction{numerator / divisor, denominator / divisor}, nil
}

// mustFraction is for values the program knows are fine.
func mustFraction(numerator, denominator int) Fraction {
	f, err := NewFraction(numerator, denominator)
	if err != nil {
		panic(err)
	}
	return f
}

func (f Fraction) Add(other Fraction) Fraction {
	return mustFraction(f.numerator*other.denominator+other.numerator*f.denominator,
		f.denominator*other.denominator)
}

// Compare returns a negative number, zero, or a positive number, the shape
// that slices.SortFunc wants. Go has no operator overloading, so < can't be
// used on fractions.
func (f Fraction) Compare(other Fraction) int {
	return f.numerator*other.denominator - other.numerator*f.denominator
}

// String is what fmt prints for a Fraction.
func (f Fraction) String() string {
	if f.denominator == 1 {
		return fmt.Sprint(f.numerator)
	}
	return fmt.Sprintf("%d/%d", f.numerator, f.denominator)
}

func yesNo(value bool) string {
	if value {
		return "yes"
	}
	return "no"
}

func main() {
	half, third := mustFraction(1, 2), mustFraction(1, 3)
	fmt.Println("1/2 + 1/3 =", half.Add(third))
	fmt.Println("2/-4 is stored as", mustFraction(2, -4))
	fmt.Println("6/3 is stored as", mustFraction(6, 3))
	if _, err := NewFraction(1, 0); err != nil {
		fmt.Println("1/0: error:", err)
	}
	// == compares structs field by field, which is right in lowest terms.
	fmt.Println("1/2 == 2/4:", yesNo(half == mustFraction(2, 4)))
	fmt.Println("1/3 < 1/2:", yesNo(third.Compare(half) < 0))
	fractions := []Fraction{mustFraction(3, 4), third, mustFraction(-1, 2), mustFraction(2, 4)}
	// Fraction.Compare is a method used as a function of two fractions.
	slices.SortFunc(fractions, Fraction.Compare)
	text := make([]string, len(fractions))
	for i, f := range fractions {
		text[i] = f.String()
	}
	fmt.Println("sorted:", strings.Join(text, ", "))
}
```

**Python**

```python
from functools import total_ordering
from math import gcd


@total_ordering
class Fraction:
    """A fraction kept in lowest terms, with the sign on the numerator."""

    __slots__ = ("_numerator", "_denominator")

    def __init__(self, numerator: int, denominator: int):
        if denominator == 0:
            raise ValueError("denominator is zero")
        if denominator < 0:
            numerator, denominator = -numerator, -denominator
        divisor = gcd(numerator, denominator)
        # A leading underscore means "private" by convention only.
        self._numerator = numerator // divisor
        self._denominator = denominator // divisor

    def _pair(self) -> tuple[int, int]:
        return (self._numerator, self._denominator)

    def __add__(self, other: "Fraction") -> "Fraction":
        a, b = self._pair()
        c, d = other._pair()
        return Fraction(a * d + c * b, b * d)

    # Lowest terms make equal fractions look the same, so == can compare fields.
    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Fraction):
            return NotImplemented
        return self._pair() == other._pair()

    # Objects that are equal must have equal hashes.
    def __hash__(self) -> int:
        return hash(self._pair())

    # total_ordering writes <=, >, and >= from __lt__ and __eq__.
    def __lt__(self, other: "Fraction") -> bool:
        a, b = self._pair()
        c, d = other._pair()
        return a * d < c * b

    def __str__(self) -> str:
        if self._denominator == 1:
            return str(self._numerator)
        return f"{self._numerator}/{self._denominator}"


def yes_no(value: bool) -> str:
    return "yes" if value else "no"


half, third = Fraction(1, 2), Fraction(1, 3)
print(f"1/2 + 1/3 = {half + third}")
print(f"2/-4 is stored as {Fraction(2, -4)}")
print(f"6/3 is stored as {Fraction(6, 3)}")
try:
    Fraction(1, 0)
except ValueError as error:
    print(f"1/0: error: {error}")
print(f"1/2 == 2/4: {yes_no(half == Fraction(2, 4))}")
print(f"1/3 < 1/2: {yes_no(third < half)}")
fractions = [Fraction(3, 4), third, Fraction(-1, 2), Fraction(2, 4)]
print("sorted:", ", ".join(str(f) for f in sorted(fractions)))
```

**JavaScript**

```js
function gcd(a, b) {
  return b === 0 ? Math.abs(a) : gcd(b, a % b);
}

// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction {
  // # makes a field private. Code outside the class can't read or write it,
  // and trying to is a SyntaxError.
  #numerator;
  #denominator;

  constructor(numerator, denominator) {
    if (denominator === 0) {
      throw new RangeError("denominator is zero");
    }
    const sign = Math.sign(denominator);
    const divisor = gcd(numerator, denominator);
    this.#numerator = (sign * numerator) / divisor;
    this.#denominator = (sign * denominator) / divisor;
  }

  plus(other) {
    return new Fraction(
      this.#numerator * other.#denominator +
        other.#numerator * this.#denominator,
      this.#denominator * other.#denominator,
    );
  }

  // JavaScript has no operator overloading, so == and < can't be taught
  // about fractions. These methods do the work instead.
  equals(other) {
    return (
      this.#numerator === other.#numerator &&
      this.#denominator === other.#denominator
    );
  }

  compare(other) {
    return (
      this.#numerator * other.#denominator -
      other.#numerator * this.#denominator
    );
  }

  // Template strings and String() call toString.
  toString() {
    return this.#denominator === 1
      ? `${this.#numerator}`
      : `${this.#numerator}/${this.#denominator}`;
  }
}

const yesNo = (value) => (value ? "yes" : "no");

const [half, third] = [new Fraction(1, 2), new Fraction(1, 3)];
console.log(`1/2 + 1/3 = ${half.plus(third)}`);
console.log(`2/-4 is stored as ${new Fraction(2, -4)}`);
console.log(`6/3 is stored as ${new Fraction(6, 3)}`);
try {
  new Fraction(1, 0);
} catch (error) {
  console.log(`1/0: error: ${error.message}`);
}
console.log(`1/2 == 2/4: ${yesNo(half.equals(new Fraction(2, 4)))}`);
console.log(`1/3 < 1/2: ${yesNo(third.compare(half) < 0)}`);
const fractions = [
  new Fraction(3, 4),
  third,
  new Fraction(-1, 2),
  new Fraction(2, 4),
];
fractions.sort((a, b) => a.compare(b));
console.log(`sorted: ${fractions.join(", ")}`);
```

**TypeScript**

```ts
function gcd(a: number, b: number): number {
  return b === 0 ? Math.abs(a) : gcd(b, a % b);
}

// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction {
  // private is checked by the compiler only. JavaScript's # fields are
  // private at run time too; TypeScript accepts both.
  private readonly numerator: number;
  private readonly denominator: number;

  constructor(numerator: number, denominator: number) {
    if (denominator === 0) {
      throw new RangeError("denominator is zero");
    }
    const sign = Math.sign(denominator);
    const divisor = gcd(numerator, denominator);
    this.numerator = (sign * numerator) / divisor;
    this.denominator = (sign * denominator) / divisor;
  }

  plus(other: Fraction): Fraction {
    return new Fraction(
      this.numerator * other.denominator + other.numerator * this.denominator,
      this.denominator * other.denominator,
    );
  }

  // No operator overloading, so equality and ordering are methods.
  equals(other: Fraction): boolean {
    return (
      this.numerator === other.numerator &&
      this.denominator === other.denominator
    );
  }

  compare(other: Fraction): number {
    return (
      this.numerator * other.denominator - other.numerator * this.denominator
    );
  }

  toString(): string {
    return this.denominator === 1
      ? `${this.numerator}`
      : `${this.numerator}/${this.denominator}`;
  }
}

const yesNo = (value: boolean): string => (value ? "yes" : "no");

const [half, third] = [new Fraction(1, 2), new Fraction(1, 3)];
console.log(`1/2 + 1/3 = ${half.plus(third)}`);
console.log(`2/-4 is stored as ${new Fraction(2, -4)}`);
console.log(`6/3 is stored as ${new Fraction(6, 3)}`);
try {
  new Fraction(1, 0);
} catch (error) {
  if (error instanceof RangeError) {
    console.log(`1/0: error: ${error.message}`);
  }
}
console.log(`1/2 == 2/4: ${yesNo(half.equals(new Fraction(2, 4)))}`);
console.log(`1/3 < 1/2: ${yesNo(third.compare(half) < 0)}`);
const fractions = [
  new Fraction(3, 4),
  third,
  new Fraction(-1, 2),
  new Fraction(2, 4),
];
fractions.sort((a, b) => a.compare(b));
console.log(`sorted: ${fractions.join(", ")}`);
```

**Dart**

```dart
// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction implements Comparable<Fraction> {
  // A leading _ makes a name private to this file (its library).
  final int _numerator;
  final int _denominator; // always positive

  // A factory constructor can do work before it makes the object.
  factory Fraction(int numerator, int denominator) {
    if (denominator == 0) {
      throw ArgumentError('denominator is zero');
    }
    final sign = denominator.sign;
    final divisor = numerator.gcd(denominator);
    return Fraction._(
      sign * numerator ~/ divisor,
      sign * denominator ~/ divisor,
    );
  }

  // The real constructor is private, so every fraction goes through the
  // factory above.
  Fraction._(this._numerator, this._denominator);

  Fraction operator +(Fraction other) => Fraction(
    _numerator * other._denominator + other._numerator * _denominator,
    _denominator * other._denominator,
  );

  // == on objects compares identity unless a class overrides it, and a class
  // that overrides == must override hashCode to match.
  @override
  bool operator ==(Object other) =>
      other is Fraction &&
      _numerator == other._numerator &&
      _denominator == other._denominator;

  @override
  int get hashCode => Object.hash(_numerator, _denominator);

  // compareTo is what List.sort uses; < is defined from it.
  @override
  int compareTo(Fraction other) =>
      _numerator * other._denominator - other._numerator * _denominator;

  bool operator <(Fraction other) => compareTo(other) < 0;

  @override
  String toString() =>
      _denominator == 1 ? '$_numerator' : '$_numerator/$_denominator';
}

String yesNo(bool value) => value ? 'yes' : 'no';

void main() {
  final half = Fraction(1, 2), third = Fraction(1, 3);
  print('1/2 + 1/3 = ${half + third}');
  print('2/-4 is stored as ${Fraction(2, -4)}');
  print('6/3 is stored as ${Fraction(6, 3)}');
  try {
    Fraction(1, 0);
  } on ArgumentError catch (error) {
    // Only to show the check: real code would not pass 0 and catch the Error.
    print('1/0: error: ${error.message}');
  }
  print('1/2 == 2/4: ${yesNo(half == Fraction(2, 4))}');
  print('1/3 < 1/2: ${yesNo(third < half)}');
  final fractions = [Fraction(3, 4), third, Fraction(-1, 2), Fraction(2, 4)];
  fractions.sort();
  print('sorted: ${fractions.join(', ')}');
}
```

**Kotlin**

```kotlin
// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction(numerator: Long, denominator: Long) : Comparable<Fraction> {
    // private val: readable inside the class only, and never reassigned.
    private val numerator: Long
    private val denominator: Long // always positive

    init {
        require(denominator != 0L) { "denominator is zero" }
        val sign = if (denominator < 0) -1 else 1
        val divisor = gcd(numerator, denominator)
        this.numerator = sign * numerator / divisor
        this.denominator = sign * denominator / divisor
    }

    operator fun plus(other: Fraction) =
        Fraction(
            numerator * other.denominator + other.numerator * denominator,
            denominator * other.denominator,
        )

    // == calls equals, which compares identity unless a class overrides it.
    // A class that overrides equals must override hashCode to match.
    override fun equals(other: Any?) =
        other is Fraction && numerator == other.numerator && denominator == other.denominator

    override fun hashCode() = 31 * numerator.hashCode() + denominator.hashCode()

    // <, >, sorted, and maxOf all use compareTo.
    override fun compareTo(other: Fraction) =
        (numerator * other.denominator).compareTo(other.numerator * denominator)

    override fun toString() = if (denominator == 1L) "$numerator" else "$numerator/$denominator"

    private companion object {
        tailrec fun gcd(a: Long, b: Long): Long = if (b == 0L) Math.abs(a) else gcd(b, a % b)
    }
}

fun yesNo(value: Boolean) = if (value) "yes" else "no"

fun main() {
    val half = Fraction(1, 2)
    val third = Fraction(1, 3)
    println("1/2 + 1/3 = ${half + third}")
    println("2/-4 is stored as ${Fraction(2, -4)}")
    println("6/3 is stored as ${Fraction(6, 3)}")
    try {
        Fraction(1, 0)
    } catch (e: IllegalArgumentException) {
        println("1/0: error: ${e.message}")
    }
    println("1/2 == 2/4: ${yesNo(half == Fraction(2, 4))}")
    println("1/3 < 1/2: ${yesNo(third < half)}")
    val fractions = listOf(Fraction(3, 4), third, Fraction(-1, 2), Fraction(2, 4))
    println("sorted: ${fractions.sorted().joinToString(", ")}")
}
```

All nine print the same lines:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
1/2 + 1/3 = 5/6
2/-4 is stored as -1/2
6/3 is stored as 2
1/0: error: denominator is zero
1/2 == 2/4: yes
1/3 < 1/2: yes
sorted: -1/2, 1/3, 1/2, 3/4
```

**C: a struct and a naming convention.** C has no classes. A fraction is a `struct`, and
its "methods" are functions whose names start with `fraction_`. Nothing stops other code
from writing `half.denominator = 0`. A library hides the fields by putting only
`struct fraction;` in its header, so other files can hold a `struct fraction *` but
can't look inside it and must call the library's functions. In one file, the rule is
only a comment. `fraction_make` is the constructor, and it returns `false` instead of
making a broken fraction. There is no operator overloading, so equality is
`fraction_equal`, and ordering is `fraction_compare`, which returns a negative number,
zero, or a positive number, the shape that `qsort` wants.

**C++: private members, and operators the compiler can write.** The constructor throws
`std::invalid_argument` for a zero denominator, so no `Fraction` ever holds one. The
fields are `private`, and `half.numerator_` outside the class does not compile:
`'long int Fraction::numerator_' is private within this context`. `operator== = default`
compares the fields one by one, which is right because equal fractions are stored the
same way. C++20's three-way comparison, `<=>`, gives `<`, `<=`, `>`, and `>=` from one
function. The default one would compare the numerators first, and then `1/3 < 1/2` would
print `no` and the sorted list would be `-1/2, 1/2, 1/3, 3/4`. So this one compares a/b
with c/d as a×d with c×b. `operator<<` is a `friend`, which lets it read the private
fields.

**Rust: privacy is per module.** Rust has no classes. A `struct` holds the data, and
`impl` blocks add methods. A field is private to its module unless it is marked `pub`,
so the fraction lives in `mod fraction`, and `main` gets
`error[E0616]: field 'numerator' of struct 'Fraction' is private` if it reads one. There
are no constructors either: `new` is an ordinary function, and it returns
`Result<Fraction, String>`. Operators are traits. `#[derive(PartialEq, Eq)]` writes `==`
from the fields, `Add` gives `+`, `Display` gives `{}` in `println!`, and `Ord` with
`PartialOrd` gives `<` and `sort`. A derived `Ord` would compare the fields in order,
with the same wrong answer as C++'s default `<=>`, so `cmp` is written by hand.

**Go: lower-case names stay in the package.** Go's privacy is per package: a name that
starts with a lower-case letter can't be used from another package. This example is all
in package `main`, so `numerator` is private only by promise. Go has no constructors,
and `NewFraction` is the convention: a function that returns the value and an `error`.
`mustFraction` panics instead, for values the program knows are fine. Go has no operator
overloading, but `==` already compares structs field by field. Ordering is a `Compare`
method, and `slices.SortFunc(fractions, Fraction.Compare)` passes the method as a
function of two fractions. A `String` method is what `fmt` prints.

**Python: special methods, and privacy by convention.** Python's operators call methods
with double underscores: `+` calls `__add__`, `==` calls `__eq__`, `<` calls `__lt__`,
and `str` and f-strings call `__str__`. `@total_ordering` writes `<=`, `>`, and `>=`
from `__lt__` and `__eq__`. A class that defines `__eq__` must define `__hash__` too, or
Python sets `__hash__` to `None`, and the object can't go in a `set` or be a `dict` key:
`TypeError: unhashable type: 'Fraction'`. The leading `_` on `_numerator` asks other
code to keep out, and nothing enforces it. `__slots__` lists the attributes an object
may have. It saves memory, and it turns a typo such as `self._numrator = 1` into an
`AttributeError`.

**JavaScript: `#` fields, and no operator overloading.** A field whose name starts with
`#` is private, and the language enforces it: `half.#numerator` outside the class is a
`SyntaxError` before the program starts. Privacy is per class, not per object, so `plus`
can read `other.#denominator`, as C++, TypeScript, Dart, and Kotlin methods can read
another object's private fields. JavaScript has no operator overloading, so the methods
are `plus`, `equals`, and `compare`. Worse, the operators still work on objects, and
give the wrong answer. `half == new Fraction(1, 2)` is `false`, because `==` compares
identity, and `third < half` is `false` too, because `<` turns both objects into strings
with `toString` and compares the text `"1/3"` with `"1/2"`. Template literals and `join`
call `toString` as well, which is the useful part.

**TypeScript: `private` is a compile-time check.** `private readonly numerator` makes
the compiler refuse `half.numerator` with error TS2341:
`Property 'numerator' is private and only accessible within class 'Fraction'.` The check
is erased with the types, so at run time the field is an ordinary property, and
`JSON.stringify(half)` gives `{"numerator":1,"denominator":2}`. For privacy at run time,
TypeScript accepts JavaScript's `#` fields too. TypeScript also accepts `third < half`
without a word, and it gives JavaScript's answer.

**Dart: a factory constructor, and `_` for the library.** A name that starts with `_` is
private to its library, which is usually one file. The class has a private constructor,
`Fraction._`, and a public `factory` constructor that checks and reduces the numbers
before it calls the private one, so every fraction is in lowest terms. `operator ==` and
`hashCode` are overridden together. `implements Comparable<Fraction>` requires
`compareTo`, which `List.sort` uses, and `operator <` is written in terms of it. A zero
denominator throws an `ArgumentError`, which Dart counts as a bug in the caller (chapter
11), so the example catches it only to show the message.

**Kotlin: `init`, `require`, and operator functions.** The constructor's parameters have
no `val`, so they are not properties. The `init` block checks them with `require`, which
throws `IllegalArgumentException`, and sets the two private properties.
`operator fun plus` gives `+`, `Comparable<Fraction>` gives `<` and `sorted`, and
`equals` with `hashCode` gives `==`. The `gcd` helper lives in a
`private companion object`, Kotlin's place for functions that belong to the class rather
than to one object. A `data class` would write `equals`, `hashCode`, and `toString` for
you, but its properties are exactly what the caller passed in, so it could check a
fraction but not reduce it.

---

## 12.4 Interfaces: one call, many types

The program below writes one table of scores in four formats. `run` takes an exporter
and calls its `name` and `export` methods, and never asks which kind of exporter it has.
A fifth format would be one new type, and `run` would not change.

**C**

```c
#include <stdio.h>

struct row {
    const char *name;
    int score;
};

// An interface in C is a struct of function pointers, often called a vtable.
// Every exporter starts with a pointer to one, and each kind of exporter
// fills in its own functions.
struct exporter;
struct exporter_vtable {
    const char *(*name)(const struct exporter *self);
    void (*export)(const struct exporter *self, const struct row *rows,
                   int count);
};

struct exporter {
    const struct exporter_vtable *vtable;
};

// A CSV exporter is an exporter plus its own data. Because base comes first,
// a pointer to a csv_exporter can be used as a pointer to an exporter.
struct csv_exporter {
    struct exporter base;
    char separator;
};

const char *csv_name(const struct exporter *self) {
    (void)self;
    return "csv";
}

void csv_export(const struct exporter *self, const struct row *rows,
                int count) {
    const struct csv_exporter *csv = (const struct csv_exporter *)self;
    printf("name%cscore\n", csv->separator);
    for (int i = 0; i < count; i++) {
        printf("%s%c%d\n", rows[i].name, csv->separator, rows[i].score);
    }
}

const char *json_name(const struct exporter *self) {
    (void)self;
    return "json";
}

void json_export(const struct exporter *self, const struct row *rows,
                 int count) {
    (void)self;
    printf("[");
    for (int i = 0; i < count; i++) {
        printf("%s{\"name\":\"%s\",\"score\":%d}", i > 0 ? "," : "",
               rows[i].name, rows[i].score);
    }
    printf("]\n");
}

const char *markdown_name(const struct exporter *self) {
    (void)self;
    return "markdown";
}

void markdown_export(const struct exporter *self, const struct row *rows,
                     int count) {
    (void)self;
    printf("| name | score |\n| --- | --- |\n");
    for (int i = 0; i < count; i++) {
        printf("| %s | %d |\n", rows[i].name, rows[i].score);
    }
}

const char *summary_name(const struct exporter *self) {
    (void)self;
    return "summary";
}

void summary_export(const struct exporter *self, const struct row *rows,
                    int count) {
    (void)self;
    int best = 0;
    for (int i = 1; i < count; i++) {
        if (rows[i].score > rows[best].score) {
            best = i;
        }
    }
    printf("%d rows, top score %d by %s\n", count, rows[best].score,
           rows[best].name);
}

// One vtable per kind, shared by every exporter of that kind.
static const struct exporter_vtable csv_vtable = {csv_name, csv_export};
static const struct exporter_vtable json_vtable = {json_name, json_export};
static const struct exporter_vtable markdown_vtable = {markdown_name,
                                                       markdown_export};
static const struct exporter_vtable summary_vtable = {summary_name,
                                                      summary_export};

// This function knows only the interface, not the kinds of exporter.
void run(const struct exporter *exporter, const struct row *rows, int count) {
    printf("== %s\n", exporter->vtable->name(exporter));
    exporter->vtable->export(exporter, rows, count);
}

int main(void) {
    struct row rows[] = {{"ada", 90}, {"grace", 85}, {"alan", 78}};
    struct csv_exporter csv = {{&csv_vtable}, ','};
    struct exporter json = {&json_vtable};
    struct exporter markdown = {&markdown_vtable};
    struct exporter summary = {&summary_vtable};
    const struct exporter *exporters[] = {&csv.base, &json, &markdown,
                                          &summary};
    for (int i = 0; i < 4; i++) {
        run(exporters[i], rows, 3);
    }
    return 0;
}
```

**C++**

```cpp
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

struct Row {
    std::string name;
    int score;
};

// An abstract class: = 0 marks a function that every subclass must define,
// and no Exporter can be made on its own.
class Exporter {
  public:
    virtual ~Exporter() = default;
    virtual std::string name() const = 0;
    virtual std::string export_rows(const std::vector<Row> &rows) const = 0;
};

class CsvExporter : public Exporter {
  public:
    explicit CsvExporter(char separator) : separator_(separator) {}

    std::string name() const override { return "csv"; }

    std::string export_rows(const std::vector<Row> &rows) const override {
        std::ostringstream out;
        out << "name" << separator_ << "score";
        for (const Row &row : rows) {
            out << '\n' << row.name << separator_ << row.score;
        }
        return out.str();
    }

  private:
    char separator_;
};

class JsonExporter : public Exporter {
  public:
    std::string name() const override { return "json"; }

    std::string export_rows(const std::vector<Row> &rows) const override {
        std::ostringstream out;
        out << '[';
        for (std::size_t i = 0; i < rows.size(); i++) {
            out << (i > 0 ? "," : "") << "{\"name\":\"" << rows[i].name
                << "\",\"score\":" << rows[i].score << '}';
        }
        out << ']';
        return out.str();
    }
};

class MarkdownExporter : public Exporter {
  public:
    std::string name() const override { return "markdown"; }

    std::string export_rows(const std::vector<Row> &rows) const override {
        std::ostringstream out;
        out << "| name | score |\n| --- | --- |";
        for (const Row &row : rows) {
            out << "\n| " << row.name << " | " << row.score << " |";
        }
        return out.str();
    }
};

class SummaryExporter : public Exporter {
  public:
    std::string name() const override { return "summary"; }

    std::string export_rows(const std::vector<Row> &rows) const override {
        auto best = std::max_element(
            rows.begin(), rows.end(),
            [](const Row &a, const Row &b) { return a.score < b.score; });
        return std::to_string(rows.size()) + " rows, top score " +
               std::to_string(best->score) + " by " + best->name;
    }
};

// This function knows only Exporter. The virtual call picks the subclass's
// function at run time.
void run(const Exporter &exporter, const std::vector<Row> &rows) {
    std::cout << "== " << exporter.name() << '\n'
              << exporter.export_rows(rows) << '\n';
}

int main() {
    std::vector<Row> rows{{"ada", 90}, {"grace", 85}, {"alan", 78}};
    std::vector<std::unique_ptr<Exporter>> exporters;
    exporters.push_back(std::make_unique<CsvExporter>(','));
    exporters.push_back(std::make_unique<JsonExporter>());
    exporters.push_back(std::make_unique<MarkdownExporter>());
    exporters.push_back(std::make_unique<SummaryExporter>());
    for (const auto &exporter : exporters) {
        run(*exporter, rows);
    }
}
```

**Rust**

```rust
// A row is a name and a score.
type Row = (&'static str, u32);

// A trait lists the methods a type must have. A type says so with impl.
trait Exporter {
    fn name(&self) -> &str;
    fn export(&self, rows: &[Row]) -> String;
}

struct CsvExporter {
    separator: char,
}

impl Exporter for CsvExporter {
    fn name(&self) -> &str {
        "csv"
    }

    fn export(&self, rows: &[Row]) -> String {
        let sep = self.separator;
        let mut lines = vec![format!("name{sep}score")];
        for (name, score) in rows {
            lines.push(format!("{name}{sep}{score}"));
        }
        lines.join("\n")
    }
}

struct JsonExporter;

impl Exporter for JsonExporter {
    fn name(&self) -> &str {
        "json"
    }

    fn export(&self, rows: &[Row]) -> String {
        let mut items = Vec::new();
        for (name, score) in rows {
            items.push(format!("{{\"name\":\"{name}\",\"score\":{score}}}"));
        }
        format!("[{}]", items.join(","))
    }
}

struct MarkdownExporter;

impl Exporter for MarkdownExporter {
    fn name(&self) -> &str {
        "markdown"
    }

    fn export(&self, rows: &[Row]) -> String {
        let mut lines = vec!["| name | score |".to_string(), "| --- | --- |".to_string()];
        for (name, score) in rows {
            lines.push(format!("| {name} | {score} |"));
        }
        lines.join("\n")
    }
}

struct SummaryExporter;

impl Exporter for SummaryExporter {
    fn name(&self) -> &str {
        "summary"
    }

    fn export(&self, rows: &[Row]) -> String {
        let (name, score) = rows.iter().max_by_key(|row| row.1).expect("some rows");
        format!("{} rows, top score {score} by {name}", rows.len())
    }
}

// &dyn Exporter is a pointer to some Exporter, plus a pointer to that type's
// table of methods, so the call is chosen at run time.
fn run(exporter: &dyn Exporter, rows: &[Row]) {
    println!("== {}", exporter.name());
    println!("{}", exporter.export(rows));
}

fn main() {
    let rows = [("ada", 90), ("grace", 85), ("alan", 78)];
    // Each Box holds a different type, so the Vec holds trait objects.
    let exporters: Vec<Box<dyn Exporter>> = vec![
        Box::new(CsvExporter { separator: ',' }),
        Box::new(JsonExporter),
        Box::new(MarkdownExporter),
        Box::new(SummaryExporter),
    ];
    for exporter in &exporters {
        run(exporter.as_ref(), &rows);
    }
}
```

**Go**

```go
package main

import (
	"fmt"
	"strings"
)

type Row struct {
	Name  string
	Score int
}

// Exporter is an interface. A type satisfies it by having these methods; it
// never says "implements".
type Exporter interface {
	Name() string
	Export(rows []Row) string
}

type CSV struct{ Separator string }

func (c CSV) Name() string { return "csv" }

func (c CSV) Export(rows []Row) string {
	lines := []string{"name" + c.Separator + "score"}
	for _, row := range rows {
		lines = append(lines, fmt.Sprintf("%s%s%d", row.Name, c.Separator, row.Score))
	}
	return strings.Join(lines, "\n")
}

type JSON struct{}

func (JSON) Name() string { return "json" }

func (JSON) Export(rows []Row) string {
	items := make([]string, len(rows))
	for i, row := range rows {
		items[i] = fmt.Sprintf(`{"name":"%s","score":%d}`, row.Name, row.Score)
	}
	return "[" + strings.Join(items, ",") + "]"
}

type Markdown struct{}

func (Markdown) Name() string { return "markdown" }

func (Markdown) Export(rows []Row) string {
	lines := []string{"| name | score |", "| --- | --- |"}
	for _, row := range rows {
		lines = append(lines, fmt.Sprintf("| %s | %d |", row.Name, row.Score))
	}
	return strings.Join(lines, "\n")
}

type Summary struct{}

func (Summary) Name() string { return "summary" }

func (Summary) Export(rows []Row) string {
	best := rows[0]
	for _, row := range rows[1:] {
		if row.Score > best.Score {
			best = row
		}
	}
	return fmt.Sprintf("%d rows, top score %d by %s", len(rows), best.Score, best.Name)
}

// This line fails to compile if CSV ever stops satisfying Exporter.
var _ Exporter = CSV{}

// run knows only the interface. An interface value holds the concrete value
// and its type, and the call is chosen at run time.
func run(exporter Exporter, rows []Row) {
	fmt.Println("==", exporter.Name())
	fmt.Println(exporter.Export(rows))
}

func main() {
	rows := []Row{{"ada", 90}, {"grace", 85}, {"alan", 78}}
	exporters := []Exporter{CSV{Separator: ","}, JSON{}, Markdown{}, Summary{}}
	for _, exporter := range exporters {
		run(exporter, rows)
	}
}
```

**Python**

```python
from typing import Protocol

Row = tuple[str, int]


# A Protocol describes the methods an exporter needs. Classes don't inherit
# from it: any class with these methods counts, and a type checker such as
# mypy checks that they match.
class Exporter(Protocol):
    def name(self) -> str: ...

    def export(self, rows: list[Row]) -> str: ...


class CsvExporter:
    def __init__(self, separator: str):
        self.separator = separator

    def name(self) -> str:
        return "csv"

    def export(self, rows: list[Row]) -> str:
        lines = [f"name{self.separator}score"]
        lines += [f"{name}{self.separator}{score}" for name, score in rows]
        return "\n".join(lines)


class JsonExporter:
    def name(self) -> str:
        return "json"

    def export(self, rows: list[Row]) -> str:
        items = [f'{{"name":"{name}","score":{score}}}' for name, score in rows]
        return "[" + ",".join(items) + "]"


class MarkdownExporter:
    def name(self) -> str:
        return "markdown"

    def export(self, rows: list[Row]) -> str:
        lines = ["| name | score |", "| --- | --- |"]
        lines += [f"| {name} | {score} |" for name, score in rows]
        return "\n".join(lines)


class SummaryExporter:
    def name(self) -> str:
        return "summary"

    def export(self, rows: list[Row]) -> str:
        best_name, best_score = max(rows, key=lambda row: row[1])
        return f"{len(rows)} rows, top score {best_score} by {best_name}"


# This function knows only the Protocol, not the classes.
def run(exporter: Exporter, rows: list[Row]) -> None:
    print(f"== {exporter.name()}")
    print(exporter.export(rows))


rows = [("ada", 90), ("grace", 85), ("alan", 78)]
exporters: list[Exporter] = [
    CsvExporter(","),
    JsonExporter(),
    MarkdownExporter(),
    SummaryExporter(),
]
for exporter in exporters:
    run(exporter, rows)
```

**JavaScript**

```js
// JavaScript has no interfaces. Any object with name and exportRows methods
// can be used as an exporter: this is called duck typing.
class CsvExporter {
  constructor(separator) {
    this.separator = separator;
  }

  name() {
    return "csv";
  }

  exportRows(rows) {
    const sep = this.separator;
    const lines = rows.map((r) => `${r.name}${sep}${r.score}`);
    return [`name${sep}score`, ...lines].join("\n");
  }
}

class JsonExporter {
  name() {
    return "json";
  }

  exportRows(rows) {
    return JSON.stringify(rows);
  }
}

class MarkdownExporter {
  name() {
    return "markdown";
  }

  exportRows(rows) {
    const lines = rows.map((r) => `| ${r.name} | ${r.score} |`);
    return ["| name | score |", "| --- | --- |", ...lines].join("\n");
  }
}

// Not every exporter needs a class. A plain object with the right methods
// works just as well.
const summaryExporter = {
  name() {
    return "summary";
  },
  exportRows(rows) {
    const best = rows.reduce((a, b) => (b.score > a.score ? b : a));
    return `${rows.length} rows, top score ${best.score} by ${best.name}`;
  },
};

function run(exporter, rows) {
  console.log(`== ${exporter.name()}`);
  console.log(exporter.exportRows(rows));
}

const rows = [
  { name: "ada", score: 90 },
  { name: "grace", score: 85 },
  { name: "alan", score: 78 },
];
const exporters = [
  new CsvExporter(","),
  new JsonExporter(),
  new MarkdownExporter(),
  summaryExporter,
];
for (const exporter of exporters) {
  run(exporter, rows);
}
```

**TypeScript**

```ts
interface Row {
  name: string;
  score: number;
}

// TypeScript's interfaces are structural: any value with these members is an
// Exporter, whether or not its class says "implements Exporter".
interface Exporter {
  name(): string;
  exportRows(rows: Row[]): string;
}

class CsvExporter implements Exporter {
  readonly separator: string;
  constructor(separator: string) {
    this.separator = separator;
  }

  name(): string {
    return "csv";
  }

  exportRows(rows: Row[]): string {
    const sep = this.separator;
    const lines = rows.map((r) => `${r.name}${sep}${r.score}`);
    return [`name${sep}score`, ...lines].join("\n");
  }
}

class JsonExporter implements Exporter {
  name(): string {
    return "json";
  }

  exportRows(rows: Row[]): string {
    return JSON.stringify(rows);
  }
}

class MarkdownExporter implements Exporter {
  name(): string {
    return "markdown";
  }

  exportRows(rows: Row[]): string {
    const lines = rows.map((r) => `| ${r.name} | ${r.score} |`);
    return ["| name | score |", "| --- | --- |", ...lines].join("\n");
  }
}

// A plain object is an Exporter too, because it has the right shape.
const summaryExporter: Exporter = {
  name() {
    return "summary";
  },
  exportRows(rows) {
    const best = rows.reduce((a, b) => (b.score > a.score ? b : a));
    return `${rows.length} rows, top score ${best.score} by ${best.name}`;
  },
};

function run(exporter: Exporter, rows: Row[]): void {
  console.log(`== ${exporter.name()}`);
  console.log(exporter.exportRows(rows));
}

const rows: Row[] = [
  { name: "ada", score: 90 },
  { name: "grace", score: 85 },
  { name: "alan", score: 78 },
];
const exporters: Exporter[] = [
  new CsvExporter(","),
  new JsonExporter(),
  new MarkdownExporter(),
  summaryExporter,
];
for (const exporter of exporters) {
  run(exporter, rows);
}
```

**Dart**

```dart
typedef Row = ({String name, int score});

// Outside this file, an abstract interface class can only be implemented: not
// extended, and never made directly. Every Dart class also defines an
// interface of its own, so `implements` works with any class.
abstract interface class Exporter {
  String get name;
  String export(List<Row> rows);
}

class CsvExporter implements Exporter {
  final String separator;
  CsvExporter(this.separator);

  @override
  String get name => 'csv';

  @override
  String export(List<Row> rows) => [
    'name${separator}score',
    for (final row in rows) '${row.name}$separator${row.score}',
  ].join('\n');
}

class JsonExporter implements Exporter {
  @override
  String get name => 'json';

  @override
  String export(List<Row> rows) {
    final items = [
      for (final row in rows) '{"name":"${row.name}","score":${row.score}}',
    ];
    return '[${items.join(',')}]';
  }
}

class MarkdownExporter implements Exporter {
  @override
  String get name => 'markdown';

  @override
  String export(List<Row> rows) => [
    '| name | score |',
    '| --- | --- |',
    for (final row in rows) '| ${row.name} | ${row.score} |',
  ].join('\n');
}

class SummaryExporter implements Exporter {
  @override
  String get name => 'summary';

  @override
  String export(List<Row> rows) {
    final best = rows.reduce((a, b) => b.score > a.score ? b : a);
    return '${rows.length} rows, top score ${best.score} by ${best.name}';
  }
}

void run(Exporter exporter, List<Row> rows) {
  print('== ${exporter.name}');
  print(exporter.export(rows));
}

void main() {
  final rows = <Row>[
    (name: 'ada', score: 90),
    (name: 'grace', score: 85),
    (name: 'alan', score: 78),
  ];
  final exporters = <Exporter>[
    CsvExporter(','),
    JsonExporter(),
    MarkdownExporter(),
    SummaryExporter(),
  ];
  for (final exporter in exporters) {
    run(exporter, rows);
  }
}
```

**Kotlin**

```kotlin
data class Row(val name: String, val score: Int)

// An interface lists what an exporter must have. A class says ": Exporter"
// to implement it, and must override every member.
interface Exporter {
    val name: String

    fun export(rows: List<Row>): String
}

class CsvExporter(private val separator: String) : Exporter {
    override val name = "csv"

    override fun export(rows: List<Row>) =
        (listOf("name${separator}score") + rows.map { "${it.name}$separator${it.score}" })
            .joinToString("\n")
}

class JsonExporter : Exporter {
    override val name = "json"

    override fun export(rows: List<Row>) =
        rows.joinToString(",", "[", "]") { """{"name":"${it.name}","score":${it.score}}""" }
}

class MarkdownExporter : Exporter {
    override val name = "markdown"

    override fun export(rows: List<Row>) =
        (listOf("| name | score |", "| --- | --- |") + rows.map { "| ${it.name} | ${it.score} |" })
            .joinToString("\n")
}

// An object expression makes a one-off object that implements an interface.
val summaryExporter =
    object : Exporter {
        override val name = "summary"

        override fun export(rows: List<Row>): String {
            val best = rows.maxBy { it.score }
            return "${rows.size} rows, top score ${best.score} by ${best.name}"
        }
    }

fun run(exporter: Exporter, rows: List<Row>) {
    println("== ${exporter.name}")
    println(exporter.export(rows))
}

fun main() {
    val rows = listOf(Row("ada", 90), Row("grace", 85), Row("alan", 78))
    val exporters = listOf(CsvExporter(","), JsonExporter(), MarkdownExporter(), summaryExporter)
    for (exporter in exporters) {
        run(exporter, rows)
    }
}
```

All nine print the same lines:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
== csv
name,score
ada,90
grace,85
alan,78
== json
[{"name":"ada","score":90},{"name":"grace","score":85},{"name":"alan","score":78}]
== markdown
| name | score |
| --- | --- |
| ada | 90 |
| grace | 85 |
| alan | 78 |
== summary
3 rows, top score 90 by ada
```

**C: a table of function pointers.** C builds an interface by hand.
`struct exporter_vtable` holds one function pointer per method, and every exporter
starts with a pointer to a table. `run` calls `exporter->vtable->export(exporter, ...)`
and passes the object itself as `self`, which is what C++ does behind the scenes for a
`virtual` call. The CSV exporter has data of its own, so `struct csv_exporter` puts a
`struct exporter` first. C guarantees that a pointer to a struct, converted, points to
its first member, so `&csv.base` can be passed as an exporter, and `csv_export` can
convert it back. Large C programs are built this way. The Linux kernel's
`struct file_operations` is a table of function pointers that each driver fills in.

**C++: abstract classes and `virtual`.** `Exporter` has only pure virtual functions,
marked `= 0`, so it can't be made on its own, and every subclass must define them.
`override` asks the compiler to check that a function really does override one in the
base class. The base class has a `virtual` destructor, so deleting a `CsvExporter`
through an `Exporter` pointer runs the right destructor. The vector holds
`std::unique_ptr<Exporter>`, because objects of different sizes can't sit side by side
in one vector.

**Rust: traits and trait objects.** A trait lists methods, and
`impl Exporter for CsvExporter` says that a type has them. `&dyn Exporter` and
`Box<dyn Exporter>` are **trait objects**: a pointer to the value and a pointer to its
type's table of methods. Not every trait can be used this way. A trait with a generic
method can't, since its table would need an entry for every type argument, and the
compiler says `the trait 'Exporter' is not dyn compatible`. Chapter 13 covers the other
way to use a trait, generics, where the compiler makes a copy of the code for each type
and no table is needed.

**Go: interfaces are satisfied silently.** `CSV` never mentions `Exporter`. It has
`Name` and `Export` methods, and that is enough. The upside is that a package can define
an interface for types it doesn't own. The downside is that a missing method shows up
only where a value is used as an `Exporter`. `var _ Exporter = CSV{}` puts that check
next to the type: if `CSV` loses `Export`, the build fails with
`CSV does not implement Exporter (missing method Export)`.

**Python: a `Protocol` for the type checker.** Python calls whatever method the object
has. `Exporter(Protocol)` writes the expectation down for a type checker such as mypy,
which checks it by shape, as Go does: the classes never inherit from `Exporter`. With
`export` misspelled, `mypy --strict` reports
`"SummaryExporter" is missing following "Exporter" protocol member: export`. At run time
a `Protocol` does nothing, and `isinstance(x, Exporter)` raises a `TypeError` unless the
protocol is marked `@runtime_checkable`. The other choice, `abc.ABC`, works by name:
classes inherit from it, and Python refuses to make one that lacks an abstract method
(exercise 12.1).

**JavaScript: duck typing.** If it walks like a duck and quacks like a duck, it is a
duck. Any object with `name` and `exportRows` methods works, including the plain object
`summaryExporter`. Nothing checks this ahead of time, and a missing method fails only
when it is called: `TypeError: exporter.exportRows is not a function`.

**TypeScript: structural interfaces.** TypeScript writes JavaScript's duck typing down.
`interface Exporter` is a shape, and any value with that shape is an `Exporter`, whether
or not its class says `implements Exporter`. `implements` only asks for the check to
happen at the class. Otherwise a missing method is an error where the value is used,
which begins `Property 'exportRows' is missing in type '{ name(): string; }'`.
Interfaces are erased, so nothing of them is left at run time.

**Dart: every class is an interface.** Dart needs no separate interface declarations,
because any class can be used with `implements`, which promises its members and brings
none of its code. `abstract interface class` declares a class that is only an interface:
code in other files can implement it, but can't extend it or make one. A record type,
`({String name, int score})`, gives the rows named fields without a class.

**Kotlin: interfaces with properties.** A Kotlin interface can declare properties, like
`val name`, as well as methods. `object : Exporter { ... }` is an **object expression**:
a one-off object of an unnamed class that implements the interface.

---

## 12.5 Inheritance and composition

The best-known example of inheritance going wrong is in Joshua Bloch's Effective Java:
count the items added to a collection. Here `Bag` has `add` and `add_all`, and `add_all`
happens to call `add` for each item. A subclass that counts in both methods counts each
item from `add_all` twice. The same counting bag built by composition holds a `Bag` and
passes calls on to it, and counts correctly.

**C**

```c
#include <stdio.h>

#define CAPACITY 16

// C has no inheritance, so a counting bag is built by composition: it holds a
// bag and calls the bag's functions.
struct bag {
    const char *items[CAPACITY];
    int size;
};

void bag_add(struct bag *bag, const char *item) {
    if (bag->size < CAPACITY) {
        bag->items[bag->size++] = item;
    }
}

void bag_add_all(struct bag *bag, const char *const *items, int count) {
    for (int i = 0; i < count; i++) {
        bag_add(bag, items[i]); // always the plain bag_add
    }
}

struct counting_bag {
    struct bag bag;
    int count;
};

void counting_bag_add(struct counting_bag *bag, const char *item) {
    bag->count++;
    bag_add(&bag->bag, item);
}

void counting_bag_add_all(struct counting_bag *bag, const char *const *items,
                          int count) {
    bag->count += count;
    bag_add_all(&bag->bag, items, count);
}

int main(void) {
    struct counting_bag bag = {0};
    const char *fruit[] = {"apple", "pear", "plum"};
    counting_bag_add_all(&bag, fruit, 3);
    counting_bag_add(&bag, "fig");
    printf("composition: the bag holds %d items, the count says %d\n",
           bag.bag.size, bag.count);
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <string>
#include <vector>

// A bag of items. add_all is written in terms of add, and add is virtual, so a
// subclass that overrides add changes what add_all does.
class Bag {
  public:
    virtual ~Bag() = default;

    virtual void add(const std::string &item) { items_.push_back(item); }

    virtual void add_all(const std::vector<std::string> &items) {
        for (const std::string &item : items) {
            add(item); // calls the subclass's add, if it has one
        }
    }

    std::size_t size() const { return items_.size(); }

  private:
    std::vector<std::string> items_;
};

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag : public Bag {
  public:
    void add(const std::string &item) override {
        count++;
        Bag::add(item);
    }

    void add_all(const std::vector<std::string> &items) override {
        count += items.size();
        Bag::add_all(items); // which calls add, which counts again
    }

    std::size_t count = 0;
};

// Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag {
  public:
    void add(const std::string &item) {
        count++;
        bag_.add(item);
    }

    void add_all(const std::vector<std::string> &items) {
        count += items.size();
        bag_.add_all(items); // the inner Bag calls its own add
    }

    std::size_t size() const { return bag_.size(); }

    std::size_t count = 0;

  private:
    Bag bag_;
};

template <typename CountingBag> void fill(const std::string &label) {
    CountingBag bag;
    bag.add_all({"apple", "pear", "plum"});
    bag.add("fig");
    std::cout << label << ": the bag holds " << bag.size()
              << " items, the count says " << bag.count << '\n';
}

int main() {
    fill<InheritedCountingBag>("inheritance");
    fill<ComposedCountingBag>("composition");
}
```

**Rust**

```rust
// Rust has no inheritance. A trait can still share code: add_all has a default
// body, written in terms of add, which every type that implements the trait
// must provide.
trait Collection {
    fn add(&mut self, item: &str);
    fn size(&self) -> usize;

    fn add_all(&mut self, items: &[&str]) {
        for item in items {
            self.add(item);
        }
    }
}

#[derive(Default)]
struct Bag {
    items: Vec<String>,
}

impl Collection for Bag {
    fn add(&mut self, item: &str) {
        self.items.push(item.to_string());
    }

    fn size(&self) -> usize {
        self.items.len()
    }
}

// Composition: a CountingBag has a Bag, and passes the work on to it.
#[derive(Default)]
struct CountingBag {
    bag: Bag,
    count: usize,
}

impl Collection for CountingBag {
    fn add(&mut self, item: &str) {
        self.count += 1;
        self.bag.add(item);
    }

    fn size(&self) -> usize {
        self.bag.size()
    }

    // add_all is not written here, so the default one is used. It calls
    // CountingBag's add, which counts each item once.
}

fn main() {
    let mut bag = CountingBag::default();
    bag.add_all(&["apple", "pear", "plum"]);
    bag.add("fig");
    println!(
        "composition: the bag holds {} items, the count says {}",
        bag.size(),
        bag.count
    );
}
```

**Go**

```go
package main

import "fmt"

// A bag of items. AddAll is written in terms of Add.
type Bag struct {
	items []string
}

func (b *Bag) Add(item string) {
	b.items = append(b.items, item)
}

func (b *Bag) AddAll(items []string) {
	for _, item := range items {
		b.Add(item) // b is a *Bag, so this is always Bag's Add
	}
}

func (b *Bag) Size() int {
	return len(b.items)
}

// Embedding: CountingBag contains a Bag, and Bag's methods are promoted, so
// bag.Size() works without writing it out. This is composition, not
// inheritance: Bag's methods never call back into CountingBag.
type CountingBag struct {
	Bag
	count int
}

func (c *CountingBag) Add(item string) {
	c.count++
	c.Bag.Add(item)
}

func (c *CountingBag) AddAll(items []string) {
	c.count += len(items)
	c.Bag.AddAll(items)
}

// HalfCountingBag wraps only Add. Its AddAll is Bag's, which calls Bag's Add,
// so the items added that way are never counted.
type HalfCountingBag struct {
	Bag
	count int
}

func (c *HalfCountingBag) Add(item string) {
	c.count++
	c.Bag.Add(item)
}

func report(label string, size, count int) {
	fmt.Printf("%s: the bag holds %d items, the count says %d\n", label, size, count)
}

func main() {
	fruit := []string{"apple", "pear", "plum"}

	bag := &CountingBag{}
	bag.AddAll(fruit)
	bag.Add("fig")
	report("embedding", bag.Size(), bag.count)

	half := &HalfCountingBag{}
	half.AddAll(fruit)
	half.Add("fig")
	report("embedding, Add only", half.Size(), half.count)
}
```

**Python**

```python
class Bag:
    """A bag of items. add_all is written in terms of add."""

    def __init__(self) -> None:
        self._items: list[str] = []

    def add(self, item: str) -> None:
        self._items.append(item)

    def add_all(self, items: list[str]) -> None:
        for item in items:
            self.add(item)  # calls the subclass's add, if it has one

    def size(self) -> int:
        return len(self._items)


# Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag(Bag):
    def __init__(self) -> None:
        super().__init__()
        self.count = 0

    def add(self, item: str) -> None:
        self.count += 1
        super().add(item)

    def add_all(self, items: list[str]) -> None:
        self.count += len(items)
        super().add_all(items)  # which calls self.add, which counts again


# Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag:
    def __init__(self) -> None:
        self._bag = Bag()
        self.count = 0

    def add(self, item: str) -> None:
        self.count += 1
        self._bag.add(item)

    def add_all(self, items: list[str]) -> None:
        self.count += len(items)
        self._bag.add_all(items)  # the inner Bag calls its own add

    def size(self) -> int:
        return self._bag.size()


bags: list[tuple[str, InheritedCountingBag | ComposedCountingBag]] = [
    ("inheritance", InheritedCountingBag()),
    ("composition", ComposedCountingBag()),
]
for label, bag in bags:
    bag.add_all(["apple", "pear", "plum"])
    bag.add("fig")
    print(f"{label}: the bag holds {bag.size()} items, the count says {bag.count}")
```

**JavaScript**

```js
// A bag of items. addAll is written in terms of add.
class Bag {
  #items = [];

  add(item) {
    this.#items.push(item);
  }

  addAll(items) {
    for (const item of items) {
      this.add(item); // calls the subclass's add, if it has one
    }
  }

  size() {
    return this.#items.length;
  }
}

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag extends Bag {
  count = 0;

  add(item) {
    this.count++;
    super.add(item);
  }

  addAll(items) {
    this.count += items.length;
    super.addAll(items); // which calls this.add, which counts again
  }
}

// Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag {
  #bag = new Bag();
  count = 0;

  add(item) {
    this.count++;
    this.#bag.add(item);
  }

  addAll(items) {
    this.count += items.length;
    this.#bag.addAll(items); // the inner Bag calls its own add
  }

  size() {
    return this.#bag.size();
  }
}

for (const [label, bag] of [
  ["inheritance", new InheritedCountingBag()],
  ["composition", new ComposedCountingBag()],
]) {
  bag.addAll(["apple", "pear", "plum"]);
  bag.add("fig");
  console.log(
    `${label}: the bag holds ${bag.size()} items, the count says ${bag.count}`,
  );
}
```

**TypeScript**

```ts
// A bag of items. addAll is written in terms of add.
class Bag {
  private readonly items: string[] = [];

  add(item: string): void {
    this.items.push(item);
  }

  addAll(items: string[]): void {
    for (const item of items) {
      this.add(item); // calls the subclass's add, if it has one
    }
  }

  size(): number {
    return this.items.length;
  }
}

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag extends Bag {
  count = 0;

  override add(item: string): void {
    this.count++;
    super.add(item);
  }

  override addAll(items: string[]): void {
    this.count += items.length;
    super.addAll(items); // which calls this.add, which counts again
  }
}

// Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag {
  private readonly bag = new Bag();
  count = 0;

  add(item: string): void {
    this.count++;
    this.bag.add(item);
  }

  addAll(items: string[]): void {
    this.count += items.length;
    this.bag.addAll(items); // the inner Bag calls its own add
  }

  size(): number {
    return this.bag.size();
  }
}

const bags: [string, InheritedCountingBag | ComposedCountingBag][] = [
  ["inheritance", new InheritedCountingBag()],
  ["composition", new ComposedCountingBag()],
];
for (const [label, bag] of bags) {
  bag.addAll(["apple", "pear", "plum"]);
  bag.add("fig");
  console.log(
    `${label}: the bag holds ${bag.size()} items, the count says ${bag.count}`,
  );
}
```

**Dart**

```dart
// A bag of items. addAll is written in terms of add.
class Bag {
  final List<String> _items = [];

  void add(String item) {
    _items.add(item);
  }

  void addAll(List<String> items) {
    for (final item in items) {
      add(item); // calls the subclass's add, if it has one
    }
  }

  int get size => _items.length;
}

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag extends Bag {
  int count = 0;

  @override
  void add(String item) {
    count++;
    super.add(item);
  }

  @override
  void addAll(List<String> items) {
    count += items.length;
    super.addAll(items); // which calls add, which counts again
  }
}

// Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag {
  final Bag _bag = Bag();
  int count = 0;

  void add(String item) {
    count++;
    _bag.add(item);
  }

  void addAll(List<String> items) {
    count += items.length;
    _bag.addAll(items); // the inner Bag calls its own add
  }

  int get size => _bag.size;
}

void report(String label, int size, int count) {
  print('$label: the bag holds $size items, the count says $count');
}

void main() {
  final fruit = ['apple', 'pear', 'plum'];

  final inherited = InheritedCountingBag();
  inherited.addAll(fruit);
  inherited.add('fig');
  report('inheritance', inherited.size, inherited.count);

  final composed = ComposedCountingBag();
  composed.addAll(fruit);
  composed.add('fig');
  report('composition', composed.size, composed.count);
}
```

**Kotlin**

```kotlin
interface ItemBag {
    fun add(item: String)

    fun addAll(items: List<String>)

    val size: Int
}

// A bag of items. addAll is written in terms of add. Kotlin classes and
// methods are final unless marked open, so Bag has to opt in to being extended.
open class Bag : ItemBag {
    private val items = mutableListOf<String>()

    override fun add(item: String) {
        items.add(item)
    }

    override fun addAll(items: List<String>) {
        for (item in items) {
            add(item) // calls the subclass's add, if it has one
        }
    }

    override val size
        get() = items.size
}

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag : Bag() {
    var count = 0

    override fun add(item: String) {
        count++
        super.add(item)
    }

    override fun addAll(items: List<String>) {
        count += items.size
        super.addAll(items) // which calls add, which counts again
    }
}

// Composition: a CountingBag has a Bag. "by bag" writes out every ItemBag
// member as a call to bag, so only the members that change are written here.
class ComposedCountingBag(private val bag: Bag = Bag()) : ItemBag by bag {
    var count = 0

    override fun add(item: String) {
        count++
        bag.add(item)
    }

    override fun addAll(items: List<String>) {
        count += items.size
        bag.addAll(items) // the inner Bag calls its own add
    }
}

fun report(label: String, size: Int, count: Int) {
    println("$label: the bag holds $size items, the count says $count")
}

fun main() {
    val fruit = listOf("apple", "pear", "plum")

    val inherited = InheritedCountingBag()
    inherited.addAll(fruit)
    inherited.add("fig")
    report("inheritance", inherited.size, inherited.count)

    val composed = ComposedCountingBag()
    composed.addAll(fruit)
    composed.add("fig")
    report("composition", composed.size, composed.count)
}
```

The languages with inheritance show both bags. Go shows two kinds of embedding, and C
and Rust have only composition:

*C and Rust:*

```text
composition: the bag holds 4 items, the count says 4
```

*C++, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
inheritance: the bag holds 4 items, the count says 7
composition: the bag holds 4 items, the count says 4
```

*Go:*

```text
embedding: the bag holds 4 items, the count says 4
embedding, Add only: the bag holds 4 items, the count says 1
```

The subclass broke because it relied on something `Bag` never promised: that `add_all`
calls `add`. Nothing in `Bag`'s interface says so, and a later version of `Bag` could
change it and break the subclass in a different way. This is the **fragile base class**
problem. The composed bag relies only on what `Bag` does, not on how it does it. The
Gang of Four's Design Patterns put the rule in 1994: "Favor object composition over
class inheritance."

- **C++** looks up only `virtual` functions at run time. `add` is `virtual`, so
  `Bag::add_all` calls the subclass's `add`, and the count is 7. With `add` not virtual,
  `add_all` would call `Bag::add` directly, and the inherited bag would count 4.
- **Python, JavaScript, TypeScript, and Dart** let a subclass override any method, so
  `self.add` inside `Bag` runs the subclass's `add`.
- **Kotlin** makes classes and methods final unless they say `open`, so `Bag` is an
  `open class` with `open fun`s. A plain `class Bag` can't be extended at all:
  `this type is final, so it cannot be extended.` The composed bag uses **delegation**:
  `ItemBag by bag` writes every `ItemBag` member as a call to `bag`, so the class writes
  only `add` and `addAll`, and `size` comes for free.
- **Go** has no inheritance. **Embedding** `Bag` in `CountingBag` promotes `Bag`'s
  methods, so `bag.Size()` works without being written out, which looks like
  inheritance. It is composition. `Bag`'s `AddAll` has a `*Bag`, and it always calls
  `Bag`'s `Add`. So `CountingBag` counts correctly, and `HalfCountingBag`, which wraps
  only `Add`, misses every item that `AddAll` adds and counts 1.
- **Rust** has no inheritance either. The `Collection` trait gives `add_all` a default
  body written in terms of `add`, and `CountingBag` holds a `Bag`. `CountingBag` doesn't
  write `add_all`, so the default one runs, and it calls `CountingBag`'s `add` once per
  item. A default method is where a Rust trait calls code that another type supplies, so
  it carries the same promise as `Bag`: which methods it calls.
- **C** has only composition: a `struct counting_bag` holds a `struct bag`, and its
  functions call the bag's.

So when is inheritance the right tool? When the subclass really is a kind of the base
class everywhere it is used, and the base class was designed to be extended, with its
overridable methods and the calls between them written down. Exercise 12.1's bank
accounts are that case: `Account` is built for its subclasses, and its `withdraw` calls
`check_withdrawal` so that each kind can fill it in. When a type only wants to reuse
code, composition is the safer choice.

---

## 12.6 Mixins and default methods

Some behavior belongs to classes that have nothing else in common. A door and a light
are not kinds of each other, but both can describe themselves, and both can keep a log
of what happened to them. This example writes "describe yourself" once, with a default
that a class can replace, and "keep a history" once, with its own state, and gives both
to `Door` and `Light`. `Light` replaces the default description, and reuses it.

**C**

```c
#include <stdbool.h>
#include <stdio.h>

#define MAX_EVENTS 8

// Every thing starts with this struct. The vtable lists the functions a kind
// of thing may replace; NULL means "use the default".
struct thing;
struct thing_vtable {
    void (*describe)(const struct thing *self);
};

struct thing {
    const struct thing_vtable *vtable;
    const char *name;
    char history[MAX_EVENTS][48];
    int events;
};

void describe_default(const struct thing *self) {
    printf("this is the %s", self->name);
}

void describe(const struct thing *self) {
    if (self->vtable->describe != NULL) {
        self->vtable->describe(self);
    } else {
        describe_default(self);
    }
    printf("\n");
}

void log_event(struct thing *self, const char *event) {
    if (self->events < MAX_EVENTS) {
        snprintf(self->history[self->events++], sizeof self->history[0],
                 "%s: %s", self->name, event);
    }
}

// A door uses every default.
static const struct thing_vtable door_vtable = {NULL};

void door_open(struct thing *door) { log_event(door, "opened"); }
void door_close(struct thing *door) { log_event(door, "closed"); }

// A light has its own data, and its own describe.
struct light {
    struct thing base;
    bool on;
};

void light_describe(const struct thing *self) {
    const struct light *light = (const struct light *)self;
    describe_default(self);
    printf(", and it is %s", light->on ? "on" : "off");
}

static const struct thing_vtable light_vtable = {light_describe};

void light_switch_on(struct light *light) {
    light->on = true;
    log_event(&light->base, "switched on");
}

int main(void) {
    struct thing door = {.vtable = &door_vtable, .name = "front door"};
    struct light light = {
        .base = {.vtable = &light_vtable, .name = "porch light"}};
    door_open(&door);
    door_close(&door);
    light_switch_on(&light);

    const struct thing *things[] = {&door, &light.base};
    for (int i = 0; i < 2; i++) {
        describe(things[i]);
        for (int j = 0; j < things[i]->events; j++) {
            printf("  %s\n", things[i]->history[j]);
        }
    }
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// A mixin in C++ is often a class template that takes the class using it as a
// parameter (the curiously recurring template pattern, or CRTP). The mixin
// casts itself to that class to call its name(), with no virtual functions.
template <typename Self> class Describe {
  public:
    std::string describe() const { return "this is the " + self().name(); }

  private:
    const Self &self() const { return static_cast<const Self &>(*this); }
};

template <typename Self> class Logged {
  public:
    void log(const std::string &event) {
        history_.push_back(self().name() + ": " + event);
    }

    const std::vector<std::string> &history() const { return history_; }

  private:
    const Self &self() const { return static_cast<const Self &>(*this); }

    std::vector<std::string> history_;
};

class Door : public Describe<Door>, public Logged<Door> {
  public:
    explicit Door(std::string name) : name_(std::move(name)) {}

    const std::string &name() const { return name_; }
    void open() { log("opened"); }
    void close() { log("closed"); }

  private:
    std::string name_;
};

class Light : public Describe<Light>, public Logged<Light> {
  public:
    explicit Light(std::string name) : name_(std::move(name)) {}

    const std::string &name() const { return name_; }

    void switch_on() {
        on_ = true;
        log("switched on");
    }

    // Light's describe hides the mixin's, and reuses it.
    std::string describe() const {
        return Describe<Light>::describe() + ", and it is " +
               (on_ ? "on" : "off");
    }

  private:
    std::string name_;
    bool on_ = false;
};

// Door and Light share no base class, so a template prints either one.
template <typename Thing> void print(const Thing &thing) {
    std::cout << thing.describe() << '\n';
    for (const std::string &line : thing.history()) {
        std::cout << "  " << line << '\n';
    }
}

int main() {
    Door door("front door");
    Light light("porch light");
    door.open();
    door.close();
    light.switch_on();
    print(door);
    print(light);
}
```

**Rust**

```rust
// A trait can give a method a default body. Every type that implements
// Describe must say what its name is, and gets describe for free.
trait Describe {
    fn name(&self) -> &str;

    fn describe(&self) -> String {
        format!("this is the {}", self.name())
    }
}

// Logged needs a name too, so it requires Describe (a supertrait). A trait has
// no fields, so it asks the type for access to the history the type stores.
trait Logged: Describe {
    fn history(&self) -> &[String];
    fn history_mut(&mut self) -> &mut Vec<String>;

    fn log(&mut self, event: &str) {
        let line = format!("{}: {event}", self.name());
        self.history_mut().push(line);
    }
}

struct Door {
    name: String,
    history: Vec<String>,
}

impl Door {
    fn open(&mut self) {
        self.log("opened");
    }

    fn close(&mut self) {
        self.log("closed");
    }
}

impl Describe for Door {
    fn name(&self) -> &str {
        &self.name
    }
}

impl Logged for Door {
    fn history(&self) -> &[String] {
        &self.history
    }

    fn history_mut(&mut self) -> &mut Vec<String> {
        &mut self.history
    }
}

struct Light {
    name: String,
    history: Vec<String>,
    on: bool,
}

impl Light {
    fn switch_on(&mut self) {
        self.on = true;
        self.log("switched on");
    }
}

// Light replaces the default describe. Rust has no way to call the default
// body from here, so the text is written out again.
impl Describe for Light {
    fn name(&self) -> &str {
        &self.name
    }

    fn describe(&self) -> String {
        let state = if self.on { "on" } else { "off" };
        format!("this is the {}, and it is {state}", self.name)
    }
}

impl Logged for Light {
    fn history(&self) -> &[String] {
        &self.history
    }

    fn history_mut(&mut self) -> &mut Vec<String> {
        &mut self.history
    }
}

// This function needs only Describe.
fn print_description(thing: &dyn Describe) {
    println!("{}", thing.describe());
}

fn main() {
    let mut door = Door {
        name: "front door".to_string(),
        history: Vec::new(),
    };
    let mut light = Light {
        name: "porch light".to_string(),
        history: Vec::new(),
        on: false,
    };
    door.open();
    door.close();
    light.switch_on();

    let things: [&dyn Logged; 2] = [&door, &light];
    for thing in things {
        // A &dyn Logged converts to a &dyn Describe, because Describe is a
        // supertrait of Logged (trait upcasting, Rust 1.86 and later).
        print_description(thing);
        for line in thing.history() {
            println!("  {line}");
        }
    }
}
```

**Go**

```go
package main

import "fmt"

// Go interfaces have no default methods. Shared code goes in a struct that
// other structs embed. Named supplies a default Describe.
type Named struct {
	name string
}

func (n Named) Describe() string {
	return "this is the " + n.name
}

// Logged holds a history. An embedded struct cannot see the struct around it,
// so Logged cannot look up the name for itself: the caller passes it in.
type Logged struct {
	history []string
}

func (l *Logged) log(name, event string) {
	l.history = append(l.history, name+": "+event)
}

func (l *Logged) History() []string {
	return l.history
}

type Door struct {
	Named
	Logged
}

func (d *Door) Open() {
	d.log(d.name, "opened")
}

func (d *Door) Close() {
	d.log(d.name, "closed")
}

type Light struct {
	Named
	Logged
	on bool
}

func (l *Light) SwitchOn() {
	l.on = true
	l.log(l.name, "switched on")
}

// Light's own Describe hides the promoted one, and can still call it.
func (l *Light) Describe() string {
	state := "off"
	if l.on {
		state = "on"
	}
	return l.Named.Describe() + ", and it is " + state
}

type Thing interface {
	Describe() string
	History() []string
}

func main() {
	door := &Door{Named: Named{"front door"}}
	light := &Light{Named: Named{"porch light"}}
	door.Open()
	door.Close()
	light.SwitchOn()

	for _, thing := range []Thing{door, light} {
		fmt.Println(thing.Describe())
		for _, line := range thing.History() {
			fmt.Println("  " + line)
		}
	}
}
```

**Python**

```python
class Describe:
    """A mixin: describe() works for any class that has a name."""

    name: str

    def describe(self) -> str:
        return f"this is the {self.name}"


class Logged:
    """A mixin with state: each object gets its own history."""

    name: str

    def __init__(self) -> None:
        super().__init__()
        self.history: list[str] = []

    def log(self, event: str) -> None:
        self.history.append(f"{self.name}: {event}")


# Door and Light share no base class of their own. Each one mixes in the
# behavior it wants.
class Door(Describe, Logged):
    def __init__(self, name: str) -> None:
        super().__init__()  # finds Logged.__init__ by searching the MRO
        self.name = name

    def open(self) -> None:
        self.log("opened")

    def close(self) -> None:
        self.log("closed")


class Light(Describe, Logged):
    def __init__(self, name: str) -> None:
        super().__init__()
        self.name = name
        self.on = False

    def switch_on(self) -> None:
        self.on = True
        self.log("switched on")

    # Light replaces the default describe, and reuses it.
    def describe(self) -> str:
        state = "on" if self.on else "off"
        return f"{super().describe()}, and it is {state}"


door = Door("front door")
light = Light("porch light")
door.open()
door.close()
light.switch_on()

things: list[Door | Light] = [door, light]
for thing in things:
    print(thing.describe())
    for line in thing.history:
        print(f"  {line}")
```

**JavaScript**

```js
// A mixin in JavaScript is often a function that takes a class and returns a
// subclass of it with more methods. Describe needs only this.name.
const Describe = (Base) =>
  class extends Base {
    describe() {
      return `this is the ${this.name}`;
    }
  };

// A mixin made this way can have fields, including private ones.
const Logged = (Base) =>
  class extends Base {
    #history = [];

    log(event) {
      this.#history.push(`${this.name}: ${event}`);
    }

    get history() {
      return [...this.#history];
    }
  };

class Named {
  constructor(name) {
    this.name = name;
  }
}

// Door and Light each build their own chain: Named, then Describe, then Logged.
class Door extends Logged(Describe(Named)) {
  open() {
    this.log("opened");
  }

  close() {
    this.log("closed");
  }
}

class Light extends Logged(Describe(Named)) {
  on = false;

  switchOn() {
    this.on = true;
    this.log("switched on");
  }

  // Light replaces the default describe, and reuses it.
  describe() {
    return `${super.describe()}, and it is ${this.on ? "on" : "off"}`;
  }
}

const door = new Door("front door");
const light = new Light("porch light");
door.open();
door.close();
light.switchOn();

for (const thing of [door, light]) {
  console.log(thing.describe());
  for (const line of thing.history) {
    console.log(`  ${line}`);
  }
}
```

**TypeScript**

```ts
// A class whose constructor takes any arguments and whose objects are a T.
type Constructor<T = object> = new (...args: any[]) => T;

// A mixin is a function that takes a class and returns a subclass of it. The
// type of Base says what the mixin needs: here, a name.
function Describe<TBase extends Constructor<{ name: string }>>(Base: TBase) {
  return class extends Base {
    describe(): string {
      return `this is the ${this.name}`;
    }
  };
}

function Logged<TBase extends Constructor<{ name: string }>>(Base: TBase) {
  return class extends Base {
    #history: string[] = [];

    log(event: string): void {
      this.#history.push(`${this.name}: ${event}`);
    }

    get history(): readonly string[] {
      return this.#history;
    }
  };
}

class Named {
  readonly name: string;

  constructor(name: string) {
    this.name = name;
  }
}

// Door and Light each build their own chain: Named, then Describe, then Logged.
class Door extends Logged(Describe(Named)) {
  open(): void {
    this.log("opened");
  }

  close(): void {
    this.log("closed");
  }
}

class Light extends Logged(Describe(Named)) {
  on = false;

  switchOn(): void {
    this.on = true;
    this.log("switched on");
  }

  // Light replaces the default describe, and reuses it.
  override describe(): string {
    return `${super.describe()}, and it is ${this.on ? "on" : "off"}`;
  }
}

const door = new Door("front door");
const light = new Light("porch light");
door.open();
door.close();
light.switchOn();

for (const thing of [door, light]) {
  console.log(thing.describe());
  for (const line of thing.history) {
    console.log(`  ${line}`);
  }
}
```

**Dart**

```dart
// A mixin is a bundle of members that a class can add with "with". Describe
// declares name without a body, so a class that uses it must supply one.
mixin Describe {
  String get name;

  String describe() => 'this is the $name';
}

// "on Describe" says Logged can only be mixed into a class that already has
// Describe, so Logged can use name. A mixin can also have fields.
mixin Logged on Describe {
  final List<String> _history = [];

  List<String> get history => List.unmodifiable(_history);

  void log(String event) {
    _history.add('$name: $event');
  }
}

class Door with Describe, Logged {
  @override
  final String name;

  Door(this.name);

  void open() => log('opened');

  void close() => log('closed');
}

class Light with Describe, Logged {
  @override
  final String name;
  bool on = false;

  Light(this.name);

  void switchOn() {
    on = true;
    log('switched on');
  }

  // Light replaces the default describe, and reuses it.
  @override
  String describe() => '${super.describe()}, and it is ${on ? 'on' : 'off'}';
}

void main() {
  final door = Door('front door');
  final light = Light('porch light');
  door.open();
  door.close();
  light.switchOn();

  for (final Logged thing in [door, light]) {
    print(thing.describe());
    for (final line in thing.history) {
      print('  $line');
    }
  }
}
```

**Kotlin**

```kotlin
// An interface can give a method a default body. A class that implements
// Describe must supply name, and gets describe for free.
interface Describe {
    val name: String

    fun describe() = "this is the $name"
}

// An interface cannot store anything, so Logged declares the history it needs
// and each class supplies the list.
interface Logged {
    val name: String
    val history: MutableList<String>

    fun log(event: String) {
        history.add("$name: $event")
    }
}

class Door(override val name: String) : Describe, Logged {
    override val history = mutableListOf<String>()

    fun open() = log("opened")

    fun close() = log("closed")
}

class Light(override val name: String) : Describe, Logged {
    override val history = mutableListOf<String>()
    var on = false

    fun switchOn() {
        on = true
        log("switched on")
    }

    // Light replaces the default describe, and reuses it.
    override fun describe() = "${super.describe()}, and it is ${if (on) "on" else "off"}"
}

fun main() {
    val door = Door("front door")
    val light = Light("porch light")
    door.open()
    door.close()
    light.switchOn()

    for (thing in listOf(door, light)) {
        println(thing.describe())
        for (line in thing.history) {
            println("  $line")
        }
    }
}
```

All nine print the same lines:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
this is the front door
  front door: opened
  front door: closed
this is the porch light, and it is on
  porch light: switched on
```

- **Python** uses multiple inheritance. `Door(Describe, Logged)` looks for a method in
  `Door`, then `Describe`, then `Logged`, then `object`. That order is the **method
  resolution order**, and `Door.__mro__` lists it. `super()` follows the same order, so
  `Door.__init__` calling `super().__init__()` reaches `Logged.__init__`, which makes
  the history list, and `Logged`'s own `super().__init__()` reaches `object`. Every
  class in the chain calls `super()` and passes the call along, which is what makes
  multiple inheritance work in Python.
- **Rust** traits can have default methods but no fields, so `Logged` can't hold a
  history. It asks the type for access to one, with `history` and `history_mut`.
  `trait Logged: Describe` makes `Describe` a **supertrait**: every type that is
  `Logged` must be `Describe` too, which is how `log` can call `self.name()`. Since Rust
  1.86, a `&dyn Logged` converts to a `&dyn Describe` by itself, which is called **trait
  upcasting**, and is how `print_description(thing)` works. One limit: an override can't
  call the default body it replaces, so `Light`'s `describe` writes the text out again.
- **Go** has no default methods. `Named` supplies `Describe`, `Logged` supplies
  `History`, and `Door` and `Light` embed both. An embedded struct can't see the struct
  around it, so `Logged.log` can't find the name for itself, and each call passes the
  name in. `Light`'s own `Describe` hides the promoted one, and calls it as
  `l.Named.Describe()`.
- **C++** uses the **curiously recurring template pattern**, or CRTP.
  `class Door : public Describe<Door>` passes the class to its own base, so `Describe`
  can turn `*this` into a `const Door &` and call `name()`, with no virtual functions at
  all. Everything is decided at compile time, and that is also the price. `Door` and
  `Light` share no base class, so `print` is a template, and calling `describe` through
  a `const Describe<Light> &` runs the mixin's version and prints
  `this is the porch light`, without `, and it is on`.
- **JavaScript and TypeScript** write a mixin as a function that takes a class and
  returns a subclass of it: `class Door extends Logged(Describe(Named))`. Each mixin
  becomes a real class in the prototype chain, which runs
  `Door -> (anonymous) -> (anonymous) -> Named -> Object`, so private fields and
  `super.describe()` work. TypeScript checks what each mixin needs through the type of
  `Base`, here an object with a `name`. That type must be a constructor that takes
  `...args: any[]`, or TypeScript refuses:
  `A mixin class must have a constructor with a single rest parameter of type 'any[]'.`
- **Dart** has mixins in the language. `mixin Describe` declares `name` without a body,
  so a class that uses it must supply one. `mixin Logged on Describe` may only be added
  to a class that already has `Describe`, and `class Door with Logged` alone fails with
  `'Logged' can't be mixed onto 'Object' because 'Object' doesn't implement 'Describe'.`
  A Dart mixin can have fields, as the Python, C++, and JavaScript mixins do, so
  `_history` lives in the mixin.
- **Kotlin** interfaces can have default methods but no state, so `Logged` declares an
  abstract `history` property and each class supplies the list. Both interfaces declare
  `val name`, and one `override val name` satisfies both. An interface property is
  public, so the history list is public too, which a mixin with a field of its own would
  avoid.
- **C** puts the name and the history in the shared `struct thing`, and the table of
  functions holds only what a kind of thing may replace. A `NULL` entry means "use the
  default", and `describe` checks for it.

---

## 12.7 How a call finds its method

A call is **static** when the compiler knows which function it runs, and can even copy
the function's body into the caller. It is **dynamic** when the program has to look the
function up at run time. The nine languages differ in which kind you get by default, and
in what the lookup costs.

```text
 C++: the object holds the table pointer   Rust and Go: the pointer holds it

 Exporter *  -->  +-----------+            &dyn Exporter, or a Go interface
                  | vptr      |---+        +--------+--------+
                  | separator |   |        | data   | table  |
                  +-----------+   |        +--------+--------+
                                  v             |        |
                  +---------------+             v        v
                  | name          |          the value  +--------+
                  | export_rows   |                     | name   |
                  +---------------+                     | export |
                                                        +--------+
```

- **A table of functions.** C++, Rust, Go, and the hand-made C version all use a table
  of function pointers: one per class in C++, and one per pair of type and trait, or
  type and interface, in Rust and Go. In C++ the pointer to the table lives inside the
  object. On a 64-bit system, a struct holding one `int` is 4 bytes, and 16 once it has
  a virtual destructor, because the hidden pointer takes 8 bytes and the `int` is padded
  to match. Rust and Go keep the table pointer next to the pointer to the value instead:
  `&dyn Exporter` and a Go interface value are both 16 bytes, where `&CsvExporter` and a
  `*CSV` are 8. So a Rust or Go value pays nothing for dispatch until it is used through
  a trait object or an interface.
- **Who chooses.** C++ calls a member function directly unless it is `virtual`. Rust
  calls directly unless the call goes through `dyn`, and Go unless it goes through an
  interface value. Kotlin compiles every method call on an object to the JVM's
  `invokevirtual`, even for a final method, and the JVM skips the lookup when only one
  method can run. Every Dart method can be overridden, so a call is dynamic unless the
  compiler can prove which method runs.
- **Lookup by name.** Python and JavaScript find a method by its name when it is called.
  Python searches the classes in `__mro__` order, and JavaScript walks the prototype
  chain from the object up. Both cache what they find, so a call is not a full search
  every time, but the answer can change while the program runs: assigning a new function
  to `Door.describe` in Python, or to `Door.prototype.describe` in JavaScript, changes
  every door at once.

---

## 12.8 Why each language looks the way it does

- **C** gives structs and function pointers and stops there. Large C programs build
  objects out of them by convention. The Linux kernel's tables of operations and GTK's
  GObject system are two well-known examples.
- **C++** began in 1979 as Bjarne Stroustrup's "C with Classes", which took classes and
  virtual functions from Simula 67. Its rule is that you don't pay for what you don't
  use, so functions are not virtual unless you ask, and an object of a class with no
  virtual functions is laid out like a C struct. Templates later gave it a second kind
  of polymorphism, decided at compile time, and CRTP gets mixins out of them.
- **Rust** left inheritance out on purpose. Its traits come from Haskell's type classes:
  a trait says what a type can do, and a crate can implement its own traits for types
  from other crates. Data and behavior are kept apart, and code is shared through
  default methods, generics (chapter 13), and composition.
- **Go**'s FAQ asks whether Go is an object-oriented language and answers "Yes and no".
  Go has types and methods, but "there is no type hierarchy". Interfaces that are
  satisfied without saying so let a package describe what it needs from types it has
  never seen, and embedding gives some of the convenience of inheritance without the
  fragile base class.
- **Python** made everything an object, including classes and functions, and has had
  classes since its first release. Multiple inheritance is safe to use because of the C3
  method resolution order, adopted in Python 2.3, and the habit of calling `super()` in
  every method. Privacy is left to convention: a leading underscore is a request, not a
  lock.
- **JavaScript** was built on prototypes, an idea from the Self language: an object
  inherits directly from another object. The `class` syntax of ES2015 is a cleaner way
  to write the same prototype chains. `#` private fields came in ES2022, after years in
  which privacy meant closures or conventions.
- **TypeScript** has to describe JavaScript code that already exists, where objects are
  judged by their shape, so its types are structural. Its own `private` and `abstract`
  are checks that disappear when the types are erased.
- **Dart** was designed with classes from the start, with the rule that every class also
  defines an interface, so any class can be replaced by a stand-in, which helps in
  tests. It has had mixins from its early versions. Dart 3, in 2023, added the class
  modifiers `interface`, `base`, `final`, `sealed`, and `mixin`, so a library can say
  how its classes may be used outside it.
- **Kotlin** made classes final unless they are marked `open`, which matches the advice
  in Joshua Bloch's Effective Java: design and document for inheritance, or else
  prohibit it. To make composition as easy as inheritance it added delegation with `by`,
  and it made `data class`, `object`, and `sealed` classes part of the language.

---

## 12.9 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | Casting a pointer to the wrong kind of struct | Nothing checks it, and the code reads the wrong fields |
| C++ | A base class with no `virtual` destructor | Deleting a subclass through a base pointer is undefined behavior; `-Wall` warns `deleting object of abstract class type 'Exporter' which has non-virtual destructor will cause undefined behavior` |
| C++ | Copying a subclass into a base-class variable | The object is **sliced**: `Bag sliced = counting_bag;` keeps only the `Bag` part, and its calls run `Bag`'s functions |
| C++ | Calling a virtual function from a base class's constructor | It runs the base class's version, because the subclass part doesn't exist yet |
| C++ | Overriding without `override` | A typo in the parameters makes a new function; with `override` it is an error: `marked 'override', but does not override` |
| Rust | `#[derive(PartialOrd, Ord)]` on a type whose order is not field by field | The fields are compared in order, so 1/3 comes after 1/2 |
| Rust | Expecting an override to call the default method it replaces | There is no syntax for it; move the default body into a function that both can call |
| Go | Pointer-receiver methods, with a value used as the interface | `Bag does not implement Bagger (method Add has pointer receiver)`; use `&Bag{}` |
| Go | Treating embedding as inheritance | The embedded type's methods never call the outer type's, as `HalfCountingBag` shows |
| Python | `__eq__` without `__hash__` | `__hash__` becomes `None`: `TypeError: unhashable type: 'Fraction'` |
| Python | A list as a class attribute, such as `history = []` in the class body | One list, shared by every object; make it in `__init__` |
| Python | A mixin `__init__` that doesn't call `super().__init__()` | The classes after it in the MRO are never initialized |
| JavaScript | `==` or `<` on objects | `==` compares identity, and `<` compares the `toString` text, so `third < half` is `false` |
| JavaScript | Passing a method on its own, as in `const open = door.open` | `this` is lost: `TypeError: Cannot read properties of undefined (reading '#log')`; pass `() => door.open()` |
| TypeScript | Trusting `private` at run time | It is erased, and `JSON.stringify` and `Object.keys` still see the field; use `#` |
| TypeScript | `<` on two objects | Compiles without a word, and gives JavaScript's answer |
| Dart | `implements` where `extends` or `with` was meant | No code comes along, so every member must be written again |
| Dart | Overriding `==` without `hashCode` | No warning, and two equal objects both go into a `Set` |
| Kotlin | Extending a class that is not `open` | `this type is final, so it cannot be extended.` |
| All | Deep hierarchies, built to share code | Every change to a base class can break every subclass; prefer interfaces and composition |

---

## 12.10 Idiom check

**C: an opaque struct and a table of functions.** Put `struct thing;` in the header and
the fields in the `.c` file, so other files must go through your functions. Name the
functions after the type, as in `fraction_add`, and write a constructor that reports an
error instead of making a broken object, with a destroy function to match. For
polymorphism, use one struct of function pointers per kind, shared by every object of
that kind, and put the shared struct first in each kind's struct.

**C++: classes that keep their invariants, and abstract interfaces.** Make data members
private, and give the class a constructor that sets up its invariants or throws. Let
`= default` write comparisons that are field by field, and write `<=>` yourself when
they are not. For polymorphism, use an abstract class with pure virtual functions and a
public virtual destructor, mark overrides `override`, and mark classes that are not
designed to be extended `final`. Hold polymorphic objects in `std::unique_ptr`, and
prefer composition and templates to deep hierarchies.

**Rust: structs, traits, and modules.** Keep fields private, and offer `new`, returning
a `Result` if it can fail. Derive `Debug`, `Clone`, `PartialEq`, and the rest when field
by field is right, and implement them by hand when it isn't. Implement `Display` for
text meant for people. Share behavior through traits: generics when the type is known at
compile time (chapter 13), and `dyn Trait` when it isn't, as in a list of mixed types.
Use an `enum` when the set of kinds is closed (chapter 8).

**Go: small interfaces, defined where they are used.** Keep interfaces small, often one
method, like `io.Reader` and `fmt.Stringer`, and define them in the package that uses
them rather than the one that implements them. Accept interfaces and return concrete
types. Write a `NewThing` function when the zero value is not ready to use, and use
embedding for convenience, never as inheritance.

**Python: plain classes, protocols, and special methods.** Write plain classes, and use
`@dataclass` for records. Implement the special methods, such as `__eq__`, `__hash__`,
`__lt__`, `__str__`, and `__repr__`, so that your types work with operators and built-in
functions. Describe what you expect with `Protocol` and check it with mypy or pyright,
and use `abc.ABC` when subclasses must fill in methods. Keep mixins small, make every
`__init__` call `super().__init__()`, and mark overrides with `@typing.override` (Python
3.12).

**JavaScript: classes with `#` fields, and plain objects.** Use `class` with `#` private
fields for types that have invariants, and plain objects and functions for the rest.
Give the methods that stand in for operators the same names everywhere, such as
`equals`, `compare`, and `toString`. Prefer composition, and functions that return
objects, to long `extends` chains, and use mixin functions sparingly.

**TypeScript: interfaces for shapes, classes for behavior.** Describe data and plugin
points with `interface` or `type`, and let structural typing do the matching. Use
classes where there are invariants or state, with `#` fields if privacy must hold at run
time and `private readonly` otherwise. Mark overrides with `override`, and turn on
`noImplicitOverride` so the compiler insists on it.

**Dart: classes, interfaces, and mixins.** Use `final` fields and a constructor that
checks its input, with a `factory` constructor when it must do work first. Override `==`
and `hashCode` together, and implement `Comparable` for a natural order. Declare plugin
points as `abstract interface class`, and share code between unrelated classes with
`mixin`. Use the class modifiers, such as `final`, `base`, and `sealed`, to say how a
library's classes may be used.

**Kotlin: final classes, interfaces with defaults, and delegation.** Leave classes final
unless they are designed to be extended. Use `data class` for records, `object` for
single values, and `sealed` classes for a closed set of kinds. Put shared behavior in
interfaces with default methods, and reuse an implementation with `by` instead of
inheriting from it. Check constructor arguments with `require` in `init`.

---

## 12.11 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Bank accounts](exercises/01-bank-accounts/) | A base class whose steps each kind fills in, and the same design without inheritance |
| 2 | [Plugins](exercises/02-plugins/) | An interface with a default method, a registry, and plugins with and without settings |
| 3 | [Undo and redo](exercises/03-undo/) | The command pattern: edits as objects that apply and undo themselves |

```sh
python3 tools/check.py 12                     # your answers, every language
python3 tools/check.py 12 --lang rust,go      # some languages
python3 tools/check.py 12 --solutions         # the reference answers
```

**Stretch:** add the `student` account from exercise 1's stretch in two languages, one
with inheritance, such as Kotlin or Python, and one without, such as Rust or Go. Compare
what had to change in each.

---

Previous: [Chapter 11: Error handling](../11-error-handling/) ·
Next: [Chapter 13: Generics and polymorphism](../13-generics-and-polymorphism/)
