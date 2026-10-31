package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
)

// Go has no optional type, so each setting's zero value means "not set". An empty
// host is not allowed, and 0 is not a valid port or timeout, so that works here.
type config struct {
	host    string
	port    int
	timeout int
}

// parseNumber returns the number if text is digits only and from 1 to high.
func parseNumber(text string, high int) (int, bool) {
	// Trim removes digits from both ends, so anything left over is not a digit.
	if text == "" || strings.Trim(text, "0123456789") != "" {
		return 0, false
	}
	value, err := strconv.Atoi(text) // err is set when the number is too big
	if err != nil || value < 1 || value > high {
		return 0, false
	}
	return value, true
}

// setNumber sets *setting from value, or reports a bad value and leaves it alone.
func setNumber(setting *int, key, value string, high, number int) {
	if value == "" {
		*setting = 0
	} else if parsed, ok := parseNumber(value, high); ok {
		*setting = parsed
	} else {
		fmt.Printf("line %d: bad %s %q\n", number, key, value)
	}
}

func main() {
	var c config
	scanner := bufio.NewScanner(os.Stdin)
	for number := 1; scanner.Scan(); number++ {
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}
		key, value, found := strings.Cut(line, "=")
		if !found {
			fmt.Printf("line %d: expected key = value\n", number)
			continue
		}
		key, value = strings.TrimSpace(key), strings.TrimSpace(value)
		switch key {
		case "host":
			c.host = value
		case "port":
			setNumber(&c.port, key, value, 65535, number)
		case "timeout":
			setNumber(&c.timeout, key, value, 86400, number)
		default:
			fmt.Printf("line %d: unknown key %q\n", number, key)
		}
	}

	if c.host != "" {
		fmt.Println("host:", c.host)
	} else {
		fmt.Println("host: localhost (default)")
	}
	if c.port != 0 {
		fmt.Println("port:", c.port)
	} else {
		fmt.Println("port: 8080 (default)")
	}
	if c.timeout != 0 {
		fmt.Printf("timeout: %ds\n", c.timeout)
	} else {
		fmt.Println("timeout: none")
	}
}
