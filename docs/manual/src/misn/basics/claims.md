# System Claims

One important aspect of mission and event development are system claiming. Claims serve the purpose of avoiding collisions between Lua code. For example, `pilot.clear()` allows removing all pilots from a system. However, say that there are two events going on in a system. They both run `pilot.clear()` and add some custom pilots. What will happen then, is that the second event to run will get rid of all the pilots created from the first event, likely resulting in Lua errors. This is not what we want is it? In this case, we would want both events to try to claim the system and abort if the system was already claimed.

Systems can be claimed with either `misn.claim` or `evt.claim` depending on whether they are being claimed by a mission or an event. A mission or event can claim multiple systems at once, and claims can be exclusive (default) or inclusive. Exclusive claims don't allow any other system to claim the system, while inclusive claims can claim the same system. In general, if you use things like `pilot.clear()` you should use exclusive claims, while if you don't mind if other missions / events share the system, you should use inclusive claims. **You have to claim all systems that your mission uses to avoid collisions!**

Let us look at the standard way to use claims in a mission or event:

```lua
function create ()
   if not misn.claim( {system.get("Gamma Polaris")} ) then
      misn.finish(false)
   end

   -- ...
end
```

The above mission tries to claim the system `"Gamma Polaris"` right away in the `create` function. If it fails and the function returns false, the mission then finishes unsuccessfully with `misn.finish(false)`. This will cause the mission to only start when it can claim the `"Gamma Polaris"` system and silently fail otherwise. You can pass more systems to claim them, and by default they will be *exclusive* claims.

Say our event only adds a small derelict in the system, and we don't mind it sharing the system with other missions and events. Then we can write the event as:

```lua
function create ()
   if not evt.claim( {system.get("Gamma Polaris")}, true ) then
      evt.finish(false)
   end

   -- ...
end
```

In this case, the second parameter is set to `true` which indicates that this event is trying to do an **inclusive** claim. Again, if the claiming fails, the event silently fails.

Claims can also be tested in an event/mission-neutral way with `naev.claimTest`. However, this can only test the claims. Only `misn.claim` and `evt.claim` can enforce claims for missions and events, respectively.

As missions and events are processed by `priority`, make sure to give higher priority to those that you want to be able to claim easier. Otherwise, they will have difficulties claiming systems and may never appear to the player. Minimizing the number of claims and cutting up missions and events into smaller parts is also a way to minimize the amount of claim collisions.
