from typing import Final

MAX_PLAYERS: Final = 4  # Final is a hint for type checkers; Python itself does not enforce it

count = 0
birth_year = 1815  # no way to make a variable read-only
pi = 3.14159
ready = True

count += 3

print("count =", count)
print("birth year =", birth_year)
print("pi =", pi)
print("max players =", MAX_PLAYERS)
print("ready =", ready)  # prints True, with a capital T
