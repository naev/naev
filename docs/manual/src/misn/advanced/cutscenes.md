# Cutscenes

Cutscenes are a powerful way of conveying events that the player may or may not interact with.
In order to activate cinematic mode, you must use `player.cinematics` function.
However, the player will still be controllable and escorts will be doing their thing.
If you want to make the player and escorts stop and be invulnerable, you can use the `cinema` library.
In particular, the `cinema.on` function enables cinema mode and `cinema.off` disables it.

You can also control where the camera is with `camera.set()`.
By default, it will try to centre the camera on the player, but if you pass a position or pilot as a parameter, it will move to the position or follow the pilot, respectively.

The cornerstone of cutscenes is to use hooks to make things happen and show that to the player.
In this case, one of the most useful hooks is the `hook.timer` timer hook.
Let us put it all together to do a short example.

```lua
local cinema = require "cinema" -- load the cinema library
...
local someguy -- assume some pilot is stored here

-- function that starts the cutscene
function cutscene00 ()
    cinema.on()
    camera.set( someguy ) -- make the camera go to someguy
    hook.timer( 5, "cutscene01" ) -- advance to next step in 5 seconds
end
function cutscene01 ()
    someguy:broadcast(_("I like cheese!"),true) -- broadcast so the player can always see it
    hook.timer( 6, "cutscene02" ) -- give 6 seconds for the player to see
end
function cutscene02 ()
    cinema.off()
    camera.set()
end
```

Breaking down the example above, the cutscene itself is made of 3 functions.
The first `cutscene00` initializes the cinematic mode and sets the camera to someguy.
Afterwards, `cutscene01` makes someguy say some text and shows it to the player. Finally, in `cutscene02`, the cinematic mode is finished and the camera is returned to the player.

While that is the basics, there is no limit to what can be done.
It is possible to use shaders to create more visual effects, or the luaspfx library.
Furthermore, pilots can be controlled and made to do all sorts of actions. There is no limit to what is possible!
