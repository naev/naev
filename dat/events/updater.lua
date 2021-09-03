--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Version Updater">
 <trigger>load</trigger>
 <chance>100</chance>
</event>
--]]

--[[
   Small updater to handle moving saves to newer versions.
--]]

function create ()
   -- Changed how the FLF base diff stuff works
   if diff.isApplied("flf_dead") and diff.isApplied("FLF_base") then
      diff.remove("FLF_base")
   end

   -- Set up pirate faction
   local pirate_clans = {
      faction.get("Wild Ones"),
      faction.get("Raven Clan"),
      faction.get("Black Lotus"),
      faction.get("Dreamer Clan"),
   }
   local maxval = -100
   for k,v in ipairs(pirate_clans) do
      local vs = v:playerStanding() -- Only get first parameter
      maxval = math.max( maxval, vs )
   end
   -- Pirates and marauders are fixed offsets
   faction.get("Pirate"):setPlayerStanding(   maxval - 20 )
   faction.get("Marauder"):setPlayerStanding( maxval - 40 )

   -- Done
   evt.finish()
end
