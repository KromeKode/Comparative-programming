package main

import "fmt"

// Go interfaces have no default methods. Shared code goes in a struct that
// other structs embed. Named supplies a default Describe.
type Named struct {
	name string
}

func (n Named) Describe() string {
	return "this is the " + n.name
}

// Logged holds a history. An embedded struct cannot see the struct around it,
// so Logged cannot look up the name for itself: the caller passes it in.
type Logged struct {
	history []string
}

func (l *Logged) log(name, event string) {
	l.history = append(l.history, name+": "+event)
}

func (l *Logged) History() []string {
	return l.history
}

type Door struct {
	Named
	Logged
}

func (d *Door) Open() {
	d.log(d.name, "opened")
}

func (d *Door) Close() {
	d.log(d.name, "closed")
}

type Light struct {
	Named
	Logged
	on bool
}

func (l *Light) SwitchOn() {
	l.on = true
	l.log(l.name, "switched on")
}

// Light's own Describe hides the promoted one, and can still call it.
func (l *Light) Describe() string {
	state := "off"
	if l.on {
		state = "on"
	}
	return l.Named.Describe() + ", and it is " + state
}

type Thing interface {
	Describe() string
	History() []string
}

func main() {
	door := &Door{Named: Named{"front door"}}
	light := &Light{Named: Named{"porch light"}}
	door.Open()
	door.Close()
	light.SwitchOn()

	for _, thing := range []Thing{door, light} {
		fmt.Println(thing.Describe())
		for _, line := range thing.History() {
			fmt.Println("  " + line)
		}
	}
}
