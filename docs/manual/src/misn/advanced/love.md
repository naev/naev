# LÖVE API

> LÖVE is an *awesome* framework you can use to make 2D games in Lua.
> It's free, open-source, and works on Windows, Mac OS X, Linux, Android and iOS.

Naev implements a subset of the [LÖVE](https://love2d.org/) API (also known as Love2D), allowing it to execute many Love2D games out of the box.
Furthermore, it is possible to use the Naev API from inside the Love2D to have the games interact with the Naev engine.
In particular, the [Visual Novel Framework](../basics/vn.md), minigames, etc. are implemented using the Love2D API.
Many of the core game functionality, such as the boarding or communication menus make use of this API also, albeit indirectly.

The Love2D API works with a custom dialogue window that has to be started up.
There are two ways to do this: create a Love2D game directory and run them, or set up the necessary functions and create the Love2D instance.
Both are very similar.

The easiest way is to create a directory with a `main.lua` file that will be run like a normal Love2D game.
At the current time the Naev Love2D API does not support zip files.
An optional `conf.lua` file can control settings of the game.
Afterwards you can start the game with:
```lua
local love = require "love"
love.exec( "path/to/directory" )
```
If the directory is a correct Love2D game, it will create a window instead of Naev and be run in that.
You can use `love.graphics.setBackgroundColor( 0, 0, 0, 0 )` to make the background transparent, and the following `conf.lua` function will make the virtual Love2D window use the entire screen, allowing you to draw normally on the screen.
```lua
function love.conf(t)
   t.window.fullscreen = true
end
```

The more advanced way to set up Love2D is to directly populate the `love` namespace with the necessary functions, such as `love.load`, `love.conf`. `love.draw`, etc.
Afterwards you can use `love.run()` to start the Love2D game and create the virtual window.
This way is much more compact and does not require creating a separate directory structure with a `main.lua`.

Please note that while most of the core Love2D 11.4 API is implemented, more niche API and things that depend on external libraries like `love.physics`, `lua-enet`, or `luasocket` are not implemented.
If you wish to have missing API added, it is possible to open an issue for the missing API or create a pull request.

## Differences with Love2D API

Some of the known differences with the Love2D API are as follows:

* You can call images or canvases to render them with `object:draw( ... )` instead of only `love.graphics.draw( obj, ... )`.
* Fonts default to Naev fonts.
* You can use Naev colour strings such as `"#b"` in `love.graphics.print` and `love.graphics.printf`.
* `audio.newSource` defaults to second paramater `"static"` unless specified (older Love2D versions defaulted to `"stream"`, and it must be set explicity in newer versions).
* `love.graphics.setBackgroundColor` uses alpha colour to set the alpha of the window, with 0 making the Love2D window not drawn.
