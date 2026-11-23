/** Something that tells the time, in whole seconds. */
export interface Clock {
  now(): number;
}

/** The computer's clock, which counts the seconds since 1970. */
export class SystemClock implements Clock {
  now(): number {
    return Math.floor(Date.now() / 1000);
  }
}

interface Entry {
  value: string;
  expires: number; // the time the entry stops being live
}

/** Keeps each value for ttl seconds after it was put, and then forgets it. */
export class Cache {
  #ttl: number;
  #clock: Clock;
  #entries = new Map<string, Entry>();

  constructor(ttl: number, clock: Clock) {
    this.#ttl = ttl;
    this.#clock = clock;
  }

  /** Stores value under key for the next ttl seconds, replacing any value there. */
  put(key: string, value: string): void {
    const now = this.#clock.now();
    for (const [k, entry] of this.#entries) {
      if (!isLive(entry, now)) {
        this.#entries.delete(k);
      }
    }
    this.#entries.set(key, { value, expires: now + this.#ttl });
  }

  /** The value under key, or undefined if there is none or it has expired. */
  get(key: string): string | undefined {
    const entry = this.#entries.get(key);
    if (entry === undefined || !isLive(entry, this.#clock.now())) {
      return undefined;
    }
    return entry.value;
  }

  /** How many entries haven't expired. */
  get size(): number {
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

function isLive(entry: Entry, now: number): boolean {
  return now < entry.expires;
}
