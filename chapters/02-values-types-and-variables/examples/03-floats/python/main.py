import math

total = 0.1 + 0.2

print("0.1 + 0.2 =", total)
print("equal to 0.3?", "yes" if total == 0.3 else "no")
print("close to 0.3?", "yes" if math.isclose(total, 0.3) else "no")
