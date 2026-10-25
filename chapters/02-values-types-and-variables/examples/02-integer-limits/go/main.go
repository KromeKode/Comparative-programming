package main

import (
	"fmt"
	"math"
)

func main() {
	var i32 int32 = math.MaxInt32
	fmt.Println("int32 max:", i32)
	i32++ // signed overflow is defined in Go: it wraps around
	fmt.Println("int32 max + 1:", i32)

	var i64 int64 = math.MaxInt64
	fmt.Println("int64 max:", i64)
	i64++
	fmt.Println("int64 max + 1:", i64)
}
