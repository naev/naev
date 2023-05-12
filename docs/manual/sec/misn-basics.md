## Basics

In this section we will discuss basic and fundamental aspects of mission and event developments that you will have to take into account in almost all cases.

### Headers
\label{sec:misn-headers}

Headers contain all the necessary data about a mission or event to determine where and when they should be run. They are written as XML code embedded in a Lua comment at the top of each individual mission or event. In the case a Lua file does not contain a header, it is ignored and not loaded as a mission or event.

The header has to be at the top of the file starting with `--[[` and ending with `--]]` which are long Lua comments with newlines. A full example is shown below using all the parameters, however, some are contradictory in this case.

```lua
--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Mission Name">
 <unique />
 <chance>5</chance>
 <location>Bar</location>
 <chapter>[^0]</chapter>
 <spob>Caladan</spob>
 <faction>Empire</faction>
 <system>Delta Pavonis</system>
 <cond>player.credits() &gt; 10e3</cond>
 <done>Another Mission</done>
 <priority>4</priority>
 <tags>
  <some_random_binary_tag />
 </tags>
 <notes />
</mission>
--]]
```

Let us go over the different parameters. First of all, either a `<mission>` or `<event>` node is necessary as the root for either missions (located in `dat/missions/`) or events (located in `dat/events/`). The `name` attribute has to be set to a unique string and will be used to identify the mission.

Next it is possible to identify mission properties. In particular, only the `<unique />` property is supported, which indicates the mission can only be completed once. It will not appear again to the same player.

The header includes all the information about mission availability. Most are optional and ignored if not provided. The following nodes can be used to control the availability:

