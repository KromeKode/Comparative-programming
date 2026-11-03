typedef Score = ({String name, int points});

const List<Score> scores = [
  (name: 'ada', points: 91),
  (name: 'bob', points: 78),
  (name: 'cy', points: 85),
  (name: 'dee', points: 62),
  (name: 'eve', points: 99),
  (name: 'finn', points: 54),
];

String yesNo(bool value) => value ? 'yes' : 'no';

void main() {
  // where and map are lazy: they return an Iterable that does the work only
  // when something, here join, loops over it.
  final passes = scores.where((score) => score.points >= 70);
  print('passed: ${passes.map((score) => score.name).join(', ')}');

  // fold combines the items into one value, starting at 0.
  final total = passes.fold(0, (sum, score) => sum + score.points);
  print('total of the passes: $total');

  // reduce is fold without a starting value: it starts from the first item.
  final best = scores.reduce((a, b) => b.points > a.points ? b : a);
  print('best: ${best.name} with ${best.points}');
  print('everyone passed: ${yesNo(scores.every((s) => s.points >= 70))}');
  print('anyone above 95: ${yesNo(scores.any((s) => s.points > 95))}');

  // sort works in place, so sort a copy. indexed pairs each item with its index.
  final ranked = [...scores]..sort((a, b) => b.points.compareTo(a.points));
  final top = ranked
      .take(3)
      .indexed
      .map((entry) => '${entry.$1 + 1}. ${entry.$2.name} ${entry.$2.points}');
  print('top three: ${top.join(', ')}');

  // expand turns each name into its letters, and a Set keeps one of each.
  final letters =
      scores.expand((score) => score.name.split('')).toSet().toList()..sort();
  print('letters used: ${letters.join(' ')}');
}
