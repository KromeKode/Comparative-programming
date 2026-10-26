function describe(code: number): string {
  // switch compares with ===, and falls through like C without a break or return.
  switch (code) {
    case 200:
      return "ok";
    case 301:
    case 302:
    case 307:
    case 308:
      return "redirect";
    case 404:
      return "not found";
  }
  if (code >= 400 && code <= 499) {
    return "client error";
  }
  if (code >= 500 && code <= 599) {
    return "server error";
  }
  return "unknown";
}

for (const code of [200, 301, 308, 404, 418, 500, 503, 99]) {
  console.log(`${code}: ${describe(code)}`);
}
