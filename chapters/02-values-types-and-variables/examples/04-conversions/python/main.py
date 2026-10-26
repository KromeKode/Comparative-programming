parsed = int("42")
print('"42" + 1 as a number:', parsed + 1)

x = 3.99
print("3.99 truncated:", int(x))  # int() drops the fraction
print("-3.99 truncated:", int(-x))
print("2.5 rounded:", round(2.5))  # 2: halfway rounds to the even neighbor
print("-2.5 rounded:", round(-2.5))

print("7 / 2 as integers:", 7 // 2)  # // is integer (floor) division
print("7 / 2 as floats:", 7 / 2)  # / always gives a float
