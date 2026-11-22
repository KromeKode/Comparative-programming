// A record type. Records are the one place where Dart writes the code for
// you: two records are == when their fields are, they have a matching
// hashCode, and toString lists the fields.
typedef Book = ({String title, String author, int year, bool inPrint});

int compareBool(bool a, bool b) => a == b ? 0 : (a ? 1 : -1);

// Dart writes no ordering, and has no reflection outside the Dart VM, so
// this compares the fields one by one.
int compareBooks(Book a, Book b) {
  for (final order in [
    a.title.compareTo(b.title),
    a.author.compareTo(b.author),
    a.year.compareTo(b.year),
    compareBool(a.inPrint, b.inPrint),
  ]) {
    if (order != 0) return order;
  }
  return 0;
}

void main() {
  const Book dune = (
    title: 'Dune',
    author: 'Frank Herbert',
    year: 1965,
    inPrint: true,
  );
  print('dune: $dune');

  const Book copy = (
    title: 'Dune',
    author: 'Frank Herbert',
    year: 1965,
    inPrint: true,
  );
  print('copy == dune: ${copy == dune}');

  // A record has no copy method, so a changed copy names every field.
  final Book reissue = (
    title: dune.title,
    author: dune.author,
    year: 1966,
    inPrint: dune.inPrint,
  );
  print('reissue: $reissue');
  print('reissue == dune: ${reissue == dune}');
  print('dune < reissue: ${compareBooks(dune, reissue) < 0}');

  print('distinct books: ${[dune, copy, reissue].toSet().length}');
}
