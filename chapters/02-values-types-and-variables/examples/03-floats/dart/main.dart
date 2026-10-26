void main() {
  final sum = 0.1 + 0.2;

  print('0.1 + 0.2 = $sum');
  print('equal to 0.3? ${sum == 0.3 ? 'yes' : 'no'}');
  print('close to 0.3? ${(sum - 0.3).abs() < 1e-9 ? 'yes' : 'no'}');
}
