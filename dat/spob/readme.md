## Space Objects (Spobs)

Space Objects (Spobs) are, as their name indicates, objects located around space that the player can target and try to land on. In general, most Spobs are either planets or stations where the player can access different services. However, they can also have much more complex behaviours with Lua programming such as wormholes.

### Creating Spobs

You can either create spobs manually by copying and pasting existing spobs and editing them (make sure to add them to a system!), or create and manipulate them with the in-game editor (requires `devmode = true` in the config file or running naev with `--devmode`). Note that the in-game editor doesn't support all the complex functionality, but does a large part of the job such as choosing graphics and positioning the spobs.

### Tags

Tags are a versatile way to

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

Below is the complete list of dominantly used descriptive tags.

* **tourism**: spob has interests and draws in tourists
* **rich**: the population living on the spob is rich by the standards of the faction
* **poor**: the population living on the spob is poor by the standards of the faction
* **urban**: the spob consists of mainly heavily developed cities and urban environments
* **rural**: the spob consists of mainly undeveloped and virgin lands
* **research**: the spob has a strong focus in research (special research laboratories, etc...)
* **mining**: mining is an important part of the spob economy
* **agriculture**: agriculture is an important part of the spob economy
* **industrial**: industry is an important part of the spob economy
* **trade**: trade is an important part of the spob economy
* **military**: the spob has an important factional military presence
* **prison**: the spob has important prison installations
* **criminal**: the spob has a large criminal element such as important pirate or mafia presence
