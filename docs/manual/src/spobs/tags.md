# Spob Common Tags

Below are some common tags used by many of the different scripts you can use in Sea of Darkness or with plugins.
It is recommended to use as many tags as possible early in the development cycle, as it is rather tedious to go over all the spobs to add new tags afterwards.

## Special Tags

These tags significantly change the functionality of the spob:

* **restricted**: player should not normally have access here, and normal missions shouldn't spawn or try to come to the spob.
* **nonpc**: There should be no normal generic NPCs spawning at the spaceport bar.
* **nonews**: There is no news at the spaceport bar.
* **nosightseeing**: Will never be a point of interest for sightseeing missions.
* **garbage**: Can be used as a garbage dump for the waste desposal missions.
* **wormhole**: The spob is a wormhole.
  Doesn't really do anything yet though.
* **hypergate**: The spob is a hypergate, is necessary for the spob to be a valid hypergate target.
* **obelisk**: Is a special obelisk that can be used to gain Sirius flow abilities.
* **active**: The spob is active (currently only matters for hypergates) and can be used.
  This allows the hypergate interface to be used and allows hypergates to connect to them.

## Descriptive Tags

Below is the complete list of dominantly used descriptive tags.
It should be noted that tagging is incomplete at present and it is possible that none of these tags will apply to many spobs (e.g. uninhabited, average, uninteresting or deserted spobs).
Most others will only have one or two tags - they are supposed to represent important facets of the spob in its own estimation, not minor elements e.g. while the (temporary) Imperial Homeworld has many criminals and military personnel neither tag applies since its defining tags would be rich, urban and maybe tourism or trade.

These tags allow missions to randomly target spobs with specific properties, so they may have fairly localized specific effects.

* **station**: the spob is a space station or gas giant spaceport
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
* **immigration**: the spob draws in many immigrants or is being colonised
* **refuel**: the spobs reason for existance is as a fueling point
* **government**: the spob has important government functions or hosts the central government
* **military**: the spob has an important factional military presence
* **religious**: the spob has an important religious significance or presence
* **prison**: the spob has important prison installations
* **criminal**: the spob has a large criminal element such as important pirate or mafia presence
* **ringworld**: Is a ring world.
* **toxic**: Is a toxic location, dangerous to stay too long.
* **refuel**: Is mainly used as a refuelling location with few or no other services.
