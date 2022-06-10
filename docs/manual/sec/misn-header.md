## Headers
\label{sec:misn-headers}

Headers contain all the necessary data about a mission or event to determine where and when they should be run. They are written as XML code embedded in a Lua comment at the top of each individual mission or event. In the case a lua file does not contain a header, it is ignored and not loaded as a mission or event.

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
</mission>
--]]
```

Let us go over the different parameters. First of all, either a `<mission>` or `<event>` node is necessary as the root for either missions (located in `dat/missions/`) or events (located in `dat/events/`). The `name` attribute has to be set to a unique string and will be used to identify the mission.

Next it is possible to identify mission properties. In particular, only the `<unique />` property is supported, which indicates the mission can only be completed once. It will not appear agian to the same player.

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

Finally, notice that it is also possible to define arbitrary tags in the `<tags>` node. This can be accessed with `player.misnDoneList()` and can be used for things such as handling faction standing caps automatically.
