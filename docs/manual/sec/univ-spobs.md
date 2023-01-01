## System Objects (Spobs)

You can either create spobs manually by copying and pasting existing spobs and editing them (make sure to add them to a system!), or create and manipulate them with the in-game editor (requires `devmode = true` in the config file or running naev with `--devmode`). Note that the in-game editor doesn't support all the complex functionality, but does a large part of the job such as choosing graphics and positioning the spobs.

### System Editor

TODO

### Spob Classes

Naev planetary classes are based on [Star Trek planetary classes](https://stexpanded.fandom.com/wiki/Planet_classifications).

#### Station classes:

* Class 0: Civilian stations and small outposts
* Class 1: Major military stations and outposts
* Class 2: Pirate strongholds
* Class 3: Robotic stations
* Class 4: Artificial ecosystems such as ringworlds or discworlds

#### Planet classes:

* Class A: Geothermal (partially molten)
* Class B: Geomorteus (partially molten, high temperature; Mercury-like)
* Class C: Geoinactive (low temperature)
* Class D: Asteroid/Moon-like (barren with no or little atmosphere)
* Class E: Geoplastic (molten, high temperature)
* Class F: Geometallic (volcanic)
* Class G: Geocrystaline (crystalizing)
* Class H: Desert (hot and arid, little or no water)
* Class I: Gas Giant (comprised of gaseous compounds, Saturn-like)
* Class J: Gas Giant (comprised of gaseous compounds, Jupiter-like)
* Class K: Adaptable (barren, little or no water, Mars-like)
* Class L: Marginal (rocky and barren, little water)
* Class M: Terrestrial (Earth-like)
* Class N: Reducing (high temperature, Venus-like)
* Class O: Pelagic (very water-abundant)
* Class P: Glaciated (very water-abundant with ice)
* Class Q: Variable
* Class R: Rogue (temperate due to geothermal venting)
* Class S: Ultragiant (comprised of gaseous compounds)
* Class X: Demon (very hot and/or toxic, inhospitable)
* Class Y: Toxic (very hot and/or toxic, inhospitable, containing valuable minerals)
* Class Z: Shattered (formerly hospitable planet which has become hot and/or toxic and inhospitable)

### Spob XML

TODO

### Spob Tags \naev

Tags are a versatile way to define the main facets of interest about a spob with respect to its faction, i.e. what differentiates it from the other spobs the player will (try and) visit.

Tags consist of binary labels which are accessible through the Lua API with `spob.tags()`. They are meant to give indication of the type of spob, and are meant to be used by missions and scripts to, for example, get specific spobs such as Dvaered mining worlds to send the player to get mining equipment or the likes.

Tags can be defined by the following part of XML code:
```xml
 <tags>
  <tag>research</tag>
 </tags>
```
where the above example would signify the spob is focused on research.

#### Special Tags

These tags significantly change the functionality of the spob:

* **restricted**: player should not normally have access here, and normal missions shouldn't spawn or try to come to the spob
* **nonpc**: there should be no normal generic NPCs spawning at the spaceport bar
* **nonews**: there is no news at the spaceport bar

#### Descriptive Tags

Below is the complete list of dominantly used descriptive tags. It should be noted that tagging is incomplete at present and it is possible that none of these tags will apply to many spobs (e.g. uninhabited, average, uninteresting or deserted spobs). Most others will only have one or two tags - they are supposed to represent important facets of the spob in its own estimation, not minor elements e.g. while the (temporary) Imperial Homeworld has many criminals and military personnel neither tag applies since its defining tags would be rich, urban and maybe tourism or trade.

* **station**: the spob is a space station or gas giant spaceport
* **wormhole**: the spob is a wormhole
* **hypergate**: the spob is a hypergate
* **active**: the spob is active (currently only matters for hypergates)
* **ruined**: the spob is ruined (currently only matters for hypergates)
* **new**: recently colonised worlds / recently built stations (definitely post-Incident)
* **old**: long-time colonised worlds / old stations (definitely pre-Incident)
* **rich**: the population living on the spob is rich by the standards of the faction
* **poor**: the population living on the spob is poor by the standards of the faction
* **urban**: the spob consists of mainly heavily developed cities and urban environments
* **rural**: the spob consists of mainly undeveloped and virgin lands
* **tourism**: spob has interests and draws in tourists
* **mining**: mining is an important part of the spob economy
* **agriculture**: agriculture is an important part of the spob economy
* **industrial**: industry is an important part of the spob economy
* **medical**: medicine is an important part of the spob economy
* **trade**: trade is an important part of the spob economy
* **shipbuilding**: shipbuilding is an important part of the spob economy
* **research**: the spob has a strong focus in research (special research laboratories, etc...)
* **immigration**: the spob draws in a large number of immigrants or is being colonised
* **refuel**: the spobs reason for existance is as a fueling point
* **government**: the spob has important government functions or hosts the central government
* **military**: the spob has an important factional military presence
* **religious**: the spob has an important religious significance or presence
* **prison**: the spob has important prison installations
* **criminal**: the spob has a large criminal element such as important pirate or mafia presence

### Lua Scripting

TODO

### Techs

TODO
