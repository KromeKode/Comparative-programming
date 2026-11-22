package main

import (
	"fmt"
	"io"
	"os"

	// The import path is the module's path from go.mod, then the folder.
	"example.com/stats/textstats"
)

func main() {
	data, err := io.ReadAll(os.Stdin)
	if err != nil {
		panic(err)
	}
	text := string(data)
	fmt.Println("textstats", textstats.Version)
	fmt.Println("words:", textstats.WordCount(text))
	fmt.Println("longest:", textstats.LongestWord(text))
}
