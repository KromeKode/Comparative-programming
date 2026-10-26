package main

import (
	"bufio"
	"fmt"
	"math/bits"
	"os"
	"strconv"
	"strings"
)

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		text := strings.TrimSpace(scanner.Text())
		if text == "" {
			continue
		}
		parsed, err := strconv.ParseUint(text, 10, 32)
		if err != nil {
			fmt.Fprintln(os.Stderr, "not a 32-bit unsigned number:", text)
			os.Exit(1)
		}
		n := uint32(parsed)
		fmt.Printf("%d: %b (ones %d, width %d)\n", n, n, bits.OnesCount32(n), bits.Len32(n))
	}
}
