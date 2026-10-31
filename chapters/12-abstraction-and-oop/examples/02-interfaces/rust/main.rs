// A row is a name and a score.
type Row = (&'static str, u32);

// A trait lists the methods a type must have. A type says so with impl.
trait Exporter {
    fn name(&self) -> &str;
    fn export(&self, rows: &[Row]) -> String;
}

struct CsvExporter {
    separator: char,
}

impl Exporter for CsvExporter {
    fn name(&self) -> &str {
        "csv"
    }

    fn export(&self, rows: &[Row]) -> String {
        let sep = self.separator;
        let mut lines = vec![format!("name{sep}score")];
        for (name, score) in rows {
            lines.push(format!("{name}{sep}{score}"));
        }
        lines.join("\n")
    }
}

struct JsonExporter;

impl Exporter for JsonExporter {
    fn name(&self) -> &str {
        "json"
    }

    fn export(&self, rows: &[Row]) -> String {
        let mut items = Vec::new();
        for (name, score) in rows {
            items.push(format!("{{\"name\":\"{name}\",\"score\":{score}}}"));
        }
        format!("[{}]", items.join(","))
    }
}

struct MarkdownExporter;

impl Exporter for MarkdownExporter {
    fn name(&self) -> &str {
        "markdown"
    }

    fn export(&self, rows: &[Row]) -> String {
        let mut lines = vec!["| name | score |".to_string(), "| --- | --- |".to_string()];
        for (name, score) in rows {
            lines.push(format!("| {name} | {score} |"));
        }
        lines.join("\n")
    }
}

struct SummaryExporter;

impl Exporter for SummaryExporter {
    fn name(&self) -> &str {
        "summary"
    }

    fn export(&self, rows: &[Row]) -> String {
        let (name, score) = rows.iter().max_by_key(|row| row.1).expect("some rows");
        format!("{} rows, top score {score} by {name}", rows.len())
    }
}

// &dyn Exporter is a pointer to some Exporter, plus a pointer to that type's
// table of methods, so the call is chosen at run time.
fn run(exporter: &dyn Exporter, rows: &[Row]) {
    println!("== {}", exporter.name());
    println!("{}", exporter.export(rows));
}

fn main() {
    let rows = [("ada", 90), ("grace", 85), ("alan", 78)];
    // Each Box holds a different type, so the Vec holds trait objects.
    let exporters: Vec<Box<dyn Exporter>> = vec![
        Box::new(CsvExporter { separator: ',' }),
        Box::new(JsonExporter),
        Box::new(MarkdownExporter),
        Box::new(SummaryExporter),
    ];
    for exporter in &exporters {
        run(exporter.as_ref(), &rows);
    }
}
