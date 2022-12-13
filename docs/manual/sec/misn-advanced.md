## Advanced Usage

TODO

### Handling Aborting Missions

When missions are aborted, the `abort` function is run if it exists. Although this function can't stop the mission from aborting, it can be used to clean up the mission stuff, or even start events such as a penalty for quitting halfway through the mission. A representative example is below:

```lua
local vntk = require "vntk"

...

function abort ()
   vntk.msg(_("Mission Failure!"),_([[You have failed the mission, try again next time!]]))
end
```

Not that it is not necessary to run `misn.finish()` nor any other clean up functions; this is all done for you by the engine.

### Dynamic Factions

TODO

### Minigames
\label{sec:misn-adv-minigames}

TODO

### Cutscenes

Cutscenes are a powerful of conveying events that the player may or may not interact with. In order to activate cinematic mode, you must use `player.cinematics` function. However, the player will still be controllable and escorts will be doing their thing. If you want to make the player and escorts stop and be invulnerable, you can use the `cinema` library. In particular, the `cinema.on` function enables cinema mode and `cinema.off` disables it.

You can also control where the camera is with `camera.set()`. By default, it will try to center the camera on the player, but if you pass a position or pilot as a parameter, it will move to the position or follow the pilot, respectively.

The cornerstone of cutscenes is to use hooks to make things happen and show that to the player. In this case, one of the most useful hooks is the `hook.timer` timer hook. Let us put it all together to do a short example.

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

Breaking down the example above, the cutscene itself is made of 3 functions. The first `cutscene00` initializes the cinematic mode and sets the camera to someguy. Afterwards, `cutscene01` makes someguy same some text and shows it to the player. Finally, in `cutscene02`, the cinematic mode is finished and the camera is returned to the player.

While that is the basics, there is no limit to what can be done. It is possible to use shaders to create more visual effects, or the luaspfx library. Furthermore, pilots can be controlled and made to do all sorts of actions. There is no limit to what is possible!

### Unidiff

TODO

### Equipping with `equipopt`

TODO

### Event-Mission Communication

In general, events and missions are to be seen as self-contained isolated entities, that is, they do not affect each other outside of mission variables. However, it is possible to exploit the `hook` module API to overcome this limitation with `hook.custom` and `naev.trigger`:

* `hook.custom`: allows to define an arbitrary hook on an arbitrary string. The function takes two parameters: the first is the string to hook (should not collide with standard names), and the second is the function to run when the hook is triggered.
* `naev.trigger`: also takes two parameters and allows to trigger the hooks set by `hook.custom`. In particular, the first parameter is the same as the first parameter string passed to `hook.custom`, and the second optional parameter is data to pass to the custom hooks.

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

In this case, `"my_custom_hook_type"` is the name we are using for the hook. It is chosen to not conflict with any of the existing names. When the hook triggers, it runs the function `dohook` which just prints the parameter. Now, we can trigger this hook from anywhere simply by using the following code:

```lua
   naev.trigger( "my_custom_hook_type", some_parameter )
```

The hook will not be triggered immediately, but the second the current running code is done to ensure that no Lua code is run in parallel. In general, the mission variables should be more than good enough for event-mission communication, however, in the few cases communication needs to be more tightly coupled, custom hooks are a perfect solution.

### LuaTK API
\label{sec:misn-adv-luatk}

TODO

### Love2D API
\label{sec:misn-adv-love2d}

> LÖVE is an *awesome* framework you can use to make 2D games in Lua. It's free, open-source, and works on Windows, Mac OS X, Linux, Android and iOS.

Naev implements a subset of the [LÖVE](https://love2d.org/) API (also known as Love2D), allowing it to execute many Love2D games out of the box. Furthermore, it is possible to use the Naev API from inside the Love2D to have the games interact with the Naev engine. In particular, the VN (Sec. \ref{sec:misn-basic-vn}), minigames (Sec. \ref{sec:misn-adv-minigames}), and LuaTK (Sec. \ref{sec:misn-adv-luatk}) are implemented using the Love2D API. Many of the core game functionality, such as the boarding or communication menus make use of this API also, albeit indirectly.

The Love2D API works with a custom dialogue window that has to be started up. There are two ways to do this: create a Love2D game directory and run them, or set up the necessary functions and create the Love2D instance. Both are very similar.

The easiest way is to create a directory with a `main.lua` file that will be run like a normal Love2D game. At the current time the Naev Love2D API does not support zip files. An optional `conf.lua` file can control settings of the game. Afterwards you can start the game with:
```lua
local love = require "love"
love.exec( "path/to/directory" )
```
If the directory is a correct Love2D game, it will create a window instead of Naev and be run in that. You can use `love.graphics.setBackgroundColor( 0, 0, 0, 0 )` to make the background transparent, and the following `conf.lua` function will make the virtual Love2D window use the entire screen, allowing you to draw normally on the screen.
```lua
function love.conf(t)
   t.window.fullscreen = true
end
```

The more advanced way to set up Love2D is to directly populate the `love` namespace with the necessary functions, such as `love.load`, `love.conf`. `love.draw`, etc. Afterwards you can use `love.run()` to start the Love2D game and create the virtual window. This way is much more compact and does not require creating a separate directory structure with a `main.lua`.

Please note that while most of the core Love2D 11.4 API is implemented, more niche API and things that depend on external libraries like `love.physics`, `lua-enet`, or `luasocket` are not implemented. If you wish to have missing API added, it is possible to open an issue for the missing API or create a pull request. Also note that there are

#### Differences with Love2D API

Some of the known differences with the Love2D API are as follows:

* You can call images or canvases to render them with `object:draw( ... )` instead of only `love.graphics.draw( obj, ... )`.
* Fonts default to Naev fonts.
* You can use Naev colour strings such as `"#b"` in `love.graphics.print` and `love.graphics.printf`.
* `audio.newSource` defaults to second paramater `"static"` unless specified (older Love2D versions defaulted to `"stream"`, and it must be set explicity in newer versions).
* `love.graphics.setBackgroundColor` uses alpha colour to set the alpha of the window, with 0 making the Love2D window not drawn.
