// Package leap says which years are leap years.
package leap

// IsLeapYear reports whether year is a leap year in the Gregorian calendar: one
// divisible by 4, except for centuries, which must be divisible by 400.
func IsLeapYear(year int) bool {
	return year%4 == 0 && (year%100 != 0 || year%400 == 0)
}
