const NUMERALS = [
  [1000, "M"],
  [900, "CM"],
  [500, "D"],
  [400, "CD"],
  [100, "C"],
  [90, "XC"],
  [50, "L"],
  [40, "XL"],
  [10, "X"],
  [9, "IX"],
  [5, "V"],
  [4, "IV"],
  [1, "I"],
];

// n in Roman numerals. Throws a RangeError if n is outside 1 to 3999.
export function toRoman(n) {
  if (n < 1 || n > 3999) {
    throw new RangeError(`${n} is out of range`);
  }
  let numeral = "";
  for (const [value, letters] of NUMERALS) {
    for (; n >= value; n -= value) numeral += letters;
  }
  return numeral;
}
