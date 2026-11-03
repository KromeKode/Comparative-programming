import { readFileSync } from "node:fs";

interface Grade {
  name: string;
  subject: string;
  score: number;
}

const sum = (grades: Grade[]): number =>
  grades.reduce((total, g) => total + g.score, 0);
const byText = (a: string, b: string): number => (a < b ? -1 : a > b ? 1 : 0);

function report(grades: Grade[]): string[] {
  const lines = ["subjects:"];
  // Map.groupBy (ES2024) groups any iterable by a key into a Map of arrays.
  const subjects = Map.groupBy(grades, (g) => g.subject);
  for (const [subject, group] of [...subjects].toSorted(([a], [b]) =>
    byText(a, b),
  )) {
    const best = Math.max(...group.map((g) => g.score));
    const names = group.filter((g) => g.score === best).map((g) => g.name);
    const average = Math.floor(sum(group) / group.length);
    lines.push(
      `  ${subject}: average ${average}, best ${best} (${names.toSorted().join(", ")})`,
    );
  }

  const students = Map.groupBy(grades, (g) => g.name);
  const ranked = [...students]
    .map(([name, group]) => ({ name, total: sum(group) }))
    .toSorted((a, b) => b.total - a.total || byText(a.name, b.name));
  lines.push(
    "ranking:",
    ...ranked.map(({ name, total }, i) => `  ${i + 1}. ${name} ${total}`),
  );

  const passed = [...students]
    .filter(([, group]) => group.every((g) => g.score >= 50))
    .map(([name]) => name)
    .toSorted();
  lines.push(`passed everything: ${passed.join(", ") || "nobody"}`);
  return lines;
}

const grades = readFileSync(0, "utf8")
  .split("\n")
  .map((line) => line.split(/\s+/).filter(Boolean))
  .filter((words) => words.length > 0)
  .map(
    ([name, subject, score]): Grade => ({
      name,
      subject,
      score: Number(score),
    }),
  );
console.log(report(grades).join("\n"));
