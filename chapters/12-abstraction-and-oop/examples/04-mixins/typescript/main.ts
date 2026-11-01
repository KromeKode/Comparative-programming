// A class whose constructor takes any arguments and whose objects are a T.
type Constructor<T = object> = new (...args: any[]) => T;

// A mixin is a function that takes a class and returns a subclass of it. The
// type of Base says what the mixin needs: here, a name.
function Describe<TBase extends Constructor<{ name: string }>>(Base: TBase) {
  return class extends Base {
    describe(): string {
      return `this is the ${this.name}`;
    }
  };
}

function Logged<TBase extends Constructor<{ name: string }>>(Base: TBase) {
  return class extends Base {
    #history: string[] = [];

    log(event: string): void {
      this.#history.push(`${this.name}: ${event}`);
    }

    get history(): readonly string[] {
      return this.#history;
    }
  };
}

class Named {
  readonly name: string;

  constructor(name: string) {
    this.name = name;
  }
}

// Door and Light each build their own chain: Named, then Describe, then Logged.
class Door extends Logged(Describe(Named)) {
  open(): void {
    this.log("opened");
  }

  close(): void {
    this.log("closed");
  }
}

class Light extends Logged(Describe(Named)) {
  on = false;

  switchOn(): void {
    this.on = true;
    this.log("switched on");
  }

  // Light replaces the default describe, and reuses it.
  override describe(): string {
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
