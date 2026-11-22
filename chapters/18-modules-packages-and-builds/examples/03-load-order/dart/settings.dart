// A top-level variable is set the first time it is read, not when the program
// starts, so a library costs nothing until it is used.
final greeting = _load();

String _load() {
  print('settings: loading');
  return 'Hello';
}
