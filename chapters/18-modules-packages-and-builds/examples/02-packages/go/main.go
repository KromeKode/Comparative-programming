package main

import (
	"fmt"
	"io"
	"os"

	"example.com/textstats"
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
