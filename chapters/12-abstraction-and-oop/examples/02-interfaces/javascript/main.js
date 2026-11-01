// JavaScript has no interfaces. Any object with name and exportRows methods
// can be used as an exporter: this is called duck typing.
class CsvExporter {
  constructor(separator) {
    this.separator = separator;
  }

  name() {
    return "csv";
  }

  exportRows(rows) {
    const sep = this.separator;
    const lines = rows.map((r) => `${r.name}${sep}${r.score}`);
    return [`name${sep}score`, ...lines].join("\n");
  }
}

class JsonExporter {
  name() {
    return "json";
  }

  exportRows(rows) {
    return JSON.stringify(rows);
  }
}

class MarkdownExporter {
  name() {
    return "markdown";
  }

  exportRows(rows) {
    const lines = rows.map((r) => `| ${r.name} | ${r.score} |`);
    return ["| name | score |", "| --- | --- |", ...lines].join("\n");
  }
}

// Not every exporter needs a class. A plain object with the right methods
// works just as well.
const summaryExporter = {
  name() {
    return "summary";
  },
  exportRows(rows) {
    const best = rows.reduce((a, b) => (b.score > a.score ? b : a));
    return `${rows.length} rows, top score ${best.score} by ${best.name}`;
  },
};

function run(exporter, rows) {
  console.log(`== ${exporter.name()}`);
  console.log(exporter.exportRows(rows));
}

const rows = [
  { name: "ada", score: 90 },
  { name: "grace", score: 85 },
  { name: "alan", score: 78 },
];
const exporters = [
  new CsvExporter(","),
  new JsonExporter(),
  new MarkdownExporter(),
  summaryExporter,
];
for (const exporter of exporters) {
  run(exporter, rows);
}
