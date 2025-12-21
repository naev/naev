# Scenario Information `start.toml`

Used to be `start.xml` in order versions, `start.toml` defines some main properties about the game, including things about the player's starting conditions, and defaults about the data.

As it is a [toml](https://toml.io/en/), it is fairly easy to define. Let us look at the possible fields (they are strings and required unless specified):

* **scenario_name**: field that defines the scenario name and will appear when starting up the game. The default scenario is "Sea of Darkness".
* **ship_name**: default name of the starting ship. The player will be able to change this.
* **ship_acquired**: acquired string for the ship that will appear in the equipment view when landed.
* **ship_model**: model of the starting player ship. Should match a ship model name.
* **credits**: starting amount of credits of the player
* **system**: starting star system.
* **system_position**: defines the start location in the system. Should be a 2-dimension vector of numbers such as `[ 1000, 300 ]`.
* **chapter**: starting chapter
* **date**: starting date. Should be defined in the UST format (see in-game holo-archives for detail on time) such as "UST 603:3726.2871".
* **event** (optional): event to run when the game is started. Can be used to change things such as randomize starting credits or the likes.
* **mission** (optional): mission to run when the game is started.
* **gui_default**: default starting GUI.
* **spob_lua_default**: default script to use for spobs.
* **dtype_default**: default damage type to use when not defined.
* **local_map_default**: outfit to use for the local system map.

A full example of the 0.13.0 `start.toml` file is below:

```toml
scenario_name = "Sea of Darkness"
ship_name = "El Ego de Deiz"
ship_acquired = "Your first space-worthy ship!"
ship_model = "Llama"
credits = 30000
system = "Delta Polaris"
system_position = [ -12500, -3500 ]
event = "start_event"
chapter = "0"
date = "UST 603:3726.2871"
gui_default = "slim"
spob_lua_default = "spob/lua/default.lua"
dtype_default = "energy"
local_map_default = "Local System Map"
```

## Making Use of the Starting Event

It is recommended to use the starting event to set up things and introduce the player to your plugin.
The Lua scripting in the starting event can do all sorts of things like change the player's starting ship, make them land, change the credits, etc.
It can also be done conditionally on, for example, the player's starting choices.
