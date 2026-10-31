interface Row {
  name: string;
  score: number;
}

// TypeScript's interfaces are structural: any value with these members is an
// Exporter, whether or not its class says "implements Exporter".
interface Exporter {
  name(): string;
  exportRows(rows: Row[]): string;
}

class CsvExporter implements Exporter {
  readonly separator: string;
  constructor(separator: string) {
    this.separator = separator;
  }

  name(): string {
    return "csv";
  }

  exportRows(rows: Row[]): string {
    const sep = this.separator;
    const lines = rows.map((r) => `${r.name}${sep}${r.score}`);
    return [`name${sep}score`, ...lines].join("\n");
  }
}

class JsonExporter implements Exporter {
  name(): string {
    return "json";
  }

  exportRows(rows: Row[]): string {
    return JSON.stringify(rows);
  }
}

class MarkdownExporter implements Exporter {
  name(): string {
    return "markdown";
  }

  exportRows(rows: Row[]): string {
    const lines = rows.map((r) => `| ${r.name} | ${r.score} |`);
    return ["| name | score |", "| --- | --- |", ...lines].join("\n");
  }
}

// A plain object is an Exporter too, because it has the right shape.
const summaryExporter: Exporter = {
  name() {
    return "summary";
  },
  exportRows(rows) {
    const best = rows.reduce((a, b) => (b.score > a.score ? b : a));
    return `${rows.length} rows, top score ${best.score} by ${best.name}`;
  },
};

function run(exporter: Exporter, rows: Row[]): void {
  console.log(`== ${exporter.name()}`);
  console.log(exporter.exportRows(rows));
}

const rows: Row[] = [
  { name: "ada", score: 90 },
  { name: "grace", score: 85 },
  { name: "alan", score: 78 },
];
const exporters: Exporter[] = [
  new CsvExporter(","),
  new JsonExporter(),
  new MarkdownExporter(),
  summaryExporter,
];
for (const exporter of exporters) {
  run(exporter, rows);
}
