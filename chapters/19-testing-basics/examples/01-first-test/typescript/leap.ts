// Whether year is a leap year in the Gregorian calendar: one divisible by 4,
// except for centuries, which must be divisible by 400.
export function isLeapYear(year: number): boolean {
  return year % 4 === 0 && (year % 100 !== 0 || year % 400 === 0);
}
