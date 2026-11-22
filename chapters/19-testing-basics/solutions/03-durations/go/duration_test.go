package duration

import "testing"

func TestParse(t *testing.T) {
	tests := []struct {
		text string
		want int64
	}{
		{"45s", 45},
		{"1h30m", 5400},
		{"2d", 172800},
		{"1d2h3m4s", 93784},
		// A number can be more than the next unit up holds, and it can be 0.
		{"90m", 5400},
		{"0s", 0},
		{"1h0m", 3600},
	}
	for _, tt := range tests {
		if got, err := Parse(tt.text); got != tt.want || err != nil {
			t.Errorf("Parse(%q) = %d, %v, want %d, nil", tt.text, got, err, tt.want)
		}
	}
}

func TestParseRejects(t *testing.T) {
	tests := []string{
		"",       // no parts
		"90",     // a number with no unit
		"1h30",   // the same, after a part
		"h",      // a unit with no number
		"1x",     // not a unit
		"1h1h",   // a unit twice
		"30m1h",  // units out of order
		"1h 30m", // anything else
		"-1h",
	}
	for _, text := range tests {
		if got, err := Parse(text); err == nil {
			t.Errorf("Parse(%q) = %d, nil, want an error", text, got)
		}
	}
}

func TestFormat(t *testing.T) {
	tests := []struct {
		seconds int64
		want    string
	}{
		{5400, "1h30m"},
		{93784, "1d2h3m4s"},
		{59, "59s"},
		// Units that are 0 are left out, wherever they are, unless everything is.
		{3600, "1h"},
		{86401, "1d1s"},
		{0, "0s"},
	}
	for _, tt := range tests {
		if got := Format(tt.seconds); got != tt.want {
			t.Errorf("Format(%d) = %q, want %q", tt.seconds, got, tt.want)
		}
	}
}

func TestRoundTrip(t *testing.T) {
	for s := range int64(100_000) {
		if got, err := Parse(Format(s)); got != s || err != nil {
			t.Fatalf("Parse(Format(%d)) = %d, %v", s, got, err)
		}
	}
}
