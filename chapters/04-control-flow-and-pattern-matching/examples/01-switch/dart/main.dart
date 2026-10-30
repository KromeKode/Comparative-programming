// A switch expression (Dart 3) gives a value. Cases never fall through.
String describe(int code) => switch (code) {
  200 => 'ok',
  301 || 302 || 307 || 308 => 'redirect',
  // Cases are tried from top to bottom, so 404 must come before the range.
  404 => 'not found',
  >= 400 && <= 499 => 'client error',
  >= 500 && <= 599 => 'server error',
  _ => 'unknown',
};

void main() {
  for (final code in [200, 301, 308, 404, 418, 500, 503, 99]) {
    print('$code: ${describe(code)}');
  }
}
