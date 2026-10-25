# Exercise 1.1: Greet

Read one line from stdin. It holds a name.

- Trim spaces, tabs, and the line ending (`\n` or `\r\n`) from both ends.
- If anything is left, print `Hello, <name>!`.
- If nothing is left, or there is no input at all, print `Hello, stranger!`.
- Ignore any lines after the first.

| Input | Output |
| --- | --- |
| `Ada` | `Hello, Ada!` |
| `  Grace Hopper  ` | `Hello, Grace Hopper!` |
| *(empty)* | `Hello, stranger!` |

Start from the file for your language in this folder, for example `rust/main.rs`.
Section 1.5 of the chapter shows how to read a line in each language.

```sh
python3 tools/check.py 01 --lang rust
```

**Things to notice:** which languages have a built-in "trim both ends" function, and
which make you write the loop yourself? What does each language do with the name
`Zoë`, which is 3 characters but 4 bytes?
