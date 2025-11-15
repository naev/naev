# Event-Mission Communication

In general, events and missions are to be seen as self-contained isolated entities, that is, they do not affect each other outside of mission variables.
However, it is possible to exploit the `hook` module API to overcome this limitation with `hook.custom` and `naev.trigger`:

* `hook.custom`: allows defining an arbitrary hook on an arbitrary string.
  The function takes two parameters: the first is the string to hook (should not collide with standard names), and the second is the function to run when the hook is triggered.
* `naev.trigger`: also takes two parameters and allows triggering the hooks set by `hook.custom`. In particular, the first parameter is the same as the first parameter string passed to `hook.custom`, and the second optional parameter is data to pass to the custom hooks.

For example, you can define a mission to listen to a hook as below:

```lua
function create ()
   -- ...

   hook.custom( "my_custom_hook_type", "dohook" )
end

function dohook( param )
   print( param )
end
```

In this case, `"my_custom_hook_type"` is the name we are using for the hook.
It is chosen to not conflict with any of the existing names.
When the hook triggers, it runs the function `dohook` which just prints the parameter.
Now, we can trigger this hook from anywhere simply by using the following code:

```lua
   naev.trigger( "my_custom_hook_type", some_parameter )
```

The hook will not be triggered immediately, but the second the current running code is done to ensure that no Lua code is run in parallel.
In general, the mission variables should be more than good enough for event-mission communication, however, in the few cases communication needs to be more tightly coupled, custom hooks are a perfect solution.
