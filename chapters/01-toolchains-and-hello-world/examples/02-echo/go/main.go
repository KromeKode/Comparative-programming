package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

func main() {
	reader := bufio.NewReader(os.Stdin)
	line, _ := reader.ReadString('\n') // the error only says "end of input" here
	line = strings.TrimRight(line, "\r\n")
	fmt.Println("You typed: " + line)
}
