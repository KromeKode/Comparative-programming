// The textstats package's public library. Other packages import the files in
// lib/ with package:textstats/..., and a name that starts with an underscore
// is private to its file.

const version = '1.0';

final _word = RegExp('[A-Za-z]+');

// Private: only code in this file can call it.
Iterable<String> _words(String text) => _word.allMatches(text).map((m) => m[0]!);

/// The number of words in [text]. A word is a run of ASCII letters.
int wordCount(String text) => _words(text).length;

/// The first of the longest words in [text].
String longestWord(String text) =>
    _words(text).fold('', (best, word) => word.length > best.length ? word : best);
