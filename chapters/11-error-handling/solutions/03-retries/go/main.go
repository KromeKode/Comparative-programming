package main

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strings"
)

const maxAttempts = 3

var (
	ErrTimedOut     = errors.New("timed out")
	ErrServerBusy   = errors.New("server busy")
	ErrNotFound     = errors.New("not found")
	ErrLoginRefused = errors.New("login refused")
)

// GaveUpError wraps the error from the last attempt.
type GaveUpError struct {
	Attempts int
	Last     error
}

func (e *GaveUpError) Error() string {
	return fmt.Sprintf("gave up after %d attempts: %v", e.Attempts, e.Last)
}

func (e *GaveUpError) Unwrap() error { return e.Last }

// Try the same file again?
func isRetryable(err error) bool {
	return errors.Is(err, ErrTimedOut) || errors.Is(err, ErrServerBusy)
}

// download is the pretend server: the answer for this attempt. The last
// answer repeats.
func download(answers []string, attempt int) error {
	switch answers[min(attempt, len(answers))-1] {
	case "timeout":
		return ErrTimedOut
	case "busy":
		return ErrServerBusy
	case "missing":
		return ErrNotFound
	case "auth":
		return ErrLoginRefused
	}
	return nil
}

// downloadWithRetries returns the number of attempts it took, or the error
// that stopped it.
func downloadWithRetries(name string, answers []string) (int, error) {
	wait := 1
	for attempt := 1; ; attempt++ {
		err := download(answers, attempt)
		if err == nil {
			return attempt, nil
		}
		if !isRetryable(err) {
			return attempt, err
		}
		if attempt == maxAttempts {
			return attempt, &GaveUpError{Attempts: attempt, Last: err}
		}
		fmt.Printf("%s: %v, retrying in %ds\n", name, err, wait)
		wait *= 2
	}
}

func main() {
	var jobs [][]string
	scanner := bufio.NewScanner(os.Stdin)
	for scanner.Scan() {
		if words := strings.Fields(scanner.Text()); len(words) > 0 {
			jobs = append(jobs, words)
		}
	}

	downloaded, skipped, failed, notTried := 0, 0, 0, 0
	// The label lets the break below leave the loop, not just the switch.
batch:
	for i, job := range jobs {
		name := job[0]
		attempts, err := downloadWithRetries(name, job[1:])
		var gaveUp *GaveUpError
		switch {
		case err == nil:
			plural := "s"
			if attempts == 1 {
				plural = ""
			}
			fmt.Printf("%s: ok after %d attempt%s\n", name, attempts, plural)
			downloaded++
		// Check for GaveUpError first: it wraps a retryable error, so
		// errors.Is(err, ErrTimedOut) is true for it too.
		case errors.As(err, &gaveUp):
			fmt.Printf("%s: failed: %v\n", name, err)
			failed++
		case errors.Is(err, ErrLoginRefused):
			fmt.Printf("%s: stopped: %v\n", name, err)
			failed++
			notTried = len(jobs) - i - 1
			break batch
		default:
			fmt.Printf("%s: skipped: %v\n", name, err)
			skipped++
		}
	}
	fmt.Printf("downloaded %d, skipped %d, failed %d, not tried %d\n",
		downloaded, skipped, failed, notTried)
}
