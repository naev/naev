# Missions and Events

Naev missions and events are written in the Lua Programming Language[^lua].
In particular, they use version 5.1 of the Lua programming language.
While both missions and events share most of the same API, they differ in the following ways:

* **Missions:** only a fixed number can be active at a time and they are visible to the player. The player can also abort them at any time. Missions are saved by default. Have exclusive access to the `misn` library and are found in `dat/missions/`.
* **Events:** an infinite amount can be active at a time. They are not visible or shown to the player in any way, however, their consequences can be seen by the player. By default, they are *not saved to the player savefile*. If you want the event to be saved you have to explicitly do it with `evt.save()`. Have exclusive access to the `evt` library and are found in `dat/events/`.

The general rule of thumb when choosing which to make is that if you want the player to have control, use a mission, otherwise use an event.
Example missions include cargo deliveries, system patrols, etc.
On the other hand, most events are related to game internals and cutscenes such as the save game updater event (`dat/events/updater.lua`) or news generator event (`dat/events/news.lua`).

A full overview of the Lua API can be found at [naev.org/api](https://naev.org/api) and is out of the scope of this document.

[^lua]: https://www.lua.org/

## Getting Started

Missions and events share the same overall structure in which there is a large Lua comment at the top containing all sorts of meta-data, such as where it appears, requirements, etc.
Once the mission or event is started, the obligatory `create` function entry point is run.

Let us start by writing a simple mission header. This will be enclosed by long lua comments `--[[` and `--]]` in the file. Below is our simple header.

```xml
<mission name="My First Mission">
 <avail>
  <chance>50</chance>
  <location>Bar</location>
 </avail>
</mission>
```

The mission is named "My First Mission" and has a 50\% chance of appearing in any spaceport bar. For more information on headers refer to Section \ref{sec:misn-headers}.
