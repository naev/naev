# Missions and Events

Naev missions and events are written in the [Lua Programming Language](https://www.lua.org).
In particular, they use version 5.1 of the Lua programming language.
While both missions and events share most of the same API, they differ in the following ways:

* **Missions:** Always visible to the player in the info window.
  The player can also abort them at any time.
  Missions are saved by default.
  Have exclusive access to the `misn` library and are found in `dat/missions/`.
* **Events:** Not visible or shown to the player in any way, however, their consequences can be seen by the player.
  By default, they are *not saved to the player savefile*.
  If you want the event to be saved you have to explicitly do it with `evt.save()`.
  Have exclusive access to the `evt` library and are found in `dat/events/`.

The general rule of thumb when choosing which to make is that if you want the player to have control, use a mission, otherwise use an event.
Example missions include cargo deliveries, system patrols, etc.
On the other hand, most events are related to game internals and cutscenes such as the save game updater event ([`dat/events/updater.lua`](https://github.com/naev/naev/blob/main/dat/events/updater.lua)) or news generator event ([`dat/events/news.lua`](https://github.com/naev/naev/blob/main/dat/events/news.lua)).

A full overview of the Naev Lua API can be found at [naev.org/api](https://naev.org/api) and is out of the scope of this document.

## Mission Guidelines

This following section deals with guidelines for getting missions included into the official [Naev repository](https://github.com/naev/naev).
These are rough guidelines and do not necessarily have to be followed exactly.
Exceptions can be made depending on the context.

1. **Avoid stating what the player is feeling or making choices for them.** The player should be in control of themselves.
1. **There should be no penalties for aborting missions.** Let the player abort/fail and try again.
