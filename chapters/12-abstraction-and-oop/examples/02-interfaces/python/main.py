from typing import Protocol

Row = tuple[str, int]


# A Protocol describes the methods an exporter needs. Classes don't inherit
# from it: any class with these methods counts, and a type checker such as
# mypy checks that they match.
class Exporter(Protocol):
    def name(self) -> str: ...

    def export(self, rows: list[Row]) -> str: ...


class CsvExporter:
    def __init__(self, separator: str):
        self.separator = separator

    def name(self) -> str:
        return "csv"

    def export(self, rows: list[Row]) -> str:
        lines = [f"name{self.separator}score"]
        lines += [f"{name}{self.separator}{score}" for name, score in rows]
        return "\n".join(lines)


class JsonExporter:
    def name(self) -> str:
        return "json"

    def export(self, rows: list[Row]) -> str:
        items = [f'{{"name":"{name}","score":{score}}}' for name, score in rows]
        return "[" + ",".join(items) + "]"


class MarkdownExporter:
    def name(self) -> str:
        return "markdown"

    def export(self, rows: list[Row]) -> str:
        lines = ["| name | score |", "| --- | --- |"]
        lines += [f"| {name} | {score} |" for name, score in rows]
        return "\n".join(lines)


class SummaryExporter:
    def name(self) -> str:
        return "summary"

    def export(self, rows: list[Row]) -> str:
        best_name, best_score = max(rows, key=lambda row: row[1])
        return f"{len(rows)} rows, top score {best_score} by {best_name}"


# This function knows only the Protocol, not the classes.
def run(exporter: Exporter, rows: list[Row]) -> None:
    print(f"== {exporter.name()}")
    print(exporter.export(rows))


rows = [("ada", 90), ("grace", 85), ("alan", 78)]
exporters: list[Exporter] = [
    CsvExporter(","),
    JsonExporter(),
    MarkdownExporter(),
    SummaryExporter(),
]
for exporter in exporters:
    run(exporter, rows)
