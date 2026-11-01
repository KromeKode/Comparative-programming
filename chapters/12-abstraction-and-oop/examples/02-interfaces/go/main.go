package main

import (
	"fmt"
	"strings"
)

type Row struct {
	Name  string
	Score int
}

// Exporter is an interface. A type satisfies it by having these methods; it
// never says "implements".
type Exporter interface {
	Name() string
	Export(rows []Row) string
}

type CSV struct{ Separator string }

func (c CSV) Name() string { return "csv" }

func (c CSV) Export(rows []Row) string {
	lines := []string{"name" + c.Separator + "score"}
	for _, row := range rows {
		lines = append(lines, fmt.Sprintf("%s%s%d", row.Name, c.Separator, row.Score))
	}
	return strings.Join(lines, "\n")
}

type JSON struct{}

func (JSON) Name() string { return "json" }

func (JSON) Export(rows []Row) string {
	items := make([]string, len(rows))
	for i, row := range rows {
		items[i] = fmt.Sprintf(`{"name":"%s","score":%d}`, row.Name, row.Score)
	}
	return "[" + strings.Join(items, ",") + "]"
}

type Markdown struct{}

func (Markdown) Name() string { return "markdown" }

func (Markdown) Export(rows []Row) string {
	lines := []string{"| name | score |", "| --- | --- |"}
	for _, row := range rows {
		lines = append(lines, fmt.Sprintf("| %s | %d |", row.Name, row.Score))
	}
	return strings.Join(lines, "\n")
}

type Summary struct{}

func (Summary) Name() string { return "summary" }

func (Summary) Export(rows []Row) string {
	best := rows[0]
	for _, row := range rows[1:] {
		if row.Score > best.Score {
			best = row
		}
	}
	return fmt.Sprintf("%d rows, top score %d by %s", len(rows), best.Score, best.Name)
}

// This line fails to compile if CSV ever stops satisfying Exporter.
var _ Exporter = CSV{}

// run knows only the interface. An interface value holds the concrete value
// and its type, and the call is chosen at run time.
func run(exporter Exporter, rows []Row) {
	fmt.Println("==", exporter.Name())
	fmt.Println(exporter.Export(rows))
}

func main() {
	rows := []Row{{"ada", 90}, {"grace", 85}, {"alan", 78}}
	exporters := []Exporter{CSV{Separator: ","}, JSON{}, Markdown{}, Summary{}}
	for _, exporter := range exporters {
		run(exporter, rows)
	}
}
