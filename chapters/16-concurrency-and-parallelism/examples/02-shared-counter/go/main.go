package main

import (
	"fmt"
	"sync"
	"sync/atomic"
)

const threads = 4
const times = 100000

// run starts work on several goroutines and waits for all of them.
func run(work func()) {
	var wg sync.WaitGroup
	for range threads {
		wg.Add(1)
		go func() {
			defer wg.Done()
			work()
		}()
	}
	wg.Wait()
}

func main() {
	fmt.Printf("%d goroutines add 1, %d times each\n", threads, times)

	// Lock and Unlock let one goroutine at a time run the code between them.
	lockedCount := 0
	var mu sync.Mutex
	run(func() {
		for range times {
			mu.Lock()
			lockedCount++
			mu.Unlock()
		}
	})
	fmt.Println("with a mutex:", lockedCount)

	// Add reads, adds, and writes as one step.
	var atomicCount atomic.Int64
	run(func() {
		for range times {
			atomicCount.Add(1)
		}
	})
	fmt.Println("with an atomic:", atomicCount.Load())
}