* **chance**: *required field*. indicates the chance that the mission appears. For values over 100, the whole part of dividing the value by 100 indicates how many instances can spawn, and the remainder is the chance of each instance. So, for example, a value of 320 indicates that 3 instances can spawn with 20\% each.
* **location**: *required field*. indicates where the mission or event can start. It can be one of `none`, `land`, `enter`, `load`, `computer`, or `bar`. Note that not all are supported by both missions and events. More details will be discussed later in this section.
* **unique**: the presence of this tag indicates the mission or event is unique and will *not appear again* once fully completed.
* **chapter**: indicates what chapter it can appear in. Note that this is regular expression-powered. Something like `0` will match chapter 0 only, while you can write `[01]` to match either chapter 0 or 1. All chapters except 0 would be `[^0]`, and such. Please refer to a regular expression guide such as [regexr](https://regexr.com/) for more information on how to write regex.
* **faction**: must match a faction. Multiple can be specified, and only one has to match. In the case of `land`, `computer`, or `bar` locations it refers to the spob faction, while for `enter` locations it refers to the system faction.
* **spob**: must match a specific spob. Only used for `land`, `computer`, and `bar` locations. Only one can be specified.
* **system**: must match a specific system. Only used for `enter` location and only one can be specified.
* **cond**: arbitrary Lua conditional code. The Lua code must return a boolean value. For example `player.credits() &gt; 10e3` would mean the player having more than 10,000 credits. Note that since this is XML, you have to escape `<` and `>` with `&lt;` and `&gt;`, respectively. Multiple expressions can be hooked with `and` and `or` like regular Lua code. If the code does not contain any `return` statements, `return` is prepended to the string.
* **done**: indicates that the mission must be done. This allows to create mission strings where one starts after the next one.
* **priority**: indicates what priority the mission has. Lower priority makes the mission more important. Missions are processed in priority order, so lower priority increases the chance of missions being able to perform claims. If not specified, it is set to the default value of 5.

The valid location parameters are as follows:

| Location | Event | Mission | Description |
| --- |:---:|:---:| --- |
| none     | ✔ | ✔ | Not available anywhere. |
| land     | ✔ | ✔ | Run when player lands |
| enter    | ✔ | ✔ | Run when the player enters a system. |
| load     | ✔ |   | Run when the game is loaded. |
| computer |   | ✔ | Available at mission computers. |
| bar      |   | ✔ | Available at spaceport bars. |

Note that availability differs between events and missions. Furthermore, there are two special cases for missions: `computer` and `bar` that both support an `accept` function. In the case of the mission computer, the `accept` function is run when the player tries to click on the accept button in the interface. On the other hand, the spaceport bar `accept` function is called when the NPC is approached. This NPC must be defined with `misn.setNPC` to be approachable.

Also notice that it is also possible to define arbitrary tags in the `<tags>` node. This can be accessed with `player.misnDoneList()` and can be used for things such as handling faction standing caps automatically.

Finally, there is a `<notes>` section that contains optional meta data about the meta data. This is only used by auxiliary tools to create visualizations of mission maps.

#### Example: Cargo Missions

Cargo missions appear at the mission computer in a multitude of different factions. Since they are not too important, they have a lower than default priority (6). Furthermore, they have 9 independent chances to appear, each with 60\% chance. This is written as `<chance>960</chance>`. The full example is shown below:

```lua
--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Cargo">
 <priority>6</priority>
 <chance>960</chance>
 <location>Computer</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Za'lek</faction>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
```

#### Example: Antlejos

Terraforming antlejos missions form a chain. Each mission requires the previous one and are available at the same planet (Antlejos V) with 100\% chance. The priority is slightly lower than default to try to ensure the claims get through. Most missions trigger on *Land* (`<location>Land</location>`) because Antlejos V does not have a spaceport bar at the beginning. The full example is shown below:

```lua
--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 3">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Land</location>
 <spob>Antlejos V</spob>
 <done>Terraforming Antlejos 2</done>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
```

#### Example: Taiomi

Next is an example of a unique event. The Finding Taiomi event has a 100\% of appearing in the `Bastion` system outside of Chapter 0. It triggers automatically when entering the system (`<location>enter</location>`).

```lua
--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Finding Taiomi">
 <location>enter</location>
 <unique />
 <chance>100</chance>
 <cond>system.cur() == system.get("Bastion")</cond>
 <chapter>[^0]</chapter>
 <notes>
  <campaign>Taiomi</campaign>
 </notes>
</event>
--]]
```

### Memory Model
\label{sec:misn-basic-mem}

By default, variables in Lua scripts are not saved when the player saves the game. This means that all the values you have set up will be cleared if the player saves and loads. This can lead to problems wit scripts that do the following:

```lua
local dest

function create ()
   dest = spob.get("Caladan")

   -- ...

   hook.land( "land" )
end

function land ()
   if spob.cur() == dest then -- This is wrong!
      -- ...
   end
end
```

In the above script, a variable called `dest` is created, and when the mission is created, it gets set to `spob.get("Caladan")`. Afterwards, it gets used in `land` which is triggered by a hook when the player lands. For this mission, the value `dest` will be set as long as the player doesn't save and load. When the player saves and loads, the value `dest` gets set to `nil` by default in the first line. However, upon loading, the `create` function doesn't get run again, while the hook is still active. This means that when the player lands, `spob.cur()` will be compared with `dest` will not have been set, and thus always be false. In conclusion, the player will never be able to finish the mission!

How do we fix this? The solution is the mission/event memory model. In particular, all mission / event instances have a table that gets set called `mem`. This table has the particular property of being *persistent*, i.e., even if the player saves and loads the game, the contents will not change! We can then use this table and insert values to avoid issues with saving and loading games. Let us update the previous code to work as expected with saving and loading.

```lua
function create ()
   mem.dest = spob.get("Caladan")

   -- ...

   hook.land( "land" )
end

function land ()
   if spob.cur() == mem.dest then
      -- ...
   end
end
```

We can see the changes are minimal. We no longer declare the `dest` variable, and instead of setting and accessing `dest`, we use `mem.dest`, which is the `dest` field of the `mem` persistent memory table. With these changes, the mission is now robust to saving and loading!

It is important to note that almost everything can be stored in the `mem` table, and this includes other tables. However, make sure to not create loops or it will hang the saving of the games.

The most common use of the persistent memory table `mem` is variables that keep track of the mission progress, such as if the player has delivered cargo or has talked to a certain NPC.

### Mission Variables

Mission variables allow storing arbitrary variables in save files. Unlike the `mem` per-mission/event memory model, these are per-player and can be read and written by any Lua code. The API is available as part of the [`var` module](https://naev.org/api/modules/var.html).

The core of the `var` module is three functions:

* `var.peek( varname )`: allows to obtain the value of a mission variable called `varname`. If it does not exist it returns `nil`.
* `var.push( varname, value )`: creates a new mission variable `varname` or overwrites an existing mission variable `varname` if it exists with the value `value`. Note that not all data types are supported, but many are.
* `var.pop( varname)`: removes a mission variable.

It is common to use mission variables to store outcomes in mission strings that affect other missions or events. Since they can also be read by any Lua code, they are useful in `<cond>` header statements too.

Supported variable types are `number`, `boolean`, `string`, and `time`. If you want to pass systems and other data, you have to pass it via untranslated name `:nameRaw()` and then use the corresponding `.get()` function to convert it to the corresponding type again.

### Hooks
\label{sec:misn-basic-hooks}

Hooks are the basic way missions and events can interact with the game. They are accessed via the `hook.*` API and basically serve the purpose of binding script functions to specific in-game events or actions. A full list of the hook API is [available here](https://naev.org/api/modules/hook.html) and the API is always available in missions and events. **Hooks are saved and loaded automatically.**

The basics to using hooks is as follows:

```lua
function create ()
   -- ...

   hook.land( "land" )
end

function land ()
   -- ...
end
```

In this example, at the end of the `create` function, the local function `land` is bound to the player landing with `hook.land`. Thus, whenever the player lands, the script function `land` will be run. All hook functions return a hook ID that can be used to remove the hook with `hook.rm`. For example, we can write a slightly more complicated example as such:

```lua
function create ()
   -- ...

   mem.hook_land = hook.land( "land" )
   mem.hook_enter = hook.enter( "enter" )
end

function land ()
   -- ...
end

function enter ()
   hook.rm( mem.hook_land )
   hook.rm( mem.hook_enter )
end
```

The above example is setting up a `land` hook when the player lands, and an `enter` hook, which activates whenever the player enters a system by either taking off or jumping. Both hooks are stored in persistent memory, and are removed when the `enter` function is run when the player enters a system.

Each mission or event can have an infinite number of hooks enabled. Except for `timer` and `safe` hooks, hooks do not get removed when run.

#### Timer Hooks

Timer hooks are hooks that get run once when a certain amount of real in-game time has passed. Once the hook is triggered, it gets removed automatically. If you wish to repeat a function periodically, you have to create a new timer hook. A commonly used example is shown below.

```lua
function create ()
   -- ...

   hook.enter( "enter" )
end

function enter ()
   -- ...

   hook.timer( 5, "dostuff" )
end

function dostuff ()
   if condition then
      -- ...
      return
   end
   -- ...
   hook.timer( 5, "dostuff" )
end
```

In this example, an `enter` hook is created and triggered when the player enters a system by taking off or jumping. Then, in the `enter` function, a 5 second timer hook is started that runs the `dostuff` function when the time is up. The `dostuff` function then checks a condition to do something and end, otherwise it repeats the 5 second hook. This system can be used to, for example, detect when the player is near a pilot or position, or display periodic messages.

#### Pilot Hooks

When it comes to pilots, hooks can also be used. However, given that pilots are not saved, the hooks are not saved either. The hooks can be made to be specific to a particular pilot, or apply to any pilot. In either case, the pilot triggering the hook is passed as a parameter. An illustrative example is shown below:

```lua
function enter ()
   -- ...

   local p = pilot.add( "Llama", "Independent" )
   hook.pilot( p, "death", "pilot_died" )
end

function pilot_died( p )
   -- ...
end
```

In the above example, when the player enters a system with the `enter` function, a new pilot `p` is created, and a `"death"` hook is set on that pilot. Thus, when the pilot `p` dies, the `pilot_dead` function will get called. Furthermore, the `pilot_died` function takes the pilot that died as a parameter.

There are other hooks for a diversity of pilot actions that are documented in [the official API documentation](https://naev.org/api/modules/hook.html#pilot), allowing for full control of pilot actions.

### Translation Support
\label{sec:misn-basic-translation}

Naev supports translation through [Weblate](https://hosted.weblate.org/projects/naev/naev/). However, in order for translations to be used you have to mark strings as translatable. This is done with a [gettext](https://www.gnu.org/software/gettext/) compatible interface. In particular, the following functions are provided:

* `_()`: This function takes a string, marks it as translatable, and returns the translated version.
* `N_()`: This function takes a string, marks it as translatable, however, it returns the *untranslated* version of the string.
* `n_()`: Takes two strings related to a number quantity and return the translated version that matches the number quantity. This is because some languages translate number quantities differently. For example "1 apple", but "2 apple**s**".
* `p_()`: This function takes two strings, the first is a context string, and the second is the string to translate. It returns the translated string. This allows to disambiguate same strings based on context such as `p_( "main menu", "Close" )` and `p_( "some guy", "Close" )`. In this case `"Close"` can be translated differently based on the context strings.

In general, you want to use `_()` and `n_()` to envelope all strings that are being shown to the player, which will allow for translations to work without extra effort. For example, when defining a new mission you want to translate all the strings as shown below:

```lua
misn.setTitle( _("My Mission") )
misn.setDesc( _("You have been asked to do lots of fancy stuff for a very fancy individual. How fancy!") )
misn.setReward( _("Lots of good stuff!") )
```

Note that `_()` and friends all assume that you are inputting strings in English.

It is important to note that strings not shown to the player, e.g., strings representing faction names or ship names, do not need to be translated! So when adding a pilot you can just use directly the correct strings:

```lua
pilot.add( "Hyena", "Mercenary", nil, _("Cool Dude") )
```

Note that the name (`Cool Dude` in this case) does have to be translated!

### Formatting Text
\label{sec:misn-basic-fmt}

An important part of displaying information to the player is formatting text. While `string.format` exists, it is not very good for translations, as the Lua version can not change the order of parameters unlike C. For this purpose, we have prepared the `format` library, which is much more intuitive and powerful than string.format. A small example is shown below:

```lua
local fmt = require "format"

function create ()
   -- ...
   local spb, sys = spob.getS( "Caladan" )
   local desc = fmt.f( _("Take this cheese to {spb} ({sys}), {name}."),
         { spb=spb, sys=sys, name=player.name() } )
   misn.setDesc( desc )
end
```

Let us break down this example. First, we include the library as `fmt`. This is the recommended way of including it. Afterwards, we run `fmt.f` which is the main formatting function. This takes two parameters: a string to be formatted, and a table of values to format with. The string contains substrings of the form `"{foo}"`, that is, a variable name surrounded by `{` and `}`. Each of these substrings is replaced by the corresponding field in the table passed as the second parameter, which are converted to strings. So, in this case, `{spb}` gets replaced by the value of `table.spb` which in this case is the variable `spb` that corresponds to the Spob of `Caladan`. This gets converted to a string, which in this case is the translated name of the planet. If any of the substrings are missing and not found in the table, it will raise an error.

There are additional useful functions in the `format` library. In particular the following:

* `format.number`: Converts a non-negative integer into a human readable number as a string. Gets rounded to the nearest integer.
* `format.credits`: Displays a credit value with the credit symbol ¤.
* `format.reward`: Used for displaying mission rewards.
* `format.tonnes`: Used to convert tonne values to strings.
* `format.list`: Displays a list of values with commas and the word "and". For example `fmt.list{"one", "two", "three"}` returns `"one, two, and three"`.
* `format.humanize`: Converts a number string to a human readable rough string such as `"1.5 billion"`.

More details can be found in the [generated documentation](https://naev.org/api/modules/format.html).

### Colouring Text
\label{sec:misn-basic-colour}

All string printing functions in Naev accept special combinations to change the colour. This will work whenever the string is shown to the player. In particular, the character `#` is used for a prefix to set the colour of text in a string. The colour is determined by the character after `#`. In particular, the following are valid values:

| Symbol | Description |
| --- | --- |
| `#0` | Resets colour to the default value. |
| `#r` | Red colour. |
| `#g` | Green colour. |
| `#b` | Blue colour. |
| `#o` | Orange colour. |
| `#y` | Yellow colour. |
| `#w` | White colour. |
| `#p` | Purple colour. |
| `#n` | Grey colour. |
| `#F` | Colour indicating friend. |
| `#H` | Colour indicating hostile. |
| `#N` | Colour indicating neutral. |
| `#I` | Colour indicating inert. |
| `#R` | Colour indicating restricted. |

Multiple colours can be used in a string such as `"It is a #ggood#0 #rmonday#0!"`. In this case, the word `"good"` is shown in green, and `"monday"` is shown in red. The rest of the text will be shown in the default colour.

While it is possible to accent and emphasize text with this, it is important to not go too overboard, as it can difficult translating. When possible, it is also best to put the colour outside of the string being translated. For example `_("#rred#0")`  should be written as `"#r".._("red").."#0"`.

### System Claiming
\label{sec:misn-basic-claims}

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

Say our event only adds a small derelict in the system and we don't mind it sharing the system with other missions and events. Then we can write the event as:

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

### Mission Cargo

Cargo given to the player by missions using `misn.cargoAdd` is known as **Mission Cargo**. This differs from normal cargo in that only the player's ship can carry it (escorts are not allowed to), and that if the player jettisons it, the mission gets aborted. Missions and events can still add normal cargo through `pilot.cargoAdd` or `player.fleetCargoAdd`, however, only missions can have mission cargo. It is important to note that *when the mission finishes, all associated mission cargos of the mission are also removed!*

The API for mission cargo is fairly simple and relies on three functions:

* `misn.cargoAdd`: takes a commodity or string with a commodity name, and the amount to add. It returns the id of the mission cargo. This ID can be used with the other mission cargo functions.
* `misn.cargoRm`: takes a mission cargo ID as a parameter and removes it. Returns true on success, false otherwise.
* `misn.cargojet`: same as `misn.cargoRm`, but it jets the cargo into space (small visual effect).

#### Custom Commodities

Commodities are generally defined in `dat/commodities/`, however, it is a common need for a mission to have custom cargo. Instead of bloating the commodity definitions, it is possible to create arbitrary commodities dynamically. Once created, they are saved with the player, but will disappear when the player gets rid of them. There are two functiosn to handle custom commodities:

* `commodity.new`: takes the name of the cargo, description, and an optional set of parameters and returns a new commodity. If it already exist, it returns the commodity with the same name. It is important to note that you have to pass *untranslated* strings. However, in order to allow for translation, they should be used with `N_()`.
* `commodity.illegalto`: makes a custom commodity illegal to a faction, and takes the commodity and a faction or table of factions to make the commodity illegal to as parameters. Note that this function only works with custom commodities.

An full example of adding a custom commodity to the player is as follows:

```lua
local c = commodity.new( N_("Smelly Cheese"), N_("This cheese smells really bad. It must be great!") )
c:illegalto( {"Empire", "Sirius"} )
mem.cargo_id = misn.cargoAdd( c, 1 )
-- Later it is possible to remove the cargo with misn.cargoRm( mem.cargo_id )
```

### Ship Log

The Ship Log is a framework that allows recording in-game events so that the player can easily access them later on. This is meant to help players that haven't logged in for a while or have forgotten what they have done in their game. The core API is in the [`shiplog` module](https://naev.org/api/modules/shiplog.html) and is a core library that is always loaded without the need to `require`. It consists of two functions:

* `shiplog.create`: takes three parameters, the first specifies the id of the log (string), the second the name of the log (string, visible to player), and the third is the logtype (string, visible to player and used to group logs).
* `shiplog.append`: takes two parameters, the first specifies the id of the log (string), and second is the message to append. The ID should match one created by `shiplog.create`.

The logs have the following hierarchy: logtype → log name → message. The logtype and log name are specified by `shiplog.create` and the messages are added with `shiplog.append`. Since, by default, `shiplog.create` doesn't overwrite existing logs, it's very common to write a helper log function as follows:

```lua
local function addlog( msg )
   local logid = "my_log_id"
   shiplog.create( logid, _("Secret Stuff"), _("Neutral") )
   shiplog.append( logid, msg )
end
```

You would use the function to quickly add log messages with `addlog(_("This is a message relating to secret stuff."))`. Usually logs are added when important one-time things happen during missions or when they are completed.

### Visual Novel Framework \naev
\label{sec:misn-basic-vn}

The Visual Novel framework is based on the Love2D API and allows for displaying text, characters, and other effects to the player. It can be thought of as a graph representing the choices and messages the player can engage with. The core API is in the [`vn` module](https://naev.org/api/modules/vn.html).

The VN API is similar to existing frameworks such as [Ren'Py](https://renpy.org), in which conversations are divided into scenes with characters. In particular, the flow of engaging the player with the VN framework consists roughly of the following:

1. Clear internal variables (recommended)
1. Start a new scene
1. Define all the characters that should appear in the scene (they can still be added and removed in the scene with `vn.appear` and `vn.disappear`)
1. Run the transition to make the characters and scene appear
1. Display text
1. Jump to 2. as needed or end the `vn`

For most purposes, all you will need is a single scene, however, you are not limited to that. The VN is based around adding nodes which represent things like displaying text or giving the player options. Once the conversation graph defined by the nodes is set up, `vn.run()` will begin execution and *it won't return until the dialogue is done*. Nodes are run in consecutive order unless `vn.jump` is used to jump to a label node defined with `vn.label`. Let us start by looking at a simple example:

```lua
local vn = require "vn" -- Load the library

-- Below would be what you would include when you want the dialogue
vn.clear() -- Clear internal variables
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

Above is a simple example that creates a new scene with a single character (`mychar`), introduces the character with the narrator (`vn.na`), has the character talk, and then gives two choices to the player that trigger different messages. By default the `vn.transition()` will do a fading transition, but you can change the parameters to do different ones. The narrator API is always available with `vn.na`, and once you create a character with `vn.newCharacter`, you can simple call the variable to have the character talk. The character images are looking for in the `gfx/vn/characters/` directory, and in this case it would try to use the file `gfx/vn/characters/mychar.webp`.

Player choices are controlled with `vn.menu` which receives a table where each entry consists of another table with the first entry being the string to display (e.g., `_("Good.")`), and the second entry being either a function to run, or a string representing a label to jump to (e.g., `"good"`). In the case of passing strings, `vn.jump` is used to jump to the label, so that in the example above the first option jumps to `vn.label("good")`, while the second one jumps to `vn.label("bad")`. By using `vn.jump`, `vn.label`, and `vn.menu` it is possible to create complex interactions and loops.

It is recommended to look at existing missions for examples of what can be done with the `vn` framework.

#### `vntk` Wrapper \naev

The full `vn` framework can be a bit verbose when only displaying small messages or giving small options. For this purpose, the [`vntk` module](https://naev.org/api/modules/vntk.html) can simplify the usage, as it is a wrapper around the `vn` framework. Like the `vn` framework, you have to import the library with `require`, and all the functions are blocking, that is, the Lua code execution will not continue until the dialogues have closed. Let us look at some simple examples of `vntk.msg` and `vntk.yesno` below:

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

The code is very simple and requires the library. Then it will display a message, and afterwards, it will display another with a `Yes` and `No` prompt. If the player chooses yes, the first part of the code will be executed, and if they choose no, the second part is executed.

#### Arbitrary Code Execution \naev

It is also possible to create nodes in the dialogue that execute arbitrary Lua code, and can be used to do things such as pay the player money or modify mission variables. Note that you can not write Lua code directly, or it will be executed when the `vn` is being set up. To have the code run when triggered by the `vn` framework, you must use `vn.func` and pass a function to it. A very simple example would be

```lua
-- ...
vn.label("pay_player")
vn.na(_("You got some credits!"))
vn.func( function ()
   player.pay( 50e3 )
end )
-- ...
```

It is also to execute conditional jumps in the function code with `vn.jump`. This would allow to condition the dialogue on things like the player's free space or amount of credits as shown below:

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

As you can guess, `vn.func` is really powerful and opens up all sorts of behaviour. You can also set local or global variables with it, which is very useful to detect if a player has accepted or not a mission.
