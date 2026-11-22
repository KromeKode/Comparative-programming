package main

import (
	"fmt"
	"sync"
)

const count = 1000
const workers = 3

func main() {
	// A channel carries values from one goroutine to another. These hold up
	// to 16 values, and a send to a full channel waits for room.
	jobs := make(chan int, 16)
	results := make(chan int, 16)

	// The producer sends the numbers, then closes the channel to say that
	// no more are coming.
	go func() {
		for n := 1; n <= count; n++ {
			jobs <- n
		}
		close(jobs)
	}()

	// Each worker takes numbers until jobs is closed and empty. No number
	// goes to two workers.
	var wg sync.WaitGroup
	for range workers {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for n := range jobs {
				results <- n * n
			}
		}()
	}

	// When every worker has finished, nothing else will send a result.
	go func() {
		wg.Wait()
		close(results)
	}()

	received, sum := 0, 0
	for square := range results {
		received++
		sum += square
	}
	fmt.Printf("%d workers squared 1 to %d\n", workers, count)
	fmt.Println("results:", received)
	fmt.Println("sum:", sum)
}
