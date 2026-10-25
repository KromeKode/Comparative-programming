import { readFileSync } from "node:fs";

function toFahrenheit(celsius: number): number {
  return (celsius * 9) / 5 + 32;
}

function toCelsius(fahrenheit: number): number {
  return ((fahrenheit - 32) * 5) / 9;
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  const parts: string[] = line.trim().split(/\s+/);
  if (parts.length !== 2) {
    continue; // a blank line
  }
  const value: number = Number(parts[0]);
  if (parts[1] === "C") {
    console.log(`${value.toFixed(1)} C = ${toFahrenheit(value).toFixed(1)} F`);
  } else {
    console.log(`${value.toFixed(1)} F = ${toCelsius(value).toFixed(1)} C`);
  }
}
