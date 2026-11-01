import { createInterface } from "node:readline";

class PluginError extends Error {}

// Every plugin has a name and changes a list of words. A TypeScript interface
// cannot hold code, so describe is optional ("?") and the default lives in
// the describe function below.
interface Plugin {
  readonly name: string;
  apply(words: readonly string[]): string[];
  describe?(): string;
}

function describe(plugin: Plugin): string {
  return plugin.describe?.() ?? plugin.name;
}

// Plugins with no settings can be plain objects. TypeScript checks each one
// against Plugin because of the type annotation.
const upper: Plugin = {
  name: "upper",
  apply: (words) => words.map((word) => word.toUpperCase()),
};

const lower: Plugin = {
  name: "lower",
  apply: (words) => words.map((word) => word.toLowerCase()),
};

const reverse: Plugin = {
  name: "reverse",
  apply: (words) => words.toReversed(),
};

// A plugin with settings can be made by a function, and the object it returns
// keeps the settings in a closure.
function censor(hidden: string): Plugin {
  return {
    name: "censor",
    apply: (words) =>
      words.map((word) =>
        word.toLowerCase() === hidden.toLowerCase()
          ? "*".repeat(word.length)
          : word,
      ),
    describe: () => `censor(${hidden})`,
  };
}

interface Entry {
  usage: string;
  help: string;
  make(args: readonly string[]): Plugin;
}

function noArguments(plugin: Plugin): Entry["make"] {
  return (args) => {
    if (args.length > 0) {
      throw new PluginError(`${plugin.name} takes no arguments`);
    }
    return plugin;
  };
}

// Maps a plugin's name to a way to make one. The rest of the program finds
// plugins only through here. A Map keeps its keys in the order they were added.
class Registry {
  readonly #entries = new Map<string, Entry>();

  register(name: string, entry: Entry): void {
    this.#entries.set(name, entry);
  }

  create(name: string, args: readonly string[]): Plugin {
    const entry = this.#entries.get(name);
    if (entry === undefined) {
      throw new PluginError(`no plugin named ${name}`);
    }
    return entry.make(args);
  }

  entries(): Entry[] {
    return [...this.#entries.values()];
  }
}

const registry = new Registry();
registry.register("upper", {
  usage: "upper",
  help: "make every letter uppercase",
  make: noArguments(upper),
});
registry.register("lower", {
  usage: "lower",
  help: "make every letter lowercase",
  make: noArguments(lower),
});
registry.register("reverse", {
  usage: "reverse",
  help: "reverse the order of the words",
  make: noArguments(reverse),
});
registry.register("censor", {
  usage: "censor WORD",
  help: "hide WORD wherever it appears",
  make: (args) => {
    if (args.length !== 1) {
      throw new PluginError("censor needs exactly one word");
    }
    return censor(args[0]!);
  },
});

const pipeline: Plugin[] = [];
for await (const line of createInterface({ input: process.stdin })) {
  const [command, ...args] = line.split(/\s+/).filter((word) => word !== "");
  if (command === "list" && args.length === 0) {
    for (const entry of registry.entries()) {
      console.log(`${entry.usage}: ${entry.help}`);
    }
  } else if (command === "add" && args.length > 0) {
    try {
      const plugin = registry.create(args[0]!, args.slice(1));
      pipeline.push(plugin);
      console.log(`added ${describe(plugin)}`);
    } catch (error) {
      if (!(error instanceof PluginError)) {
        throw error;
      }
      console.log(`error: ${error.message}`);
    }
  } else if (command === "pipeline" && args.length === 0) {
    const steps = pipeline.map(describe).join(" | ");
    console.log(`pipeline: ${steps || "(empty)"}`);
  } else if (command === "run") {
    let words = args;
    for (const plugin of pipeline) {
      words = plugin.apply(words);
    }
    console.log(words.join(" "));
  }
}
