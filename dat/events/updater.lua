--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Version Updater">
 <location>load</location>
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
local luatk = require "luatk"

-- Runs on saves older than 0.11.0
local function updater0120( did0110, did0100, did090 )
   -- Have to apply diff to lower pirates if necessary
   if player.chapter()=="0" then
      if not diff.isApplied( "Chapter 0" ) then
         diff.apply( "Chapter 0" )
      end
   else
      diff.remove( "Chapter 0" )
   end

   local metai = (var.peek("shipai_name") ~= nil)
   local hasbioship = player.pilot():ship():tags().bioship
   for k,v in ipairs(player.ships()) do
      if v.ship:tags().bioship then
         hasbioship = true
         break
      end
   end

   -- Player may have not met the proteron but have positive standing due to bugs in how the reputation worked
   -- Reset standing to hostile unless player is in proteron system
   local fproteron = faction.get("Proteron")
   if not fproteron:known() or system.cur():faction()~=fproteron then
      fproteron:setReputationGlobal( fproteron:reputationDefault() )
   end

   -- Do update tutorial, have to handle older versions here
   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   if not metai then
      vn.na(_([[Suddenly, a hologram materializes in front of you.]]))
      sai(fmt.f(_([["Hello there {playername}! I'm your Ship AI. Up until now I've been resident in your ship controlling your Autonav and other functionality, but with new updates, I can now materialize and communicate directly with you as a hologram."
They stare at you for a few seconds.
"Say, now that I can see you, you look very familiar. You wouldn't be related my late previous owner? Terrible what happened…"]]),{playername=player.name()}))
      sai(_([["I'm sure you have many questions about the update, but first, would you like to give me a name?"]]))
      vn.label("rename")
      local ainame
      luatk.vn( function ()
         luatk.msgInput( _("Name Ship AI"), _("Please enter a name for your Ship AI"), 50, function( str )
            ainame = str
            if ainame then
               var.push("shipai_name",ainame)
               sai.displayname = ainame -- Can't use rename here

               if tut.specialnames[ string.upper(ainame) ] then
                  vn.jump("specialname")
                  return
               end
               vn.jump("gavename")
               return
            end
            vn.jump("noname")
         end )
      end )
      vn.label("specialname")
      sai( function () return tut.specialnames[ string.upper(ainame) ] end )

      vn.label("noname")
      sai(_([["You haven't given me a name. I will be continued to be called 'Ship AI'. Is that OK?"]]))
      vn.menu{
         {_([["'Ship AI' is fine"]]), "gavename"},
         {_("Rename"), "rename"},
      }

      vn.label("gavename")
      sai( function () return fmt.f(_([["Great! I'll use the name {ainame} from now on. If you want to change it, you can do so from the #oInformation#0 window which you open with {infokey} by clicking on the '#oShip AI#0' button. From there you can also access explanations and change tutorial options."]]),
         {ainame=tut.ainame(), infokey=tut.getKey("info")}) end )
   else
      vn.na(fmt.f(_([[Your ship AI {shipai} materializes before you.]]),
         {shipai=tut.ainame()}))
      sai(_([["It seems like there has been a significant update to the game!
Many of the new features come with small tutorials in form of missions. I will not go over these to not spoil you as they will appear in-game. Let me cover some of the features that you may miss."]]))
   end

   -- 0.9.0 changes
   if did090 then
      sai(fmt.f(_([["With the update, a lot of new mechanics and features have been changed. The largest change includes a revamp of #oElectronic Warfare#0, which now includes a new stealth mechanic. In this new framework, you will be scanned by patrol ships, which means you have to be careful when carrying illegal cargo or outfits. You can activate stealth with {stealthkey} when no ships area nearby."]]),
         {stealthkey=tut.getKey("stealth")}))
   end

   -- 0.10.0 changes
   if did0100 then
      sai(_([["Asteroids have been completely reworked. They no longer explode randomly, and asteroid fields can spawn different types of asteroids. While it is possible to mine them with regular weapons, there are special outfits that will help you mine them and give you access to more rare rewards."]]))
      sai(_([["You may have also noticed that there has been a major change in outfits. Lots of outfits have been removed, added, or renamed, leading to a loss of outfits when updating old save games. Please make sure to take some time inspecting your ships and their equipment before taking off, you don't want to be flying a poorly equipped ship in space!"]]))
      if hasbioship then
         sai(_([["Bioships have also been reworked completely. Similar to the old ships, they gain ranks through experience. However, instead of the ranks being on a per-outfit level, they are now on per-ship levels. Increasing ranks will give you better core outfits and weapons, while also unlocking skill points that you can use to significantly change the functionality and performance of the bioship."]]))
      end
      --sai(_([["This update also provides significant modernization of the engine and many other features. To list a few, the star map is larger and you can save system notes, many new missions and campaigns, health bars are shown for pilots in combat, backgrounds reworked, news and NPCs reworked, unique pilots appear throughout the universe, manual aiming mode, save snapshots, difficulty settings, etc. Some features can be toggled through the #oOptions#0 window, so make sure to check that if interested."]]))
   end

   -- 0.11.0 changes
   if did0110 then
      sai(_([["The universe has also undergone drastic changes to fit the game lore and make it overall a more interesting place. The old Trader's Guild has been revamped as the Space Traders Society and has control of several new systems. All these changes include lots of new events and things to discover which I will not spoil for you."]]))
      sai(_([["The number of slots ships have, and structural slots have been significantly tweaked. Additionally, House Sirius has been reworked completely. They have new weapons and outfits that you will be able to discover. Due to this, you may notice that some of your owned outfits and ship loadouts may have changed."]]))
      sai(fmt.f(_([["Naev now supports plugins! Although there aren't many available yet, we hope that this will increase in the future. If you are interested in creating plugins, please check out the website {website} for more information."]]),
         {website="#bhttps://naev.org#0"}))
      sai(fmt.f(_([["Some settings have been moved to be per-player instead of global. This includes all the autonav settings, which are now accessible from the #oInfo menu#0, which you can open with {infokey}. Autonav has also been rewritten and by default it should be much more useful and is able to use patrol lanes among other things."]]),
         {infokey=tut.getKey("info")}) )
      sai(fmt.f(_([[The 'board' and 'land' keys have been merged into a single one called 'approach'. The 'approach' key defaults to {keybind}, and will first try to board your current target (if applicable), before trying to land on your space target object. Please rebind the key as necessary.]]),
         {keybind=tut.getKey("approach")}))
   end

   -- 0.12.0 changes
   --[[
      TODO new features explanation

      1. Ship Capturing (chapter 1+)
      1. Reputation changes
      1. Holo-Archives
      1. Fuel increase
      1. Stats are additive
      1. Scanning (scan key)
   --]]
   if player.chapter()~="0" then
      sai(_([["You will now be able to capture ships that you disable for a price. However, you will need enough free fleet capacity to add the ship to your fleet when you capture it. Good thing that the fleet capacity limits have been significantly increase!"]]))
   end
   sai(fmt.f(_([["The Holo-Archives has also been added as a repository for information on mechanics, ships, outfits, and lore! Not only does it explain in detail many mechanics that you may have missed, but it also helps you find all the ships and outfits you've met in your travels. If that was not enough, there are also in-depth sections of lore explaining things about the universe that grow as you unlock them. You can access the Holo-Archives from the information menu you open with {infokey}."]]),
      {infokey=tut.getKey("info")}))
   sai(_([["Reputation has been significantly reworked. It is no longer a single absolute value for every faction, but computed on a local level. This means that your actions will mainly have local consequences, and are more forgiving. However, this only affects local actions such as attacking and boarding, reputation gained through missions is still global. If that was not enough, all reputation changes are shown by default, which you can disable from the information window."]]))
   sai(fmt.f(_([["Up until now, other ships could scan you, but you couldn't scan them. This has been changed in the latest release. You can now scan ships with the {scankey}. However, you will only be able to scan a ship after you have them as your active target for a while. You will see a spinning icon in the GUI. When it stops spinning, you will be able to scan!"]]),
      {scankey=tut.getKey("scan")}))
   sai(_([["This update also provides a significant modernization of the engine and many other features. To list a few, fuel has been increased for most ships, statistics are now additive instead of multiplicative, ship variants, improved point defense, reworked ship trails, much faster loading, tons of new content... I hope you enjoy the update!"]]))
   sai(_([["Weapon sets have also been simplified significantly. Unless you activate 'advanced' mode, there are no weapon set modes. Instead, now you have 2 additional weapon sets for your primary and secondary weapons. Furthermore, for the weapon set hotkeys, if you hold the key, it activates the outfits while held. However, if you tap it, it'll toggle the outfits from on to off, or off to on, depending on the current state. Additionally, by default, it will automatically try to assign all your active outfits to weapon sets, making the automatic setting much easier to use!"]]))
   sai(_([["Would you like me to reset your weapon sets to be automatically handled?"]]))
   vn.menu{
      {_("Keep current weapon sets"), "ws_cont"},
      {_("Reset weapon sets."), "ws_reset"},
   }

   vn.label("ws_reset")
   vn.func( function ()
      local curship = player.pilot():name()
      local ships = player.ships()
      local deployed = {}
      for k,s in ipairs( ships ) do
         if s.deployed then
            table.insert( deployed, s )
         end
      end
      for k,s in ipairs( ships ) do
         player.shipSwap( s.name, true )
         player.pilot():weapsetAuto()
      end
      player.shipSwap( curship, true )
      player.pilot():weapsetAuto()
      for k,s in ipairs( deployed ) do
         player.shipDeploy( s.name, true )
      end
   end )
   sai(_([["I've reset all the weapon sets on all your ships. Hopefully they should be easier to use now!"]]))
   vn.jump("ws_done")

   vn.label("ws_cont")
   sai(_([["OK, your weapon sets won't be updated, but if you need to make changes, please modify them from the #oInformation#0 window!"]]))
   vn.jump("ws_done")

   vn.label("ws_done")
   if not metai then
      sai(_([["With that said, would you like me to provide small, in-game advice as you do things throughout the game? Some might refer to things you are already familiar with, but it could help you learn new things."]]))
      vn.menu{
         {_("Enable tutorial hints"), "enable"},
         {_("Disable tutorial hints"), "disable"},
      }

      vn.label("enable")
      sai(fmt.f(_([["Great! I'll be giving you short hints as you do things through the game. If you want to change my settings or turn off the hints, please do so from the '#oShip AI#0' button in the #oInformation#0 window you can open with {infokey}. Now, let's go adventuring!"]]),{infokey=tut.getKey("info")}))
      vn.done( tut.shipai.transition )

      vn.label("disable")
      vn.func( function ()
         var.push( "tut_disable", true )
      end )
      sai(fmt.f(_([["OK, I will not be giving you any hints. If you want to change my settings, turn on the hints, or get information and advice, please do so from the '#oShip AI#0' button in the #oInformation#0 window you can open with {infokey}. Now, let's go adventuring!"]]),{infokey=tut.getKey("info")}))
   else
      sai(fmt.f(_([["And that is all! If you want to brush on game mechanics or get more hints, remember you can get in touch with me directly by clicking the '#oShip AI#0' button in the #oInformation#0 window that you can open with {infokey}. Now, let's go adventuring!"]]),
         {infokey=tut.getKey("info")}))
   end

   vn.done( tut.shipai.transition )
   vn.run()
end

-- Runs on saves older than 0.11.0
local function updater0110( _did0100, _did090 )
end

-- Runs on saves older than 0.10.0
local function updater0100( _did090 )
   -- "nelly_met" variable wasn't used in older versions
   if player.misnDone("Helping Nelly Out 1") then
      var.push( "nelly_met", true )
   end

   -- Update mission-variable based items to inventory-based
   local function update_inv( mvar, item )
      local amount = var.peek( mvar )
      if amount then
         player.inventoryAdd( item, amount )
         var.pop( mvar )
      end
   end
   update_inv( "minerva_tokens", N_("Minerva Token") )
   update_inv( "totoran_emblems", N_("Totoran Emblem") )
end

-- Runs on saves older than 0.9.0
local function updater090 ()
   -- Changed how the FLF base diff stuff works
   if diff.isApplied("flf_dead") and diff.isApplied("FLF_base") then
      diff.remove("FLF_base")
   end

   -- Set up pirate faction
   local fpir = faction.get("Pirate")
   local pirmod = fpir:reputationGlobal()
   for k,v in ipairs(pir.factions_clans) do
      local vs = v:reputationGlobal() -- Only get first parameter
      local vsd = v:reputationDefault()
      -- We'll be kind and set the player's pirate standing for the clans
      -- to be positive if the player was doing well with pirates before
      if pirmod > 0 and vs==vsd and not v:isKnown() then
         v:setReputationGlobal( fpir+20 )
      end
   end
   pir.updateStandings() -- Update pirate/marauder

   -- Some previously known factions become unknown
   --faction.get("Traders Guild"):setKnown(false) -- Gone in 0.11.0, replaced with Traders Society
   if not var.peek("disc_collective") then
      faction.get("Collective"):setKnown(false)
   end
   if not var.peek("disc_proteron") then
      local pro = faction.get("Proteron")
      pro:setKnown(false)
      pro:setReputationGlobal( pro:reputationDefault() ) -- Hostile by default
   end
   local fflf = faction.get("FLF")
   fflf:setKnown(false)
   if var.peek("disc_frontier") or player.misnDone("Deal with the FLF agent") or player.misnDone("Take the Dvaered crew home")  then
      fflf:setKnown(true)
   end
end

function create ()
   local _game_version, save_version = naev.version()
   local didupdate = false

   local did090, did0100, did0110
   -- Run on saves older than 0.9.0
   if not save_version or naev.versionTest( save_version, "0.9.0" ) < 0 then
      updater090()
      didupdate = true
      did090 = true
   end
   -- Run on saves older than 0.10.0
   if not save_version or naev.versionTest( save_version, "0.10.0" ) < 0 then
      updater0100( did090 )
      didupdate = true
      did0100 = true
   end
   -- Run on saves older than 0.11.0
   if not save_version or naev.versionTest( save_version, "0.11.0") < 0 then
      updater0110( did0100, did090 )
      didupdate = true
      did0110 = true
   end
   -- Run on saves older than 0.12.0
   if not save_version or naev.versionTest( save_version, "0.12.0-beta.1") < 0 then
      updater0120( did0110, did0100, did090 )
      didupdate = true
   end

   -- Note that games before 0.10.0 will have lastplayed set days from the unix epoch
   local _local, lastplayed = naev.lastplayed()
   if not didupdate and lastplayed > 30 and lastplayed < 365*30 then
      vn.clear()
      vn.scene()
      local sai = vn.newCharacter( tut.vn_shipai() )
      vn.transition( tut.shipai.transition )
      vn.na(fmt.f(_([[Your ship AI, {ainame}, materializes in front of you.]]),
         {ainame=tut.ainame()}))
      sai(fmt.f(_([["Welcome back, {player}! My ship logs indicate that you have not played Naev in quite a long time!"]]),
         {player=player.name()}))
      sai(_([["With that said, would you like me to reset the in-game advice as you do things throughout the game? Some might refer to things you are already familiar with, but it could help you learn new things."]]))
      vn.menu{
         {_("Reset and enable tutorial hints"), "enable"},
         {fmt.f(_("Leave as is (hints are {state})"),{state=((tut.isDisabled() and ("#r".._("off").."#0")) or ("#g".._("on").."#0"))}), "asis"},
      }

      vn.label("enable")
      vn.func( function () tut.reset() end )
      sai(fmt.f(_([["Great! I'll be giving you short hints as you do things through the game. If you want to change my settings or turn off the hints, please do so from the '#oShip AI#0' button in the #oInformation#0 window you can open with {infokey}. Now, let's go adventuring!"]]),
         {infokey=tut.getKey("info")}))
      vn.done( tut.shipai.transition )

      vn.label("asis")
      vn.func( function ()
         var.push( "tut_disable", true )
      end )
      sai(fmt.f(_([["OK, I will not be resetting the hints, and leave them as is. If you want to change my settings, turn on the hints, or get information and advice, please do so from the '#oShip AI#0' button in the #oInformation#0 window you can open with {infokey}. Now, let's go adventuring!"]]),{infokey=tut.getKey("info")}))

      vn.done( tut.shipai.transition )
      vn.run()
   end

   -- Done
   evt.finish()
end
