# Handling Aborting Missions

When missions are aborted, the `abort` function is run if it exists.
Although this function can't stop the mission from aborting, it can be used to clean up the mission stuff, or even start events such as a penalty for quitting halfway through the mission.
A representative example is below:

```lua
local vntk = require "vntk"

...

function abort ()
   vntk.msg(_("Mission Failure!"),_([[You have failed the mission, try again next time!]]))
end
```

Note that it is not necessary to run `misn.finish()` nor any other clean up functions; this is all done for you by the engine.
