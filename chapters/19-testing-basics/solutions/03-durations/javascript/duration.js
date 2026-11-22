// Durations such as 1h30m: parseDuration reads one, and formatDuration writes one.

const UNITS = "dhms"; // largest first, which is the order they come in
const SECONDS = [86400, 3600, 60, 1]; // in each of the units

/**
 * The number of seconds that text says, such as 5400 for "1h30m". Throws a
 * SyntaxError, as JSON.parse does, if text isn't a duration.
 */
export function parseDuration(text) {
  if (text === "") {
    throw new SyntaxError("a duration can't be empty");
  }
  let seconds = 0;
  let nextUnit = 0; // each part's unit comes after the one before it in UNITS
  let i = 0;
  while (i < text.length) {
    const start = i;
    while (i < text.length && text[i] >= "0" && text[i] <= "9") {
      i++;
    }
    if (i === start) {
      throw new SyntaxError(`"${text}" has a part with no number`);
    }
    if (i === text.length) {
      throw new SyntaxError(`"${text}" ends with a number that has no unit`);
    }
    const number = Number(text.slice(start, i));
    const unit = UNITS.indexOf(text[i]);
    if (unit === -1) {
      throw new SyntaxError(`"${text[i]}" isn't a unit`);
    }
    if (unit < nextUnit) {
      throw new SyntaxError(`"${text}" has a unit out of order, or twice`);
    }
    seconds += number * SECONDS[unit];
    if (!Number.isSafeInteger(seconds)) {
      throw new RangeError(`"${text}" is too long to count exactly`);
    }
    nextUnit = unit + 1;
    i++;
  }
  return seconds;
}

/** seconds as a duration, such as "1h30m" for 5400, leaving out the units that are 0. */
export function formatDuration(seconds) {
  if (seconds === 0) {
    return "0s";
  }
  let text = "";
  for (let unit = 0; unit < UNITS.length; unit++) {
    const amount = Math.floor(seconds / SECONDS[unit]);
    seconds %= SECONDS[unit];
    if (amount > 0) {
      text += `${amount}${UNITS[unit]}`;
    }
  }
  return text;
}
