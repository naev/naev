# Artificial Intelligence (AI)

The AI in Naev is mainly a hierarchical finite state machine, where tasks are a FIFO queue. New tasks get added (which can have subtasks) and when done, it goes on with the task before that.

## Memory Parameters

Below are some of the important memory parameters that can affect significantly the behaviour of the AI. These can be set by either accessing the `mem` table from the AI script itself, or accessing it via `pilot.memory()`.

* **natural (boolean)**: Represents a naturally spawning ship. These ships are cleared by missions / events as necessary, and can generally be bribed. They can also be captured when boarded. This parameter is automatically set for ships spawned with the faction spawning scripts. It has to be manually set on pilots spawned elsewhere.
* **capturable (boolean)**: The ship is capturable despite being not necessarily natural.
* **vulnerability (number)**: How vulnerable the ship is. A larger value means the ship is less likely to be targetted. A negative value indicates more likely to be targetted. It's roughly equivalent to points but different AIs will prefer to target different ships so it is not very transparent. Setting a value of `math.huge` will make a pilot usually targetted last, while `-math.huge` will make it be prioritized as much as possible.
