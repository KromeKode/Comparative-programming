import 'dart:collection';
import 'dart:io';

// A sealed class has only the subclasses in this file, so a switch over a
// Step can check that it covers every kind.
sealed class Step {}

class Print extends Step {
  final String text;
  Print(this.text);
}

class Sleep extends Step {
  final int duration;
  Sleep(this.duration);
}

class Wait extends Step {
  final int task; // the number of the task to wait for
  Wait(this.task);
}

class Task {
  final String name;
  final steps = <Step>[];
  var next = 0; // the index of the step to run when it runs again
  var done = false;
  var waitingFor = 0;
  final waiters = <int>[]; // in the order they began waiting
  Task(this.name);
}

// The first word of a line, and the rest of it.
(String, String) split(String line) {
  final trimmed = line.trim();
  final space = trimmed.indexOf(' ');
  if (space < 0) return (trimmed, '');
  return (trimmed.substring(0, space), trimmed.substring(space).trim());
}

// Returns the tasks, and prints an error for each line with a problem.
List<Task> parse(List<String> lines) {
  // A wait can name a task defined further down, so the names come first.
  final numbers = <String, int>{};
  for (final line in lines) {
    final (command, name) = split(line);
    if (command == 'task') numbers.putIfAbsent(name, () => numbers.length);
  }
  final tasks = <Task>[];
  for (final (i, line) in lines.indexed) {
    final (command, rest) = split(line);
    void error(String message) => print('error: line ${i + 1}: $message');
    final steps = tasks.lastOrNull?.steps;
    final other = numbers[rest];
    if (command.isEmpty) continue;
    if (command == 'task') {
      tasks.add(Task(rest));
    } else if (steps == null) {
      error('a step before any task');
    } else if (command == 'print') {
      steps.add(Print(rest));
    } else if (command == 'sleep') {
      if (RegExp(r'^[0-9]+$').hasMatch(rest)) {
        steps.add(Sleep(int.parse(rest)));
      } else {
        error('sleep takes a whole number');
      }
    } else if (command == 'wait') {
      if (other != null) {
        steps.add(Wait(other));
      } else {
        error('no task named $rest');
      }
    } else {
      error('unknown step: $command');
    }
  }
  return tasks;
}

typedef Sleeper = ({int wake, int order, int task});

class Loop {
  final List<Task> tasks;
  var time = 0;
  final Queue<int> ready; // the tasks that can run now, in order
  // The sleeping tasks, kept in order by a comparator: the first one is the
  // one to wake first. Dart's core libraries have no heap, but a sorted set
  // can take its place.
  final sleeping = SplayTreeSet<Sleeper>(
    (a, b) => a.wake != b.wake ? a.wake - b.wake : a.order - b.order,
  );
  var sleeps = 0;

  Loop(this.tasks)
    : ready = Queue.of([for (var id = 0; id < tasks.length; id++) id]);

  // Runs a task until it sleeps, waits, or ends.
  void runUntilStopped(int id) {
    final task = tasks[id];
    while (task.next < task.steps.length) {
      switch (task.steps[task.next++]) {
        case Print(:final text):
          print('[$time] ${task.name}: $text');
        case Sleep(duration: 0):
          ready.add(id);
          return;
        case Sleep(:final duration):
          sleeping.add((wake: time + duration, order: sleeps++, task: id));
          return;
        case Wait(task: final other) when !tasks[other].done:
          task.waitingFor = other;
          tasks[other].waiters.add(id);
          return;
        case Wait():
          break; // it has finished already, so carry on
      }
    }
    task.done = true;
    print('[$time] ${task.name}: done');
    ready.addAll(task.waiters);
  }

  void run() {
    while (true) {
      while (ready.isNotEmpty) {
        runUntilStopped(ready.removeFirst());
      }
      if (sleeping.isEmpty) break;
      // Nothing can run now, so the clock jumps to the next wake.
      time = sleeping.first.wake;
      while (sleeping.isNotEmpty && sleeping.first.wake == time) {
        final first = sleeping.first;
        sleeping.remove(first);
        ready.add(first.task);
      }
    }
    final stuck = [
      for (final task in tasks)
        if (!task.done) '${task.name} waits for ${tasks[task.waitingFor].name}',
    ];
    if (stuck.isNotEmpty) {
      print('[$time] deadlock: ${stuck.join(', ')}');
    } else {
      print('[$time] all done');
    }
  }
}

void main() {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  Loop(parse(lines)).run();
}
