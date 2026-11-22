package main

import (
	"fmt"
	"sync"
)

const limit = 200000
const workers = 4

func isPrime(n int) bool {
	if n < 2 {
		return false
	}
	for d := 2; d*d <= n; d++ {
		if n%d == 0 {
			return false
		}
	}
	return true
}

func countPrimes(start, end int) int {
	count := 0
	for n := start; n < end; n++ {
		if isPrime(n) {
			count++
		}
	}
	return count
}

func main() {
	fmt.Printf("counting primes below %d with %d workers\n", limit, workers)

	// go starts a function on a new goroutine. Each one writes only its own
	// element of counts, and the WaitGroup counts the goroutines still running.
	counts := make([]int, workers)
	var wg sync.WaitGroup
	for i := range workers {
		wg.Add(1)
		go func() {
			defer wg.Done()
			counts[i] = countPrimes(limit*i/workers, limit*(i+1)/workers)
		}()
	}
	wg.Wait() // waits until every goroutine has called Done

	total := 0
	for i, count := range counts {
		fmt.Printf("worker %d: [%d, %d) has %d primes\n", i, limit*i/workers,
			limit*(i+1)/workers, count)
		total += count
	}
	fmt.Printf("total: %d primes\n", total)
}
