// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. The flag package reads the options, prints -h, and exits with status 2, after
// a message on stderr, when the command line is wrong.
package main

import (
	"flag"
	"fmt"
	"math"
	"os"
	"strconv"
)

var toCelsius = map[string]func(float64) float64{
	"c": func(t float64) float64 { return t },
	"f": func(t float64) float64 { return (t - 32) * 5 / 9 },
	"k": func(t float64) float64 { return t - 273.15 },
}

var fromCelsius = map[string]func(float64) float64{
	"c": func(t float64) float64 { return t },
	"f": func(t float64) float64 { return t*9/5 + 32 },
	"k": func(t float64) float64 { return t + 273.15 },
}

type temperature struct {
	text  string
	value float64
	unit  string
}

// parseTemperature reads a number followed by c, f, or k.
func parseTemperature(text string) (temperature, bool) {
	if text == "" {
		return temperature{}, false
	}
	unit := text[len(text)-1:]
	if _, ok := toCelsius[unit]; !ok {
		return temperature{}, false
	}
	value, err := strconv.ParseFloat(text[:len(text)-1], 64)
	if err != nil || math.IsInf(value, 0) || math.IsNaN(value) {
		return temperature{}, false
	}
	return temperature{text, value, unit}, true
}

// usageError prints a message and the usage to stderr, and exits with status 2, as the
// flag package does for its own errors.
func usageError(message string) {
	fmt.Fprintln(os.Stderr, "temp:", message)
	flag.Usage()
	os.Exit(2)
}

func main() {
	// Each flag has two names, -t and -to; flag accepts one dash or two for either.
	var to string
	var digits int
	flag.StringVar(&to, "to", "c", "the unit to convert to: c, f, or k")
	flag.StringVar(&to, "t", "c", "the same as -to")
	flag.IntVar(&digits, "digits", 1, "digits after the point, 0 to 9")
	flag.IntVar(&digits, "d", 1, "the same as -digits")
	flag.Usage = func() {
		fmt.Fprintln(os.Stderr, "usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...")
		flag.PrintDefaults()
	}
	flag.Parse() // exits with status 2 if a flag is wrong

	if _, ok := fromCelsius[to]; !ok {
		usageError("unknown unit: " + to)
	}
	if digits < 0 || digits > 9 {
		usageError(fmt.Sprintf("digits must be 0 to 9, not %d", digits))
	}
	if flag.NArg() == 0 {
		usageError("no temperatures")
	}

	// Check every temperature before printing any, so that a mistake prints nothing.
	var temperatures []temperature
	for _, text := range flag.Args() {
		t, ok := parseTemperature(text)
		if !ok {
			fmt.Fprintln(os.Stderr, "temp: not a temperature:", text)
			os.Exit(2)
		}
		temperatures = append(temperatures, t)
	}

	for _, t := range temperatures {
		converted := fromCelsius[to](toCelsius[t.unit](t.value))
		fmt.Printf("%s = %.*f%s\n", t.text, digits, converted, to)
	}
}
