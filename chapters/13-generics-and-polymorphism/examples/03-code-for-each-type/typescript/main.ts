class Point {
  readonly x: number;
  readonly y: number;

  constructor(x: number, y: number) {
    this.x = x;
    this.y = y;
  }
}

// The types are gone when the program runs, so one function checks what it
// was given. The union type lists everything it knows how to show.
type Showable = number | boolean | string | Point | Showable[];

function show(value: Showable): string {
  if (typeof value === "string") return `"${value}"`;
  if (value instanceof Point) return `(${value.x}, ${value.y})`;
  if (Array.isArray(value)) return `[${value.map(show).join(", ")}]`;
  return String(value);
}

console.log(show(42));
console.log(show("hi"));
console.log(show(true));
console.log(show([1, 2, 3]));
console.log(show([["a", "b"], [], ["c"]]));
console.log(show(new Point(1, 2)));
console.log(show([new Point(1, 2), new Point(3, 4)]));
