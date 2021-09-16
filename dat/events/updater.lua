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


-- Runs on saves older than 0.9.0
function updater090 ()
   -- Changed how the FLF base diff stuff works
   if diff.isApplied("flf_dead") and diff.isApplied("FLF_base") then
      diff.remove("FLF_base")
   end

   -- Set up pirate faction
   local fpir = faction.get("Pirate")
   local pirmod = fpir:playerStanding()
   local pirate_clans = {
      faction.get("Wild Ones"),
      faction.get("Raven Clan"),
      faction.get("Black Lotus"),
      faction.get("Dreamer Clan"),
   }
   local maxval = -100
   for k,v in ipairs(pirate_clans) do
      local vs = v:playerStanding() -- Only get first parameter
      local vsd = v:playerStandingDefault()
      -- We'll be kind and set the player's pirate standing for the clans
      -- to be positive if the player was doing well with pirates before
      if pirmod > 0 and vs==vsd and not v:isKnown() then
         v:setPlayerStanding( fpir+20 )
      end
      local vs = v:playerStanding()
      maxval = math.max( maxval, vs )
   end
   -- Pirates and marauders are fixed offsets
   fpir:setPlayerStanding( maxval - 20 )
   faction.get("Marauder"):setPlayerStanding( maxval - 40 )

   -- Some previously known factions become unknown
   faction.get("Traders Guild"):setKnown(false)
   if not var.peek("disc_collective") then
      faction.get("Collective"):setKnown(false)
   end
   if not var.peek("disc_proteron") then
      local pro = faction.get("Proteron")
      pro:setKnown(false)
      pro:setPlayerStanding(-50) -- Hostile by default
   end
end

function create ()
   local game_version, save_version = naev.version()

   -- Run on saves older than 0.9.0
   if not save_version or naev.versionTest( save_version, "0.9.0" ) < 0 then
      updater090()
   end

   -- Done
   evt.finish()
end
