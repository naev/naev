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
 <avail>
  <chance>5</chance>
  <location>Bar</location>
  <chapter>[^0]</chapter>
  <spob>Caladan</spob>
  <faction>Empire</faction>
  <system>Delta Pavonis</system>
  <cond>player.credits() &gt; 10e3</cond>
  <done>Another Mission</done>
  <priority>4</priority>
 </avail>
 <tags>
  <some_random_binary_tag />
 </tags>
 <notes />
</mission>
--]]
```

Let us go over the different parameters. First of all, either a `<mission>` or `<event>` node is necessary as the root for either missions (located in `dat/missions/`) or events (located in `dat/events/`). The `name` attribute has to be set to a unique string and will be used to identify the mission.

Next it is possible to identify mission properties. In particular, only the `<unique />` property is supported, which indicates the mission can only be completed once. It will not appear again to the same player.

The core of the header lies in the `<avail>` node which includes all the information about mission availability. Most are optional and ignored if not provided. The following nodes can be used to control the availability:

* **chance**: *required field*. indicates the chance that the mission appears. For values over 100, the whole part of dividing the value by 100 indicates how many instances can spawn, and the remainder is the chance of each instance. So, for example, a value of 320 indicates that 3 instances can spawn with 20\% each.
* **location**: *required field*. indicates where the mission or event can start. It can be one of `none`, `land`, `enter`, `load`, `computer`, or `bar`. Note that not all are supported by both missions and events. More details will be discussed later in this section.
* **chapter**: indicates what chapter it can appear in. Note that this is regular expression-powered. Something like `0` will match chapter 0 only, while you can write `[01]` to match either chapter 0 or 1. All chapters except 0 would be `[^0]`, and such. Please refer to a regular expression guide such as [regexr](https://regexr.com/) for more information on how to write regex.
* **faction**: must match a faction. Multiple can be specified, and only one has to match. In the case of `land`, `computer`, or `bar` locations it refers to the spob faction, while for `enter` locations it refers to the system faction.
* **spob**: must match a specific spob. Only used for `land`, `computer`, and `bar` locations. Only one can be specified.
* **system**: must match a specific system. Only used for `enter` location and only one can be specified.
* **cond**: arbitrary Lua conditional code. The Lua code must return a boolean value. For example `player.credits() &gt; 10e3` would mean the player having more than 10,000 credits. Note that since this is XML, you have to escape `<` and `>` with `&lt;` and `&gt;`, respectively. Multiple expressions can be hooked with `and` and `or` like regular Lua code.
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

Note that availability differs between events and missions. Furthermore, there are two special cases for missions: `computer` and `bar` that both support an `accept` function. In the case of the mission computer, the `accept` function is run when the player tries to click on the accept button in the interface. On the other hand, the spaceport bar `accept` function is called when the NPC is approached. Note that this NPC must be defined with `misn.setNPC` to be approachable.

Also notice that it is also possible to define arbitrary tags in the `<tags>` node. This can be accessed with `player.misnDoneList()` and can be used for things such as handling faction standing caps automatically.

Finally, there is a `<notes>` section that contains optional meta data about the meta data. This is only used by auxiliary tools to create visualizations of mission maps.

#### Example: Cargo Missions

Cargo missions appear at the mission computer in a multitude of different factions. Since they are not too important, they have a lower than default priority (6). Furthermore, they have 9 independent chances to appear, each with 60\% chance. This is written as `<chance>960</chance>`. The full example is shown below:

```lua
--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Cargo">
 <avail>
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
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
```

#### Example: Antlejos

Terraforming antlejos missions form a chain. Each mission requires the previous one and are available at the same planet (Antlejos V) with 100\% chance. The priority is slightly lower than default to try to ensure the claims get through. Most missions trigger on *Land* because Antlejos V does not have a spaceport bar at the beginning. The full example is shown below:

```lua
--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 3">
 <unique />
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Land</location>
  <spob>Antlejos V</spob>
  <done>Terraforming Antlejos 2</done>
 </avail>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
```

#### Example: Taiomi

Next is an example of a unique event. The Finding Taiomi event has a 100\% of appearing in the `Bastion` system outside of Chapter 0. It triggers automatically when entering the system.

```lua
--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Finding Taiomi">
 <trigger>enter</trigger>
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

### Hooks
\label{sec:misn-basic-hooks}

Hooks are the basic way missions and events can interact with the game. They are accessed via the `hook.*` API and basically serve the purpose of binding script functions to specific in-game events or actions. A full list of the hook API is [available here](https://naev.org/api/modules/hook.html). **Hooks are saved and loaded automatically.**

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

Timer hooks are hooks that get run once when a certain amount of real in-game time has passed. Once the hook is triggered, it gets removed automatically. If you wish to repeat a function periodically, you have to create a new timer hook. A comomnly used example is shown below.

```
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

### Translating
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
pilot.add( "Hyena", "Mercenary" )
```

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
* `format.list`: Displays a list of values with commas and the word "and". For example `{"one", "two", "three"}` becomes `"one, two, and three"`.
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

TODO

### Visual Novel Framework
\label{sec:misn-basic-vn}

TODO

### Mission Computer Missions
\label{sec:misn-basic-computer}

TODO

### Ship Log

TODO
