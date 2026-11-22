package main

import (
	"bufio"
	"fmt"
	"os"
	"reflect"
	"runtime"
	"strconv"
	"strings"
)

var tracer struct {
	on    bool
	depth int
	calls int
}

func say(text string) {
	if tracer.on {
		fmt.Println(strings.Repeat("  ", tracer.depth) + text)
	}
}

func showCall(name string, args []int64) string {
	texts := make([]string, len(args))
	for i, arg := range args {
		texts[i] = strconv.FormatInt(arg, 10)
	}
	return name + "(" + strings.Join(texts, ", ") + ")"
}

// trace runs body as one traced call of the function that called trace. The
// runtime can look up the caller on the call stack, and so find its name.
func trace(body func() int64, args ...int64) int64 {
	pc, _, _, _ := runtime.Caller(1)
	name := strings.TrimPrefix(runtime.FuncForPC(pc).Name(), "main.")
	tracer.calls++
	say(showCall(name, args))
	tracer.depth++
	result := body()
	tracer.depth--
	say(fmt.Sprintf("-> %d", result))
	return result
}

// Each function hands its body to trace as a closure, and the recursive
// calls in the body go through the function, and so through trace, again.
func fib(n int64) int64 {
	return trace(func() int64 {
		if n < 2 {
			return n
		}
		return fib(n-1) + fib(n-2)
	}, n)
}

func gcd(a, b int64) int64 {
	return trace(func() int64 {
		if b == 0 {
			return a
		}
		return gcd(b, a%b)
	}, a, b)
}

func power(base, exponent int64) int64 {
	return trace(func() int64 {
		if exponent == 0 {
			return 1
		}
		if exponent%2 == 0 {
			half := power(base, exponent/2)
			return half * half
		}
		return base * power(base, exponent-1)
	}, base, exponent)
}

// reflect can ask a function how many parameters it has, and call it with a
// slice of arguments, so the table needs only the functions themselves.
var functions = map[string]reflect.Value{
	"fib":   reflect.ValueOf(fib),
	"gcd":   reflect.ValueOf(gcd),
	"power": reflect.ValueOf(power),
}

func plural(count int, word string) string {
	if count == 1 {
		return fmt.Sprintf("%d %s", count, word)
	}
	return fmt.Sprintf("%d %ss", count, word)
}

func run(line string) {
	words := strings.Fields(line)
	command, name := words[0], words[1]
	var args []int64
	for _, word := range words[2:] {
		arg, _ := strconv.ParseInt(word, 10, 64)
		args = append(args, arg)
	}
	if command != "trace" && command != "count" {
		fmt.Println("unknown command:", command)
		return
	}
	function, ok := functions[name]
	if !ok {
		fmt.Println("unknown function:", name)
		return
	}
	arity := function.Type().NumIn()
	if len(args) != arity {
		fmt.Printf("%s takes %s, got %d\n", name, plural(arity, "argument"), len(args))
		return
	}
	in := make([]reflect.Value, len(args))
	for i, arg := range args {
		in[i] = reflect.ValueOf(arg)
	}
	tracer.on, tracer.depth, tracer.calls = command == "trace", 0, 0
	result := function.Call(in)[0].Int()
	fmt.Printf("%s = %d in %s\n", showCall(name, args), result, plural(tracer.calls, "call"))
}

func main() {
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		if strings.TrimSpace(scanner.Text()) != "" {
			run(scanner.Text())
		}
	}
}
