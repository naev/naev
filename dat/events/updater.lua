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
local pir = require 'common.pirate'
local tut = require 'common.tutorial'
local vn  = require 'vn'
local fmt = require 'format'

-- Runs on saves older than 0.9.0
local function updater090 ()
   -- Changed how the FLF base diff stuff works
   if diff.isApplied("flf_dead") and diff.isApplied("FLF_base") then
      diff.remove("FLF_base")
   end

   -- Set up pirate faction
   local fpir = faction.get("Pirate")
   local pirmod = fpir:playerStanding()
   for k,v in ipairs(pir.factions_clans) do
      local vs = v:playerStanding() -- Only get first parameter
      local vsd = v:playerStandingDefault()
      -- We'll be kind and set the player's pirate standing for the clans
      -- to be positive if the player was doing well with pirates before
      if pirmod > 0 and vs==vsd and not v:isKnown() then
         v:setPlayerStanding( fpir+20 )
      end
   end
   pir.updateStandings() -- Update pirate/marauder

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
   local fflf = faction.get("FLF")
   fflf:setKnown(false)
   if var.peek("disc_frontier") or player.misnDone("Deal with the FLF agent") or player.misnDone("Take the Dvaered crew home")  then
      fflf:setKnown(true)
   end

   -- Do update tutorial
   if not var.peek( "tut_update" ) then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      -- TODO explain new mechanics:
      -- - introduce ship ai
      -- - electronic warfare (stealth, scanning)
      -- - outfit balance
      -- - safe lanes
      vn.na("Suddenly, a hologram materializes in front of you.")
      sai(fmt.f(_([["Hello there {playername}! I'm your Ship AI. Up until now I've been resident in your ship controlling your Autonav and other functionality, but with new updates, I can now materializes and communicate directly to you as a hologram."
They stare at you for a few seconds.
"Say, now that I can see you, you look very familiar. You wouldn't be related my late previous owner? Terrible what happened…"]]),{playername=player.name()}))
      sai(_([["I'm sure you have many questions about the update, but first, would you like to give me a name?"]]))
      local ainame
      vn.func( function ()
         -- TODO integrate into vn
         ainame = tk.input( _("Name Ship AI"), 1, 16, _("Please enter a name for your Ship AI") )
         if ainame then
            var.push("shipai_name",ainame)
            sai.displayname = ainame -- Can't use rename here

            if tut.specialnames[ string.upper(ainame) ] then
               vn.jump("specialname")
               return
            end
         end
         vn.jump("gavename")
      end )
      vn.label("specialname")
      sai( function () return tut.specialnames[ string.upper(ainame) ] end )

      vn.label("gavename")
      sai( function () return fmt.f(_([["Great! I'll use the name {ainame} from now on. If you want to change it, you can do so from the #oInformation#0 menu which you open with {infokey} by clicking on the '#oShip AI#0' button. From there you can also access explanations and change tutorial options."]]), {ainame=ainame, infokey=tut.getKey("info")}) end )
      sai(fmt.f(_([["With the update, a lot of new mechanics and features have been changed. The largest change includes a revamp of #oElectronic Warfare#0, which now includes a new stealth mechanic. In this new framework, you will be scanned by patrol ships, which means you have to be careful when carrying illegal cargo or outfits. You can activate stealth with {stealthkey} when no ships area nearby."]]),{stealthkey=tut.getKey("stealth")}))
      sai(_([["You may have also noticed that there has been a major change in outfits. Lots of outfits have been removed, added, or renamed, leading to a loss of outfits when updating old save games. Please make sure to take some time inspecting your ships and their equipment before taking off, you don't want to be flying a poorly equipped ship in space!"]]))
      sai(_([["There are also a lot of other changes, for example, there are now patrol routes in systems that are more heavily guarded by local factions, seeking missiles can be jammed, and some factions have been completely reworked. You will notice a lot of smaller differences too as you play the game."]]))
      sai(_([["With that said, would you like me to provide small in-game advice as you do things throughout the game? Some might refer to things you are already familiar with, but it could help learn new things."]]))
      vn.menu{
         {_("Enable tutorial hints"), "enable"},
         {_("Disable tutorial hints"), "disable"},
      }

      vn.label("enable")
      sai(fmt.f(_([["Great! I'll be giving you short hints as you do things through the game. If you want to change my settings or turn off the hints, please do so from the '#oShip AI#0' button in the #oInformation#0 menu you can open with {infokey}. Now, let's go adventuring!"]]),{infokey=tut.getKey("info")}))
      vn.done( tut.shipai.transition )

      vn.label("disable")
      vn.func( function ()
         var.push( "tut_disable", true )
      end )
      sai(fmt.f(_([["OK, I will not be giving you any hints. If you want to change my settings, turn on the hints, or get information and advice, please do so from the '#oShip AI#o' button in the #oInformation#0 menu you can open with {infokey}. Now, let's go adventuring!"]]),{infokey=tut.getKey("info")}))

      vn.done( tut.shipai.transition )
      vn.run()

      var.push( "tut_update", "0.9.0" )
   end
end

function create ()
   local _game_version, save_version = naev.version()

   -- Run on saves older than 0.9.0
   if not save_version or naev.versionTest( save_version, "0.9.0" ) < 0 then
      updater090()
   end

   -- Done
   evt.finish()
end
