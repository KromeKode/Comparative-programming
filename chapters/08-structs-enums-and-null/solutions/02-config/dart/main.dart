import 'dart:io';

// null means "not set", which is different from every real value.
class Config {
  String? host;
  int? port;
  int? timeout;
}

final digitsOnly = RegExp(r'^[0-9]+$');

// The number if text is digits only and from 1 to high, or else null.
int? parseNumber(String text, int high) {
  if (!digitsOnly.hasMatch(text)) return null;
  // tryParse gives null when the number is too big for an int.
  final value = int.tryParse(text);
  return value != null && value >= 1 && value <= high ? value : null;
}

void main() {
  final config = Config();
  var number = 0;
  while (true) {
    final text = stdin.readLineSync();
    if (text == null) break;
    number++;
    final line = text.trim();
    if (line.isEmpty || line.startsWith('#')) continue;
    final equals = line.indexOf('=');
    if (equals == -1) {
      print('line $number: expected key = value');
      continue;
    }
    final key = line.substring(0, equals).trim();
    final value = line.substring(equals + 1).trim();
    switch (key) {
      case 'host':
        config.host = value.isEmpty ? null : value;
      case 'port' || 'timeout':
        final parsed = parseNumber(value, key == 'port' ? 65535 : 86400);
        if (value.isNotEmpty && parsed == null) {
          print('line $number: bad $key "$value"');
        } else if (key == 'port') {
          config.port = parsed;
        } else {
          config.timeout = parsed;
        }
      default:
        print('line $number: unknown key "$key"');
    }
  }

  print('host: ${config.host ?? 'localhost (default)'}');
  print('port: ${config.port ?? '8080 (default)'}');
  final timeout = config.timeout;
  print('timeout: ${timeout == null ? 'none' : '${timeout}s'}');
}
