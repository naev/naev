## Tips and Tricks

This section includes some useful tricks when designing plugins.

### Making Compatible Changes

While plugins can easily overwrite files, there are times you may not wish to do that, as replacing the same file as another plugin will lead to conflicts. To avoid replacing files, it is possible to use the *Universe Diff* (`unidiff`) system (TODO add section). Let us consider a motivating example where we want to simply add an outfit, and add it to an existing tech group, but not replace the file so it can work with other plugins. This can be done by creating an event that automatically applies the unidiff when the player loads the game.

Assume that we have a tech group called "Base Tech Group", and an outfit called "My Outfit". We wish to add "My Outfit" to "Base Tech Group" without replacing the file. To do this, first we define the unidiff that adds "My Outfit" to "Base Tech Group" as below:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<unidiff name="my_plugin_unidiff">
 <tech name="Base Tech Group">
  <add>My Outfit</add>
 </tech>
</unidiff>
```

The above file should be saved to `unidiff/my_plugin_unidiff.xml`, and will simply add the "My Outfit" to "Base Tech Group". However, this unidiff will be disabled by default, so we'll have to enable it with an event such as below:

```lua
--[[
<?xml version='1.0' encoding='utf8'?>
<event name="My Plugin Start">
 <location>load</location>
 <chance>100</chance>
</event>
--]]
function create ()
   local diffname = "my_plugin_unidiff"
   if not diff.isApplied(diffname) then
      diff.apply(diffname)
   end
   evt.finish()
end
```

The above file should be saved somewhere in `events/`, such as `events/my_plugin_start.lua`, and will simply apply the unidiff "my_plugin_unidiff" if it is not active, thus adding "My Outfit" to "Base Tech Group" without overwriting the file.

The same technique can be done to add new systems or any other feature supported by the unidiff system (see section TODO). For example, you can add new systems normally, but then add the jumps to existing systems in a unidiff so you do not have to overwrite the original files.
