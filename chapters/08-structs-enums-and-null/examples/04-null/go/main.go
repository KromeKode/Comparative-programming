package main

import "fmt"

// Go has no optional type. A zero value such as "" often stands for "none", and a
// second result, ok, says whether a lookup found anything.
type user struct {
	name     string
	nickname string // "" when the user has no nickname
}

func find(users []user, name string) (user, bool) {
	for _, u := range users {
		if u.name == name {
			return u, true
		}
	}
	return user{}, false
}

func main() {
	users := []user{
		{name: "ada", nickname: "Countess"},
		{name: "alan"},
		{name: "grace", nickname: "Amazing Grace"},
	}

	for _, u := range users {
		shown := u.name
		if u.nickname != "" {
			shown = u.nickname
		}
		fmt.Printf("%s is shown as %s\n", u.name, shown)
	}

	fmt.Print("nickname lengths:")
	for _, u := range users {
		fmt.Print(" ", len(u.nickname))
	}
	fmt.Println()

	for _, name := range []string{"alan", "linus"} {
		if u, ok := find(users, name); ok {
			fmt.Printf("find %q: %s\n", name, u.name)
		} else {
			fmt.Printf("find %q: nobody\n", name)
		}
	}

	for _, name := range []string{"ada", "alan", "linus"} {
		nickname := "none"
		if u, ok := find(users, name); ok && u.nickname != "" {
			nickname = u.nickname
		}
		fmt.Printf("nickname of %q: %s\n", name, nickname)
	}
}
