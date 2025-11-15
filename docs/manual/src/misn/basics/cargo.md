# Mission Cargo

Cargo given to the player by missions using `misn.cargoAdd` is known as **Mission Cargo**.
This differs from normal cargo in that only the player's ship can carry it (escorts are not allowed to), and that if the player jettisons it, the mission gets aborted.
Missions and events can still add normal cargo through `pilot.cargoAdd` or `player.fleetCargoAdd`, however, only missions can have mission cargo.
It is important to note that *when the mission finishes, all associated mission cargos of the mission are also removed!*

The API for mission cargo is fairly simple and relies on three functions:

* `misn.cargoAdd`: takes a commodity or string with a commodity name, and the amount to add.
  It returns the id of the mission cargo.
  This ID can be used with the other mission cargo functions.
* `misn.cargoRm`: takes a mission cargo ID as a parameter and removes it.
  Returns true on success, false otherwise.
* `misn.cargojet`: same as `misn.cargoRm`, but it jets the cargo into space (small visual effect).

#### Custom Commodities

Commodities are generally defined in `dat/commodities/`, however, it is a common need for a mission to have custom cargo.
Instead of bloating the commodity definitions, it is possible to create arbitrary commodities dynamically.
Once created, they are saved with the player, but will disappear when the player gets rid of them.
There are two functions to handle custom commodities:

* `commodity.new`: takes the name of the cargo, description, and an optional set of parameters and returns a new commodity.
  If it already exists, it returns the commodity with the same name.
  It is important to note that you have to pass *untranslated* strings.
  However, in order to allow for translation, they should be used with `N_()`.
* `commodity.illegalto`: makes a custom commodity illegal to a faction, and takes the commodity and a faction or table of factions to make the commodity illegal to as parameters.
  Note that this function only works with custom commodities.

An full example of adding a custom commodity to the player is as follows:

```lua
local c = commodity.new( N_("Smelly Cheese"), N_("This cheese smells really bad. It must be great!") )
c:illegalto( {"Empire", "Sirius"} )
mem.cargo_id = misn.cargoAdd( c, 1 )
-- Later it is possible to remove the cargo with misn.cargoRm( mem.cargo_id )
```
