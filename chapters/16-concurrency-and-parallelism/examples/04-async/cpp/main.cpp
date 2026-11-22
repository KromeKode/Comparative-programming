#include <coroutine>
#include <deque>
#include <exception>
#include <iostream>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

// C++20 has coroutines, with co_await and co_return, but the standard library
// has no event loop and no task type to run them with. Libraries supply those.
// This is a small one, to show what they do.

// The coroutines that are ready to run, in the order they will run.
std::deque<std::coroutine_handle<>> ready;

// Awaiting a Pause suspends the coroutine, and puts it at the back of the line.
struct Pause {
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> handle) const { ready.push_back(handle); }
    void await_resume() const noexcept {}
};

// A coroutine that returns a T. The compiler finds promise_type, and calls
// its functions when the coroutine starts, returns, and finishes.
template <typename T>
class Task {
public:
    struct promise_type {
        std::optional<T> result;

        Task get_return_object() {
            return Task(std::coroutine_handle<promise_type>::from_promise(*this));
        }
        // Start suspended, so that nothing runs until run_all resumes it.
        std::suspend_always initial_suspend() noexcept { return {}; }
        // Stay suspended at the end, so that the result is still there.
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T value) { result = std::move(value); }
        void unhandled_exception() { std::terminate(); }
    };

    explicit Task(std::coroutine_handle<promise_type> handle) : handle(handle) {}
    Task(Task&& other) noexcept : handle(std::exchange(other.handle, {})) {}
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task& operator=(Task&&) = delete;
    // The coroutine's frame, with its locals, is on the heap until this.
    ~Task() {
        if (handle) handle.destroy();
    }

    std::coroutine_handle<promise_type> handle;
};

// Runs the tasks on this one thread until all of them finish, and returns
// their results in order.
template <typename T>
std::vector<T> run_all(std::vector<Task<T>> tasks) {
    for (const auto& task : tasks) ready.push_back(task.handle);
    while (!ready.empty()) {
        auto next = ready.front();
        ready.pop_front();
        next.resume();  // runs the coroutine up to its next co_await
    }
    std::vector<T> results;
    for (const auto& task : tasks) results.push_back(*task.handle.promise().result);
    return results;
}

// name is a string_view of a string literal, which lasts as long as the program.
// A view of a temporary string would dangle after the first co_await.
Task<int> task(std::string_view name, int steps) {
    for (int step = 1; step <= steps; step++) {
        std::cout << name << ": step " << step << "\n";
        // co_await is the only place a task can stop and let another one run.
        co_await Pause{};
    }
    std::cout << name << ": done\n";
    co_return steps;
}

// These return the count they wrote. count is a reference to main's int,
// which outlives the tasks.
Task<int> add_at_once(int& count) {
    count += 1;  // no co_await, so no other task can run in the middle
    co_return count;
}

Task<int> add_with_await(int& count) {
    int value = count;
    co_await Pause{};  // every other task reads the old value here
    count = value + 1;
    co_return count;
}

int main() {
    std::cout << "three tasks, each pausing at every await\n";
    std::vector<Task<int>> tasks;
    tasks.push_back(task("a", 2));
    tasks.push_back(task("b", 3));
    tasks.push_back(task("c", 1));
    auto results = run_all(std::move(tasks));
    std::cout << "results: " << results[0] << ", " << results[1] << ", " << results[2] << "\n";

    std::cout << "4 tasks add 1 to a shared count\n";
    int count = 0;
    std::vector<Task<int>> adds;
    for (int i = 0; i < 4; i++) adds.push_back(add_at_once(count));
    run_all(std::move(adds));
    std::cout << "with no await between reading and writing: " << count << "\n";
    count = 0;
    adds.clear();
    for (int i = 0; i < 4; i++) adds.push_back(add_with_await(count));
    run_all(std::move(adds));
    std::cout << "with an await between reading and writing: " << count << "\n";
}
