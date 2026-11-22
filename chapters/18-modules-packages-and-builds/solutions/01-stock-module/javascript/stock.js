// The stock module: how many of each item there are.

export class Stock {
  // A field whose name starts with # is private: only code inside the class can
  // use it, and stock.#counts anywhere else is a syntax error.
  #counts = new Map();

  add(item, amount) {
    this.#counts.set(item, this.count(item) + amount);
  }

  // Takes amount of item away, and returns true, if there are that many.
  take(item, amount) {
    if (this.count(item) < amount) return false;
    this.#counts.set(item, this.count(item) - amount);
    return true;
  }

  // How many of item there are: 0 for an item never added.
  count(item) {
    return this.#counts.get(item) ?? 0;
  }

  // The items there are any of, as [name, count] pairs, in alphabetical order.
  items() {
    return [...this.#counts]
      .filter(([, count]) => count > 0)
      .sort(([a], [b]) => (a < b ? -1 : a > b ? 1 : 0));
  }
}
