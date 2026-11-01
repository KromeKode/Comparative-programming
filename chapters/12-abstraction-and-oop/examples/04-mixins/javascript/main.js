// A mixin in JavaScript is often a function that takes a class and returns a
// subclass of it with more methods. Describe needs only this.name.
const Describe = (Base) =>
  class extends Base {
    describe() {
      return `this is the ${this.name}`;
    }
  };

// A mixin made this way can have fields, including private ones.
const Logged = (Base) =>
  class extends Base {
    #history = [];

    log(event) {
      this.#history.push(`${this.name}: ${event}`);
    }

    get history() {
      return [...this.#history];
    }
  };

class Named {
  constructor(name) {
    this.name = name;
  }
}

// Door and Light each build their own chain: Named, then Describe, then Logged.
class Door extends Logged(Describe(Named)) {
  open() {
    this.log("opened");
  }

  close() {
    this.log("closed");
  }
}

class Light extends Logged(Describe(Named)) {
  on = false;

  switchOn() {
    this.on = true;
    this.log("switched on");
  }

  // Light replaces the default describe, and reuses it.
  describe() {
    return `${super.describe()}, and it is ${this.on ? "on" : "off"}`;
  }
}

const door = new Door("front door");
const light = new Light("porch light");
door.open();
door.close();
light.switchOn();

for (const thing of [door, light]) {
  console.log(thing.describe());
  for (const line of thing.history) {
    console.log(`  ${line}`);
  }
}
