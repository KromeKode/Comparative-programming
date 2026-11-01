#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

struct Row {
    std::string name;
    int score;
};

// An abstract class: = 0 marks a function that every subclass must define,
// and no Exporter can be made on its own.
class Exporter {
  public:
    virtual ~Exporter() = default;
    virtual std::string name() const = 0;
    virtual std::string export_rows(const std::vector<Row> &rows) const = 0;
};

class CsvExporter : public Exporter {
  public:
    explicit CsvExporter(char separator) : separator_(separator) {}

    std::string name() const override { return "csv"; }

    std::string export_rows(const std::vector<Row> &rows) const override {
        std::ostringstream out;
        out << "name" << separator_ << "score";
        for (const Row &row : rows) {
            out << '\n' << row.name << separator_ << row.score;
        }
        return out.str();
    }

  private:
    char separator_;
};

class JsonExporter : public Exporter {
  public:
    std::string name() const override { return "json"; }

    std::string export_rows(const std::vector<Row> &rows) const override {
        std::ostringstream out;
        out << '[';
        for (std::size_t i = 0; i < rows.size(); i++) {
            out << (i > 0 ? "," : "") << "{\"name\":\"" << rows[i].name
                << "\",\"score\":" << rows[i].score << '}';
        }
        out << ']';
        return out.str();
    }
};

class MarkdownExporter : public Exporter {
  public:
    std::string name() const override { return "markdown"; }

    std::string export_rows(const std::vector<Row> &rows) const override {
        std::ostringstream out;
        out << "| name | score |\n| --- | --- |";
        for (const Row &row : rows) {
            out << "\n| " << row.name << " | " << row.score << " |";
        }
        return out.str();
    }
};

class SummaryExporter : public Exporter {
  public:
    std::string name() const override { return "summary"; }

    std::string export_rows(const std::vector<Row> &rows) const override {
        auto best = std::max_element(
            rows.begin(), rows.end(),
            [](const Row &a, const Row &b) { return a.score < b.score; });
        return std::to_string(rows.size()) + " rows, top score " +
               std::to_string(best->score) + " by " + best->name;
    }
};

// This function knows only Exporter. The virtual call picks the subclass's
// function at run time.
void run(const Exporter &exporter, const std::vector<Row> &rows) {
    std::cout << "== " << exporter.name() << '\n'
              << exporter.export_rows(rows) << '\n';
}

int main() {
    std::vector<Row> rows{{"ada", 90}, {"grace", 85}, {"alan", 78}};
    std::vector<std::unique_ptr<Exporter>> exporters;
    exporters.push_back(std::make_unique<CsvExporter>(','));
    exporters.push_back(std::make_unique<JsonExporter>());
    exporters.push_back(std::make_unique<MarkdownExporter>());
    exporters.push_back(std::make_unique<SummaryExporter>());
    for (const auto &exporter : exporters) {
        run(*exporter, rows);
    }
}
