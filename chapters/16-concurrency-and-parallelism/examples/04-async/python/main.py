import asyncio
from dataclasses import dataclass


async def task(name: str, steps: int) -> int:
    for step in range(1, steps + 1):
        print(f"{name}: step {step}")
        # await is the only place a task can stop and let another one run.
        # sleep(0) waits for nothing, so this task goes to the back of the
        # event loop's queue, behind the tasks that are ready.
        await asyncio.sleep(0)
    print(f"{name}: done")
    return steps


@dataclass
class Counter:
    value: int = 0


async def add_at_once(count: Counter) -> None:
    count.value += 1  # no await, so no other task can run in the middle


async def add_with_await(count: Counter) -> None:
    value = count.value
    await asyncio.sleep(0)  # every other task reads the old value here
    count.value = value + 1


async def main() -> None:
    print("three tasks, each pausing at every await")
    # gather runs the tasks at the same time, on this one thread, and
    # returns their results in the order they were given.
    results = await asyncio.gather(task("a", 2), task("b", 3), task("c", 1))
    print("results:", ", ".join(str(result) for result in results))

    print("4 tasks add 1 to a shared count")
    count = Counter()
    await asyncio.gather(*(add_at_once(count) for _ in range(4)))
    print(f"with no await between reading and writing: {count.value}")
    count = Counter()
    await asyncio.gather(*(add_with_await(count) for _ in range(4)))
    print(f"with an await between reading and writing: {count.value}")


if __name__ == "__main__":
    # asyncio.run starts an event loop, runs main until it finishes, and
    # closes the loop.
    asyncio.run(main())
