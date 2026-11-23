// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
import { readFileSync } from "node:fs";

// ^ and $, because exec finds a match anywhere in the line. Without the m flag, $ matches
// only at the very end. In JavaScript, \d is only [0-9], even with the u flag, but this
// says so.
const LINE = new RegExp(
  String.raw`^(?<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?<time>[0-9]{2}:[0-9]{2}:[0-9]{2})` +
    String.raw` (?<method>GET|POST|PUT|DELETE) (?<path>/[^ ]*)` +
    String.raw` (?<status>[0-9]{3}) (?<ms>[0-9]{1,9})ms$`,
  "u",
);
const ID = /\/[0-9]+/gu; // replaceAll needs the g flag

// A Map keeps its keys in the order they went in.
const routes = new Map();
let status = 0;
const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // the input ended with a newline, or was empty
lines.forEach((text, index) => {
  const line = text.endsWith("\r") ? text.slice(0, -1) : text;
  const match = LINE.exec(line);
  if (match === null) {
    console.error(`logscan: line ${index + 1}: not a log line`);
    status = 1;
    return;
  }
  const { method, path, status: code, ms } = match.groups;
  const key = `${method} ${path.replaceAll(ID, "/:id")}`;
  let route = routes.get(key);
  if (route === undefined) {
    route = { requests: 0, failed: 0, slowest: 0 };
    routes.set(key, route);
  }
  route.requests += 1;
  if (Number(code) >= 400) route.failed += 1;
  route.slowest = Math.max(route.slowest, Number(ms));
});
for (const [key, { requests, failed, slowest }] of routes) {
  const plural = requests === 1 ? "" : "s";
  console.log(
    `${key}: ${requests} request${plural}, ${failed} failed, slowest ${slowest}ms`,
  );
}
process.exitCode = status;
