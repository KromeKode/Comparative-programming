import { createInterface } from "node:readline";

class PluginError extends Error {}

// A plugin is any object with a name and an apply method. It may also have a
// describe method. JavaScript has no interfaces, so the default lives here:
// if the plugin has no describe, use its name.
function describe(plugin) {
  return plugin.describe?.() ?? plugin.name;
}

// Plugins with no settings can be plain objects.
const upper = {
  name: "upper",
  apply: (words) => words.map((word) => word.toUpperCase()),
};

const lower = {
  name: "lower",
  apply: (words) => words.map((word) => word.toLowerCase()),
};

const reverse = {
  name: "reverse",
  apply: (words) => words.toReversed(),
};

// A plugin with settings can be made by a function, and the object it returns
// keeps the settings in a closure.
function censor(hidden) {
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

function noArguments(plugin) {
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
  #entries = new Map();

  register(name, entry) {
    this.#entries.set(name, entry);
  }

  create(name, args) {
    const entry = this.#entries.get(name);
    if (entry === undefined) {
      throw new PluginError(`no plugin named ${name}`);
    }
    return entry.make(args);
  }

  entries() {
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
    return censor(args[0]);
  },
});

const pipeline = [];
for await (const line of createInterface({ input: process.stdin })) {
  const [command, ...args] = line.split(/\s+/).filter((word) => word !== "");
  if (command === "list" && args.length === 0) {
    for (const entry of registry.entries()) {
      console.log(`${entry.usage}: ${entry.help}`);
    }
  } else if (command === "add" && args.length > 0) {
    try {
      const plugin = registry.create(args[0], args.slice(1));
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
