package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

func main() {
	reader := bufio.NewReader(os.Stdin)
	line, _ := reader.ReadString('\n') // at end of input, line holds whatever was read

	name := strings.TrimSpace(line)
	if name == "" {
		fmt.Println("Hello, stranger!")
	} else {
		fmt.Printf("Hello, %s!\n", name)
	}
}
