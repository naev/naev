## Space Objects (Spobs)

Space Objects (Spobs) are, as their name indicates, objects located around space that the player can target and try to land on. In general, most Spobs are either planets or stations where the player can access different services. However, they can also have much more complex behaviours with Lua programming such as wormholes.

### Creating Spobs

You can either create spobs manually by copying and pasting existing spobs and editing them (make sure to add them to a system!), or create and manipulate them with the in-game editor (requires `devmode = true` in the config file or running naev with `--devmode`). Note that the in-game editor doesn't support all the complex functionality, but does a large part of the job such as choosing graphics and positioning the spobs.

### Special features

There are a few special features a spob can have including:

- Be not landable (the services section should be empty - `<services/>` )
- Special Tags (see below)
- Altered landing requirements - `<land>**requirement**</land>`
**requirement** should be one of:
- land_lowclass (poor, easier to bribe),
- land_hiclass (rich, not bribable),
- emp_mil_restricted (Empire Military spob),
- srs_mil_restricted (Sirius military spob),
- dv_mil_restricted (Dvaered military spob),
- srm_mil_restricted (Soromid military spob),
- zlk_mil_restricted (Za'lek's military spob),
- pir_clanworld (Pirate clanworld),
- something else you have programmed in `dat/landing.lua` !

### Tags

Tags are a versatile way to define the main facets of interest about a spob with respect to its faction, i.e. what differentiates it from the other spobs the player will (try and) visit (many spobs are not ever landable, though they also have nothing of interest to them so no loss!)

```xml
 <tags>
  <tag>research</tag>
 </tags>
```

#### Special Tags

Below are a set of tags that change the behaviour of some core mechanics.

* **restricted**: player should not normally have access here, and normal missions shouldn't spawn or try to come to the spob
* **nonpc**: there should be no normal generic NPCs spawning at the spaceport bar

#### Descriptive Tags

Below is the complete list of dominantly used descriptive tags. It should be noted that tagging is incomplete at present and it is possible that none of these tags will apply to many spobs (e.g. uninhabited, average, uninteresting or deserted spobs). Most others will only have one or two tags - they are supposed to represent important facets of the spob in its own estimation, not minor elements e.g. while the (temporary) Imperial Homeworld has many criminals and military personnel neither tag applies since its defining tags would be rich, urban and maybe tourism or trade.

* **station**: the spob is a space station or gas giant spaceport
* **wormhole**: the spob is a wormhole
* **hypergate**: the spob is a hypergate
* **active**: the spob is active (only matters for hypergates)
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
