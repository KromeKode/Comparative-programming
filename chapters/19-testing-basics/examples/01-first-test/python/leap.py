def is_leap_year(year: int) -> bool:
    """Whether year is a leap year in the Gregorian calendar: one divisible by 4,
    except for centuries, which must be divisible by 400."""
    return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)
