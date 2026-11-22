/** The computer's clock, which counts the seconds since 1970. */
export class SystemClock {
  now() {
    return Math.floor(Date.now() / 1000);
  }
}

/**
 * Keeps each value for ttl seconds after it was put, and then forgets it. The clock
 * is anything with a now() method that gives the time in whole seconds.
 */
export class Cache {
  #ttl;
  #clock;
  #entries = new Map(); // from each key to {value, expires}

  constructor(ttl, clock) {
    this.#ttl = ttl;
    this.#clock = clock;
  }

  /** Stores value under key for the next ttl seconds, replacing any value there. */
  put(key, value) {
    const now = this.#clock.now();
    for (const [k, entry] of this.#entries) {
      if (!isLive(entry, now)) {
        this.#entries.delete(k);
      }
    }
    this.#entries.set(key, { value, expires: now + this.#ttl });
  }

  /** The value under key, or undefined if there is none or it has expired. */
  get(key) {
    const entry = this.#entries.get(key);
    if (entry === undefined || !isLive(entry, this.#clock.now())) {
      return undefined;
    }
    return entry.value;
  }

  /** How many entries haven't expired. */
  get size() {
    const now = this.#clock.now();
    let live = 0;
    for (const entry of this.#entries.values()) {
      if (isLive(entry, now)) {
        live++;
      }
    }
    return live;
  }
}

function isLive(entry, now) {
  return now < entry.expires;
}
