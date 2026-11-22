// Package settings has a variable whose value comes from a function call.
package settings

import "fmt"

// Package variables are set before main runs, and before the packages that
// import this one are set up, so a package's dependencies are always ready.
var Greeting = load()

func load() string {
	fmt.Println("settings: loading")
	return "Hello"
}
