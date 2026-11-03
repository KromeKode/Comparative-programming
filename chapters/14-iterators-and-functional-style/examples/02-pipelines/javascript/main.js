const scores = [
  { name: "ada", points: 91 },
  { name: "bob", points: 78 },
  { name: "cy", points: 85 },
  { name: "dee", points: 62 },
  { name: "eve", points: 99 },
  { name: "finn", points: 54 },
];

const yesNo = (value) => (value ? "yes" : "no");

// Array methods are eager: filter builds a new array, then map builds another.
const passes = scores.filter((score) => score.points >= 70);
console.log(`passed: ${passes.map((score) => score.name).join(", ")}`);

// There is no sum, so reduce folds the array into one value, starting at 0.
const total = passes.reduce((sum, score) => sum + score.points, 0);
console.log(`total of the passes: ${total}`);

// There is no max by a key either, so reduce keeps the better of each pair.
const best = scores.reduce((a, b) => (b.points > a.points ? b : a));
console.log(`best: ${best.name} with ${best.points}`);
console.log(`everyone passed: ${yesNo(scores.every((s) => s.points >= 70))}`);
console.log(`anyone above 95: ${yesNo(scores.some((s) => s.points > 95))}`);

// toSorted returns a sorted copy; sort would change scores itself.
const top = scores
  .toSorted((a, b) => b.points - a.points)
  .slice(0, 3)
  .map((score, i) => `${i + 1}. ${score.name} ${score.points}`);
console.log(`top three: ${top.join(", ")}`);

// flatMap turns each name into its letters, and a Set keeps one of each.
const letters = [...new Set(scores.flatMap((score) => [...score.name]))].sort();
console.log(`letters used: ${letters.join(" ")}`);
