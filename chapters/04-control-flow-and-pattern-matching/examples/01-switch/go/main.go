package main

import "fmt"

func describe(code int) string {
	// Go never falls through to the next case, so there is no break to forget.
	switch code {
	case 200:
		return "ok"
	case 301, 302, 307, 308:
		return "redirect"
	case 404:
		return "not found"
	}
	// A switch with no value tests each case as a condition, top to bottom.
	switch {
	case code >= 400 && code <= 499:
		return "client error"
	case code >= 500 && code <= 599:
		return "server error"
	}
	return "unknown"
}

func main() {
	for _, code := range []int{200, 301, 308, 404, 418, 500, 503, 99} {
		fmt.Printf("%d: %s\n", code, describe(code))
	}
}
