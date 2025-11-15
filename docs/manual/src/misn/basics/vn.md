# Visual Novel Framework

The Visual Novel framework is based on the Love2D API and allows for displaying text, characters, and other effects to the player.
It can be thought of as a graph representing the choices and messages the player can engage with.
The core API is in the [`vn` module](https://naev.org/api/modules/vn.html).

The VN API is similar to existing frameworks such as [Ren'Py](https://renpy.org), in which conversations are divided into scenes with characters.
In particular, the flow of engaging the player with the VN framework consists roughly of the following:

1. Reset internal state (recommended)
1. Start a new scene
1. Define all the characters that should appear in the scene (they can still be added and removed in the scene with `vn.appear` and `vn.disappear`)
1. Run the transition to make the characters and scene appear
1. Display text
1. Jump to 2. as needed or end the `vn`

For most purposes, all you will need is a single scene, however, you are not limited to that.
The VN is based around adding nodes which represent things like displaying text or giving the player options.
Once the conversation graph defined by the nodes is set up, `vn.run()` will begin execution and *it won't return until the dialogue is done*.
Nodes are run in consecutive order unless `vn.jump` is used to jump to a label node defined with `vn.label`.
Let us start by looking at a simple example:

```lua
local vn = require "vn" -- Load the library

-- Below would be what you would include when you want the dialogue
vn.reset() -- Resets internal state
vn.scene() -- Start a new scene
local mychar = vn.newCharacter( _("Alex"), {image="mychar.webp"} )
vn.transition() -- Will fade in the new character
vn.na(_([[You see a character appear in front of you.]]) -- Narrator
mychar(_([[How do you do?]])
vn.menu{ -- Give a list of options the player chooses from
   {_("Good."), "good"},
   {_("Bad."), "bad"},
}

vn.label("good") -- Triggered when the "good" option is chosen
mychar(_("Great!"))
vn.done() -- Finish

vn.label("bad") -- Triggered on "bad" option
mychar(_("That's not good…"))
vn.run()
```

Above is a simple example that creates a new scene with a single character (`mychar`), introduces the character with the narrator (`vn.na`), has the character talk, and then gives two choices to the player that trigger different messages.
By default the `vn.transition()` will do a fading transition, but you can change the parameters to do different ones.
The narrator API is always available with `vn.na`, and once you create a character with `vn.newCharacter`, you can simple call the variable to have the character talk.
The character images are looking for in the `gfx/vn/characters/` directory, and in this case it would try to use the file `gfx/vn/characters/mychar.webp`.

Player choices are controlled with `vn.menu` which receives a table where each entry consists of another table with the first entry being the string to display (e.g., `_("Good.")`), and the second entry being either a function to run, or a string representing a label to jump to (e.g., `"good"`).
In the case of passing strings, `vn.jump` is used to jump to the label, so that in the example above the first option jumps to `vn.label("good")`, while the second one jumps to `vn.label("bad")`.
By using `vn.jump`, `vn.label`, and `vn.menu` it is possible to create complex interactions and loops.

It is recommended to look at existing missions for examples of what can be done with the `vn` framework.

## `vntk` Wrapper

The full `vn` framework can be a bit verbose when only displaying small messages or giving small options.
For this purpose, the [`vntk` module](https://naev.org/api/modules/vntk.html) can simplify the usage, as it is a wrapper around the `vn` framework.
Like the `vn` framework, you have to import the library with `require`, and all the functions are blocking, that is, the Lua code execution will not continue until the dialogues have closed.
Let us look at some simple examples of `vntk.msg` and `vntk.yesno` below:

```lua
local vntk = require "vntk"

-- …
vntk.msg( _("Caption"), _("Some message to show to the player.") )

-- …
if vntk.yesno( _("Cheese?"), _("Do you like cheese?") ) then
   -- player likes cheese
else
   -- player does not
end
```

The code is very simple and requires the library.
Then it will display a message, and afterwards, it will display another with a `Yes` and `No` prompt.
If the player chooses yes, the first part of the code will be executed, and if they choose no, the second part is executed.

## Arbitrary Code Execution

It is also possible to create nodes in the dialogue that execute arbitrary Lua code, and can be used to do things such as pay the player money or modify mission variables.
Note that you can not write Lua code directly, or it will be executed when the `vn` is being set up.
To have the code run when triggered by the `vn` framework, you must use `vn.func` and pass a function to it.
A very simple example would be

```lua
-- ...
vn.label("pay_player")
vn.na(_("You got some credits!"))
vn.func( function ()
   player.pay( 50e3 )
end )
-- ...
```

It is also to execute conditional jumps in the function code with `vn.jump`.
This would allow to condition the dialogue on things like the player's free space or amount of credits as shown below:

```lua
-- ...
vn.func( function ()
   if player.pilot():cargoFree() < 10 then
      vn.jump("no_space")
   else
      vn.jump("has_space")
   end
end )

vn.label("no_space")
-- ...

vn.label("has_space")
-- ...
```

In the code above, a different dialogue will be run depending on whether the player has less than 10 free cargo space or more than that.

As you can guess, `vn.func` is really powerful and opens up all sorts of behaviour.
You can also set local or global variables with it, which is very useful to detect if a player has accepted or not a mission.
