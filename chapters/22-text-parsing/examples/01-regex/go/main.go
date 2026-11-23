// Reads a web server's log, one request on each line, such as
//
//	2026-11-23 14:05:12 GET /books/42 200 12ms
//
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
package main

import (
	"bufio"
	"fmt"
	"os"
	"regexp"
	"strconv"
)

// ^ and $, because FindStringSubmatch finds a match anywhere in the line. Without the m
// flag, $ matches only at the very end. In Go, \d is only [0-9], but this says so.
var line = regexp.MustCompile(
	`^(?P<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?P<time>[0-9]{2}:[0-9]{2}:[0-9]{2})` +
		` (?P<method>GET|POST|PUT|DELETE) (?P<path>/[^ ]*)` +
		` (?P<status>[0-9]{3}) (?P<ms>[0-9]{1,9})ms$`)
var id = regexp.MustCompile(`/[0-9]+`)

// The groups are numbered in the match, so these find their numbers once.
var (
	method = line.SubexpIndex("method")
	path   = line.SubexpIndex("path")
	code   = line.SubexpIndex("status")
	ms     = line.SubexpIndex("ms")
)

type route struct {
	requests, failed, slowest int
}

func main() {
	var keys []string // a map has no order, so this keeps the order they came in
	routes := map[string]*route{}
	status := 0
	// A Scanner splits at "\n" and removes a "\r" before it. A line longer than 64 KiB
	// stops it with the error "token too long".
	scanner := bufio.NewScanner(os.Stdin)
	for number := 1; scanner.Scan(); number++ {
		match := line.FindStringSubmatch(scanner.Text())
		if match == nil {
			fmt.Fprintf(os.Stderr, "logscan: line %d: not a log line\n", number)
			status = 1
			continue
		}
		key := match[method] + " " + id.ReplaceAllLiteralString(match[path], "/:id")
		r := routes[key]
		if r == nil {
			r = &route{}
			routes[key] = r
			keys = append(keys, key)
		}
		// The regex let through only digits, few enough to fit, so Atoi can't fail.
		c, _ := strconv.Atoi(match[code])
		t, _ := strconv.Atoi(match[ms])
		r.requests++
		if c >= 400 {
			r.failed++
		}
		r.slowest = max(r.slowest, t)
	}
	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "logscan:", err)
		os.Exit(1)
	}
	for _, key := range keys {
		r := routes[key]
		plural := "s"
		if r.requests == 1 {
			plural = ""
		}
		fmt.Printf("%s: %d request%s, %d failed, slowest %dms\n",
			key, r.requests, plural, r.failed, r.slowest)
	}
	os.Exit(status)
}
