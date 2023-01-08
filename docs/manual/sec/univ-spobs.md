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

* `<spob>`: Category which encapsulates all tag data relating to the spob.
 * `<lua>`: Runs a Lua script in relation to this spob.
 * `<pos>`: Position of the spob within its parent system, defined by `x` and `y` coordinates relative to the system center.
 * `<GFX>`: Category relating to graphics.
  * `<space>`: Defines the image, in .WEBP format, which represents the spob when travelling through the parent system. The dimensions of the graphic can also influence the area at which a ship can begin its landing sequence.
  * `<exterior>`: Defines the image, in .WEBP format, displayed on the spob's "Landing Main" tab.
 * `<presence>`: Category relating to faction presence, used to generate patrol lanes within the parent system.
  * `<faction>`: Defines the spob's owning or dominant faction.
  * `<base>`: Defines the base presence of the spob. The maximum base presence of all spobs of the same faction is used as the base presence of the faction in the system. For example, if there are two spobs with base 50 and 100 for a faction in a system, the system's base presence for the faction is 100 and the 50 value is ignored.
  * `<bonus>`: Defines the bonus presence of the spob. The bonus presence of all the spobs of the same faction in a system are added together and added to the presence of the system. For example, for a system with a base presence of 100, if there are two spobs with a bonus of 50 each, the total presence becomes $100+50+50=200$.
  * `<range>`: The range at which the presence of the spob extends. A value of 0 indicates that the presence range only extends to the current system, while a presence of 2 would indicate that it extends to up to 2 systems away. The presence falloff is defined as $1-\frac{dist}{range+1}$, and is multiplied to both base presence and bonus presence. For example, a spob with 100 presence and a range of 3  would give 75 presesnce to 1 system away, 50 presence to 2 systems away, and 25 presence to 3 systems away.
 * `<general>`: Category relating to many functions of the spob including world statistics, available services, etc.
  * `<class>`: Defines the spob's planetary or station class as listed above in the Station Classes and Planetary Classes categories above. This may be referenced by missions or scripts.
  * `<population>`: Defines the spob's habitating population.
  * `<hide>`: Modifies the range at which your ship's sensors can first discover the spob. A value of `1` is default range; values greater than 1 make it easier while values between 1 and 0 make it more difficult. A spob with a `hide` value of `0` will automatically reveal themselves to the player upon entering the system.
  * `<services>`: Defines which services are available to the player while landed at the spob.
   * `<land>`: Includes the Landing Main tab and allows the player to land on the spob. A spob without the `land` tag cannot be landed on.
   * `<refuel>`: Refuels the player's ship on landing. A landable spob without this tag will not generate an Autosave (and will warn the player of this) to mitigate the chances of a "soft lock" where the player becomes trapped in a region of systems with no fuel sources and no autosaves prior to entering said region.
   * `<bar>`: Includes the Bar tab, allowing the player to converse with generic or mission-relevant NPCs and view a news feed. Certain spob tags may alter the availability of NPCs and the news.
   * `<missions>`: Includes the Mission Computer tab, where the player can accept generic missions.
   * `<commodity>`: Includes the Commodities Exchange tab, where the player can buy and sell trade goods.
   * `<outfits>`: Includes the Outfitter tab, allowing the player to buy and sell ship outfits. Also grants access to the Equipment tab where the player can swap outfits to and from their active ship.
   * `<shipyard>`: Includes the Shipyard tab, allowing the player to purchase new ships. Grants access to the Equipment tab as above; also allows the player to swap their active and fleet ships and change the oufits on all player-owned ships.
  * `<commodities>`: Declares the spob as having ready access to commodities, independent of the Commodities Exchange service.
  * `<description>`: Text string presented to the player on the Landing Main tab. This text body is perhaps the primary method of presenting the spob's lore to the player.
  * `<bar>`: Text string presented to the player on the Bar tab. Compared to the `description` tag's lore regarding the spob as a whole, this text describes only the Spaceport Bar and its surroundings.
 * `<tech>`: Category which includes Tech Lists, used to define the items in stock at the Outfitter and Shipyard.
  * `<item>`: Includes one Tech List.
 * `<tags>`: Category which includes tags that describe the spob. These tags can be referenced in missions and scripts; see the Spob Tags section below for more information.

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
