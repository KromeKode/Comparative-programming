import { readFileSync } from "node:fs";

const MAX = 2 ** 31 - 1;
const MIN = -(2 ** 31);

// Every way parseInt32 can fail, as a union of tagged objects. A switch on
// kind must handle every case, or the function that formats it fails to
// compile.
type ParseError =
  | { kind: "empty" }
  | { kind: "no digits" }
  | { kind: "invalid character"; character: string; position: number }
  | { kind: "too large" }
  | { kind: "too small" };

type ParseResult =
  | { ok: true; value: number }
  | { ok: false; error: ParseError };

function describe(error: ParseError): string {
  switch (error.kind) {
    case "empty":
      return "empty input";
    case "no digits":
      return "no digits after the sign";
    case "invalid character":
      return `invalid character '${error.character}' at position ${error.position}`;
    case "too large":
      return `too large, the maximum is ${MAX}`;
    case "too small":
      return `too small, the minimum is ${MIN}`;
  }
}

function fail(error: ParseError): ParseResult {
  return { ok: false, error };
}

function parseInt32(text: string): ParseResult {
  if (text === "") {
    return fail({ kind: "empty" });
  }
  const negative = text[0] === "-";
  const start = text[0] === "+" || text[0] === "-" ? 1 : 0;
  if (start === text.length) {
    return fail({ kind: "no digits" });
  }
  // Check every character first, so a bad one is reported even when the
  // digits before it are already too large.
  for (let i = start; i < text.length; i++) {
    if (text[i] < "0" || text[i] > "9") {
      return fail({
        kind: "invalid character",
        character: text[i],
        position: i + 1,
      });
    }
  }
  // Stop growing once the number is past every limit. A number is exact only
  // up to 2 ** 53, so it must not grow past that.
  let magnitude = 0;
  for (let i = start; i < text.length; i++) {
    magnitude = Math.min(magnitude * 10 + Number(text[i]), MAX + 2);
  }
  if (!negative && magnitude > MAX) {
    return fail({ kind: "too large" });
  }
  if (negative && -magnitude < MIN) {
    return fail({ kind: "too small" });
  }
  return { ok: true, value: negative ? -magnitude : magnitude };
}

const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") {
  lines.pop();
}
for (const line of lines) {
  const result = parseInt32(line);
  if (result.ok) {
    console.log(`"${line}" -> ${result.value}`);
  } else {
    console.log(`"${line}" -> error: ${describe(result.error)}`);
  }
}
