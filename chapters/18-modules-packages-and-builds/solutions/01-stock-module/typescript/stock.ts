// The stock module: how many of each item there are.

export class Stock {
  // # makes the field private when the program runs, as in JavaScript. TypeScript's
  // own `private` would only stop other files at compile time.
  #counts = new Map<string, number>();

  add(item: string, amount: number): void {
    this.#counts.set(item, this.count(item) + amount);
  }

  // Takes amount of item away, and returns true, if there are that many.
  take(item: string, amount: number): boolean {
    if (this.count(item) < amount) return false;
    this.#counts.set(item, this.count(item) - amount);
    return true;
  }

  // How many of item there are: 0 for an item never added.
  count(item: string): number {
    return this.#counts.get(item) ?? 0;
  }

  // The items there are any of, as [name, count] pairs, in alphabetical order.
  items(): [string, number][] {
    return [...this.#counts]
      .filter(([, count]) => count > 0)
      .sort(([a], [b]) => (a < b ? -1 : a > b ? 1 : 0));
  }
}
