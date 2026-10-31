typedef Row = ({String name, int score});

// Outside this file, an abstract interface class can only be implemented: not
// extended, and never made directly. Every Dart class also defines an
// interface of its own, so `implements` works with any class.
abstract interface class Exporter {
  String get name;
  String export(List<Row> rows);
}

class CsvExporter implements Exporter {
  final String separator;
  CsvExporter(this.separator);

  @override
  String get name => 'csv';

  @override
  String export(List<Row> rows) => [
    'name${separator}score',
    for (final row in rows) '${row.name}$separator${row.score}',
  ].join('\n');
}

class JsonExporter implements Exporter {
  @override
  String get name => 'json';

  @override
  String export(List<Row> rows) {
    final items = [
      for (final row in rows) '{"name":"${row.name}","score":${row.score}}',
    ];
    return '[${items.join(',')}]';
  }
}

class MarkdownExporter implements Exporter {
  @override
  String get name => 'markdown';

  @override
  String export(List<Row> rows) => [
    '| name | score |',
    '| --- | --- |',
    for (final row in rows) '| ${row.name} | ${row.score} |',
  ].join('\n');
}

class SummaryExporter implements Exporter {
  @override
  String get name => 'summary';

  @override
  String export(List<Row> rows) {
    final best = rows.reduce((a, b) => b.score > a.score ? b : a);
    return '${rows.length} rows, top score ${best.score} by ${best.name}';
  }
}

void run(Exporter exporter, List<Row> rows) {
  print('== ${exporter.name}');
  print(exporter.export(rows));
}

void main() {
  final rows = <Row>[
    (name: 'ada', score: 90),
    (name: 'grace', score: 85),
    (name: 'alan', score: 78),
  ];
  final exporters = <Exporter>[
    CsvExporter(','),
    JsonExporter(),
    MarkdownExporter(),
    SummaryExporter(),
  ];
  for (final exporter in exporters) {
    run(exporter, rows);
  }
}
