import { createInterface } from "node:readline";

class SettingError extends Error {}

// What the command loop needs from a setting, without knowing its T.
interface AnySetting {
  readonly name: string;
  setText(text: string): void;
  reset(): void;
  describe(): string;
  isDefault(): boolean;
}

// One setting whose value is a T. It knows how to read a T from text and how
// to show one. === can't compare two arrays by their items, so a setting can
// bring its own equal.
class Setting<T> implements AnySetting {
  readonly name: string;
  readonly #default: T;
  #value: T;
  readonly #parse: (text: string) => T | undefined;
  readonly #show: (value: T) => string;
  readonly #needs: string;
  readonly #equal: (a: T, b: T) => boolean;

  constructor(
    name: string,
    defaultValue: T,
    parse: (text: string) => T | undefined,
    show: (value: T) => string,
    needs: string,
    equal: (a: T, b: T) => boolean = (a, b) => a === b,
  ) {
    this.name = name;
    this.#default = defaultValue;
    this.#value = defaultValue;
    this.#parse = parse;
    this.#show = show;
    this.#needs = needs;
    this.#equal = equal;
  }

  get value(): T {
    return this.#value;
  }

  setText(text: string): void {
    const value = this.#parse(text);
    if (value === undefined) {
      throw new SettingError(
        `${this.name} needs ${this.#needs}, not "${text}"`,
      );
    }
    this.#value = value;
  }

  reset(): void {
    this.#value = this.#default;
  }

  describe(): string {
    return `${this.name} = ${this.#show(this.#value)}`;
  }

  isDefault(): boolean {
    return this.#equal(this.#value, this.#default);
  }
}

function parseInt(text: string): number | undefined {
  if (!/^-?[0-9]+$/.test(text)) return undefined;
  const value = Number(text);
  return value >= -(2 ** 31) && value < 2 ** 31 ? value : undefined;
}

function parseBool(text: string): boolean | undefined {
  return text === "true" ? true : text === "false" ? false : undefined;
}

const sameItems = (a: string[], b: string[]) =>
  a.length === b.length && a.every((item, i) => item === b[i]);

// Typed handles: volume.value is a number and muted.value a boolean.
const volume = new Setting("volume", 5, parseInt, String, "a whole number");
const muted = new Setting("muted", false, parseBool, String, "true or false");
const name = new Setting(
  "name",
  "guest",
  (text) => text,
  (text) => text,
  "",
);
const tags = new Setting<string[]>(
  "tags",
  [],
  (text) => text.split(" "),
  (tags) => `[${tags.join(", ")}]`,
  "",
  sameItems,
);
const all: AnySetting[] = [volume, muted, name, tags];

function find(wanted: string): AnySetting {
  const setting = all.find((setting) => setting.name === wanted);
  if (setting === undefined)
    throw new SettingError(`no setting named "${wanted}"`);
  return setting;
}

function summary(): string {
  if (muted.value) return `${name.value} has muted the sound`;
  return `${name.value} hears the sound at ${volume.value * 10}%`;
}

function run(words: string[]): void {
  const [command, wanted, ...value] = words;
  if (command === "set") {
    const setting = find(wanted);
    setting.setText(value.join(" "));
    console.log(setting.describe());
  } else if (command === "get") {
    console.log(find(wanted).describe());
  } else if (command === "reset") {
    const setting = find(wanted);
    setting.reset();
    console.log(setting.describe());
  } else if (command === "list") {
    for (const setting of all) {
      console.log(
        setting.describe() + (setting.isDefault() ? " (default)" : ""),
      );
    }
  } else if (command === "summary") {
    console.log(summary());
  }
}

for await (const line of createInterface({ input: process.stdin })) {
  try {
    run(line.split(/\s+/).filter((word) => word !== ""));
  } catch (error) {
    if (!(error instanceof SettingError)) throw error;
    console.log(`error: ${error.message}`);
  }
}
