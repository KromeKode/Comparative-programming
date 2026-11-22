package main

import (
	"fmt"

	"example.com/loadorder/settings"
)

func main() {
	fmt.Println("main: start")
	fmt.Println("main: the greeting is", settings.Greeting)
	fmt.Println("main: the greeting is still", settings.Greeting)
}
