import { createInterface } from "node:readline";

class SettingError extends Error {}

// One setting. JavaScript has no T to fill in, but each setting still knows
// how to read its value from text and how to show it. === can't compare two
// arrays by their items, so a setting can bring its own equal.
class Setting {
  #default;
  #value;
  #parse;
  #show;
  #needs;
  #equal;

  constructor(
    name,
    defaultValue,
    parse,
    show,
    needs,
    equal = (a, b) => a === b,
  ) {
    this.name = name;
    this.#default = defaultValue;
    this.#value = defaultValue;
    this.#parse = parse;
    this.#show = show;
    this.#needs = needs;
    this.#equal = equal;
  }

  get value() {
    return this.#value;
  }

  setText(text) {
    const value = this.#parse(text);
    if (value === undefined) {
      throw new SettingError(
        `${this.name} needs ${this.#needs}, not "${text}"`,
      );
    }
    this.#value = value;
  }

  reset() {
    this.#value = this.#default;
  }

  describe() {
    return `${this.name} = ${this.#show(this.#value)}`;
  }

  isDefault() {
    return this.#equal(this.#value, this.#default);
  }
}

function parseInt(text) {
  if (!/^-?[0-9]+$/.test(text)) return undefined;
  const value = Number(text);
  return value >= -(2 ** 31) && value < 2 ** 31 ? value : undefined;
}

function parseBool(text) {
  return text === "true" ? true : text === "false" ? false : undefined;
}

const sameItems = (a, b) =>
  a.length === b.length && a.every((item, i) => item === b[i]);

// Named handles: the summary reads volume.value and muted.value directly.
const volume = new Setting("volume", 5, parseInt, String, "a whole number");
const muted = new Setting("muted", false, parseBool, String, "true or false");
const name = new Setting(
  "name",
  "guest",
  (text) => text,
  (text) => text,
  "",
);
const tags = new Setting(
  "tags",
  [],
  (text) => text.split(" "),
  (tags) => `[${tags.join(", ")}]`,
  "",
  sameItems,
);
const all = [volume, muted, name, tags];

function find(wanted) {
  const setting = all.find((setting) => setting.name === wanted);
  if (setting === undefined)
    throw new SettingError(`no setting named "${wanted}"`);
  return setting;
}

function summary() {
  if (muted.value) return `${name.value} has muted the sound`;
  return `${name.value} hears the sound at ${volume.value * 10}%`;
}

function run(words) {
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
