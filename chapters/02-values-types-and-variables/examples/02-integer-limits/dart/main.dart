void main() {
  var i32Max = 2147483647;
  print('int32 max: $i32Max');
  print('int32 max + 1: ${i32Max + 1}'); // Dart's int is 64 bits, so this still fits
  print('int32 max + 1, as 32 bits: ${(i32Max + 1).toSigned(32)}');

  var i64Max = 9223372036854775807;
  print('int64 max: $i64Max');
  print('int64 max + 1: ${i64Max + 1}'); // native Dart wraps silently
}
