--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Helping Nelly Out 1">
 <unique />
 <priority>1</priority>
 <chance>100</chance>
 <cond>require("common.pirate").systemPresence() &lt;= 0 and not player.misnActive("Tutorial")</cond>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Za'lek</faction>
  <chapter>[01]</chapter>
 <notes>
  <campaign>Tutorial Nelly</campaign>
 </notes>
</mission>
--]]
--[[
   Nelly Tutorial Campaign

   Basically similar to what EV:Nova has with Barry, except for Naev.

   First mission is designed to teach about:
   1. Information Window
   2. Delivery Missions
   3. Outfitter
   4. Equipping outfits
   5. Boarding
   6. Mission computer

   Mission Details:
   0. Explanation about information window
   1. Go to nearby system to deliver cargo
   2. Told to buy and equip an Ion Cannon (don't enforce it, but complain if player doesn't get it)
   3. Fly back to original planet
   4. Find and board derelict on the way there
   5. Land on planet and mission over
--]]
local tutnel= require "common.tut_nelly"
local tut   = require "common.tutorial"
local pir   = require "common.pirate"
local vn    = require 'vn'
local vntk  = require 'vntk'
local fmt   = require "format"
local lmisn = require "lmisn"

--[[
   Mission States:
   0: accepted and heading to mem.destpnt
   1: Told about info window
   2: Cargo delivered and head back to mem.retpnt
   3: Ship boarded
--]]
mem.misn_state = nil
local derelict -- Non-persistent state

-- Constants
local cargo_type  = commodity.get("Food")
local cargo_q     = 5
local reward_amount = tutnel.reward.nelly01
local outfit_tobuy = outfit.get("Ion Cannon") -- If changed, references to ion cannons/damage below should too.

function create ()
   -- Save current system to return to
   mem.retpnt, mem.retsys = spob.cur()
   if not misn.claim( {mem.retsys, "nelly"} ) then
      misn.finish()
   end
   -- Need commodity exchange and mission computer
   local rs = mem.retpnt:services()
   if rs.commodity == nil or rs.missions == nil then
      misn.finish()
   end

   -- Find destination system that sells ion cannons
   local pntfilter = function( p )
      -- No pirates
      if pir.systemPresence( p:system() ) > 0 then
         return false
      end
      -- Sells Outfits
      if p:services().outfits == nil then
         return false
      end
      -- Sells a particular outfit
      for k,o in ipairs(p:outfitsSold()) do
         if o == outfit_tobuy then
            return true
         end
      end
      return false
   end
   mem.destpnt, mem.destsys = lmisn.getRandomSpobAtDistance( system.cur(), 1, 1, "Independent", false, pntfilter )
   if not mem.destpnt then
      misn.finish()
   end

   misn.setNPC( tutnel.nelly.name, tutnel.nelly.portrait, _("You see a young individual sitting alone at a table.") )

   misn.setTitle( _("Helping Nelly Out") )
   misn.setDesc( fmt.f(_("Help Nelly deliver {tonnes} of {cargo} to {destpnt} in the {destsys} system."), {tonnes=fmt.tonnes(cargo_q), cargo=cargo_type, destpnt=mem.destpnt, destsys=mem.destsys} ))
   misn.setReward(reward_amount)
end


function accept ()
   local doaccept
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )

   if var.peek("nelly_met") then
      nel(fmt.f(_([[Nelly lightens up when you near her.
"Hello again! I'm in a bit of a mess. You see, I was supposed to deliver some {cargo} to {pnt} in the {sys} system, but my ship broke down and I don't think I'll be able to deliver it any time soon. Would you be willing to help me take the cargo there and come back? I'll pay you your fair share."]]),
         {cargo=cargo_type, pnt=mem.destpnt, sys=mem.destsys}))
   else
      nel(fmt.f(_([[The lone individual lightens up when you near her.
"Say, you look like a pilot with a working ship. I'm in a bit of a mess. You see, I was supposed to deliver some {cargo} to {pnt} in the {sys} system, but my ship broke down and I don't think I'll be able to deliver it any time soon. Would you be willing to help me take the cargo there and come back? I'll pay you your fair share."]]),
         {cargo=cargo_type, pnt=mem.destpnt, sys=mem.destsys}))
   end

   vn.menu{
      {_("Help them out"), "accept"},
      {_("Decline to help"), "decline"},
   }

   vn.label("decline")
   nel(_([[They look dejected.
"That's a shame. If you change your mind I'll be waiting here."]]))
   vn.done( tutnel.nelly.transition )

   vn.label("nofreespace")
   nel(fmt.f(_([["You don't have enough free space to help me out. You'll need to carry {amount} of {cargo}. Please make free space by either selling unwanted cargo, or getting rid of it from the info menu which you can open with {infokey}."]]),
         {amount=fmt.tonnes(cargo_q), cargo=cargo_type, infokey=tut.getKey("info")}))
   vn.done( tutnel.nelly.transition )

   vn.label("accept")
   vn.func( function ()
      if player.pilot():cargoFree() < cargo_q then
         vn.jump("nofreespace")
         return
      end
      doaccept = true
   end )
   if var.peek("nelly_met") then
      nel(_([["Great! I'll have the dock workers load up your ship and we can be off. This should be a piece of cake."]]))
   else
      nel(_([["Great! My name is Nelly. Glad to make your acquaintance. I'll have the dock workers load up your ship and we can be off. This should be a piece of cake."
   They cock their head a bit at you.
   "Say, you wouldn't happen to be a novice pilot?"]]))
      vn.func( function ()
         var.push( "nelly_met", true )
      end )
      vn.menu{
         {_([["Yes"]]), "novice_yes"},
         {_([["No"]]), "novice_no"},
      }

      vn.label("novice_yes")
      nel(fmt.f(_([["I knew it! You seem to have a nice fresh aura around you. Reminds me of back in the day when I was starting out. Starting out can be a bit tricky, so I hope you don't mind if I give you some advice on the road."
   "For starters, if you haven't already, you should buy a #oLocal Map#0 that will help you get the directions to {sys}. You can buy it at the main landing window or the outfiting window. Anyway, Let's get going!"]]), {sys=mem.destsys}))
      vn.done( tutnel.nelly.transition )

      vn.label("novice_no")
      nel(_([["Weird. I could have sworn you had some sort of new pilot aura around you. Must have been my imagination. Let's get going!"]]))
   end
   vn.done( tutnel.nelly.transition )
   vn.run()

   -- Check to see if truly accepted
   if not doaccept then return end

   misn.accept()

   mem.misn_state = 0

   mem.cargo_id = misn.cargoAdd( cargo_type, cargo_q )

   mem.misn_marker = misn.markerAdd( mem.destpnt )

   misn.osdCreate( _("Helping Nelly Out"), {
      fmt.f(_("Deliver cargo to {pnt} in {sys}"), {sys=mem.destsys, pnt=mem.destpnt} ),
      fmt.f(_("Return to {pnt} in {sys}"), {sys=mem.retsys, pnt=mem.retpnt} ),
   } )

   hook.enter("enter")
   hook.land("land")
end

function land ()
   if mem.hk_info_timer then
      hook.rm( mem.hk_info_timer )
      mem.hk_info_timer = nil
   end

   local pcur = spob.cur()
   if mem.misn_state==1 and pcur == mem.destpnt then
      local wanthelp = true

      -- Delivered Cargo
      vn.clear()
      vn.scene()
      local nel = vn.newCharacter( tutnel.vn_nelly() )
      vn.transition( tutnel.nelly.transition )
      vn.na(fmt.f(_([[You land on {pnt} and the dockworkers offload your cargo. This delivery stuff is quite easy.]]),{pnt=mem.destpnt}))
      nel(fmt.f(_([["Say, I heard this place sells #oIon Cannons#0. If you want to be able to take down ships non-lethally, #oion damage#0 is your best bet. Here, I'll forward you {credits}. Do you need help buying and equipping the outfit?"]]),{credits=fmt.credits(outfit_tobuy:price())}))
      vn.func( function ()
         player.pay( outfit_tobuy:price() )
      end )
      vn.menu{
         {_("Get useful advice"), "help_yes"},
         {_("Buy the outfit alone"), "help_no"},
      }

      vn.label("help_no")
      nel(fmt.f(_([["OK! Taking the initiative I see. Go buy the {outfit} at the #oOutfits Tab#0 and make sure to equip it at the #oEquipment Tab#0 before taking off. Once you get that done, let's head back to {pnt} in {sys}."]]),{outfit=outfit_tobuy, pnt=mem.retpnt, sys=mem.retsys}))
      vn.func( function ()
         wanthelp = false
      end )
      vn.done( tutnel.nelly.transition )

      vn.label("help_yes")
      nel(_([["OK! I'll guide you through it. For now, please go to the #oOutfits Tab#0 of the landing window."]]))

      vn.done( tutnel.nelly.transition )
      vn.run()

      player.pay( outfit_tobuy:price() )

      -- Get rid of cargo
      misn.cargoRm( mem.cargo_id )
      mem.misn_state = 2
      misn.osdActive(2)

      misn.markerMove( mem.misn_marker, mem.retpnt )

      -- Hook the outfits
      if wanthelp then
         mem.hk_land_outfits = hook.land( "outfits", "outfits" )
      end

   elseif mem.misn_state==3 and pcur == mem.retpnt then
      -- Finished mission
      vn.clear()
      vn.scene()
      local nel = vn.newCharacter( tutnel.vn_nelly() )
      vn.transition( tutnel.nelly.transition )
      nel(_([["We made it! A job well done. Here let me pay you what I promised you.]]))
      vn.sfxVictory()
      vn.na(fmt.reward(reward_amount))
      vn.func( function () -- Rewards
         player.pay( reward_amount )
      end )
      nel(_([["Now time to get back to my ship. I hope it's repaired already. If you want to do more cargo missions to make some easy credits, I recommend you to look at the mission computer, which should be available on most planets like here. Sometimes you will also meet interesting characters at the spaceport bar who will offer you interesting missions."
They beam you a grin.
"Anyway, see you around! Make sure to check the spaceport bar whenever you land for new missions!"]]))
      if mem.gotore then
         nel(_([["Oh, and don't forget to sell the ore you got from the derelict at the commodity exchange!"]]))
      end
      vn.done( tutnel.nelly.transition )
      vn.run()

      tutnel.log(_("You helped Nelly complete a delivery mission."))

      misn.finish(true)
   end
end

function enter ()
   if mem.hk_info_timer then
      hook.rm( mem.hk_info_timer )
      mem.hk_info_timer = nil
   end
   if mem.hk_land_outfits then
      hook.rm( mem.hk_land_outfits )
      mem.hk_land_outfits = nil
   end
   if mem.hk_outfit_buy then
      hook.rm( mem.hk_outfit_buy )
      mem.hk_outfit_buy = nil
   end
   if mem.hk_land_equipment then
      hook.rm( mem.hk_land_equipment )
      mem.hk_land_equipment = nil
   end
   if mem.hk_equip then
      hook.rm( mem.hk_equip )
      mem.hk_equip = nil
   end

   if mem.misn_state==0 then
      vn.clear()
      vn.scene()
      local nel = vn.newCharacter( tutnel.vn_nelly() )
      vn.transition( tutnel.nelly.transition )
      vn.na(fmt.f(_("After the dock workers load the cargo on your ship, you take off with Nelly aboard. On to {sys}!"), {sys=mem.destsys}))
      nel(_([[Just after taking off Nelly pipes up.
"Say, are you familiar with the information window? It shows all the important things about your ship and current missions."]]))
      vn.menu{
         {_("Hear about the Info window"), "info_yes"},
         {_("Already know"), "info_no"},
      }

      vn.label("info_no")
      nel(_([["OK, great! Then let's get going!"]]))
      vn.done( tutnel.nelly.transition )

      vn.label("info_yes")
      nel(fmt.f(_([["The information window, which you can open with {infokey}, is critical to managing your ship and finding out where to go. Try opening the info menu with {infokey} and I will show you around it."]]),{infokey=tut.getKey("info")}))
      vn.func( function ()
         mem.hk_info = hook.info( "info" )
         mem.hk_info_timer = hook.timer( 15, "info_reminder" )
      end )
      vn.done( tutnel.nelly.transition )
      vn.run()

      -- Advance so it deosn't do the same convo
      mem.misn_state = 1

   elseif mem.misn_state==2 and system.cur() == mem.retsys then
      hook.timer( 5e3, "talk_derelict" )

      local pos = player.pos() * 0.6 -- Should be to the center of the system
      derelict = pilot.add( "Koala", "Derelict", pos, p_("ship", "Derelict") )
      derelict:disable()
      derelict:intrinsicSet( "ew_hide", 300 ) -- Much more visible
      derelict:setHilight(true)
      hook.pilot( derelict, "board", "board" )

      mem.misn_state = 3
   end
end

function talk_derelict ()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   vn.transition( tutnel.nelly.transition )
   vn.na(_([[After you enter the system, Nelly points something out on the radar.]]))
   -- TODO autoboard!
   nel(fmt.f(_([["Oooh, look at that. A Koala derelict is nearby. There might be something interesting on it! We should go board it. Try #odouble-click#0ing or selecting the derelict and pressing {boardkey}. It should enable your autonav system to board the ship. You can toggle the overlay to see exactly where the ship is with {overlaykey}."]]),{boardkey=tut.getKey("approach"), overlaykey=tut.get("overlay")}))
   vn.done( tutnel.nelly.transition )
   vn.run()
end

function board ()
   -- TODO maybe explain better boarding window when done
   vn.clear()
   vn.scene()
   local nel = tutnel.vn_nelly()
   vn.transition( tutnel.nelly.transition )
   vn.na(_([[You enter the derelict ship which is eerily silent. A large hole in the engine room indicates that likely the core engine blew out, forcing the ship crew to abandon ship. Although you don't find anything of significant value, there is still lots of ore cargo available on the ship. You quickly load as much as you can onto your ship before you depart.]]))
   vn.appear( nel )
   nel(fmt.f(_([["Looks like your scored a lot of ore there. That should bring you a pretty penny at a planet with commodity exchange. Boarding derelicts is not always as easy as this and sometimes bad things can happen. We should head to {pnt} that should be nearby now."]]), {pnt=mem.retpnt}))
   vn.done( tutnel.nelly.transition )
   vn.run()

   local pp = player.pilot()
   pp:cargoAdd( "Ore", pp:cargoFree() )
   mem.gotore = true

   player.unboard()
end

function info_reminder ()
   player.msg(fmt.f(_([[Nelly: "Try opening the info menu with {infokey}."]]),{infokey=tut.getKey("info")}), true)
   mem.times_said = (mem.times_said or 0) + 1
   if mem.times_said < 4 then
      mem.hk_info_timer = hook.timer( 15, "info_reminder" )
   else
      mem.hk_info_timer = nil
   end
end

local function info_msg( msg )
   vntk.msg( tutnel.nelly.name, msg )
end

function info ()
   if mem.hk_info_timer then
      hook.rm( mem.hk_info_timer )
      mem.hk_info_timer = nil
   end
   if mem.hk_info then
      hook.rm( mem.hk_info )
      mem.hk_info = nil
   end

   info_msg( _([["Ah, the info menu in all it's glory. In the main window, you can see overall statistics of your gameplay and license information. Try to navigate to the #oMissions#0 tab. Feel free to click the other tabs for more information."]]) )

   mem.hk_info_ship      = hook.info( "info_ship",     "ship" )
   mem.hk_info_weapons   = hook.info( "info_weapons",  "weapons" )
   mem.hk_info_cargo     = hook.info( "info_cargo",    "cargo" )
   mem.hk_info_mission   = hook.info( "info_mission",  "mission" )
   mem.hk_info_standing  = hook.info( "info_standing", "standing" )
   mem.hk_info_shiplog   = hook.info( "info_shiplog",  "shiplog" )
end

local function info_checkdone ()
   if mem.hk_info_ship or
         mem.hk_info_weapons or
         mem.hk_info_cargo or
         mem.hk_info_mission or
         mem.hk_info_standing or
         mem.hk_info_ship then
      return
   end
   -- TODO not sure we need more message annoyances here
end

function info_ship ()
   info_msg( _([["The #oShip Info#0 tab contains information relevant to your ship including all active bonuses and equipped outfits."]]) )
   hook.rm( mem.hk_info_ship )
   mem.hk_info_ship = nil
   info_checkdone()
end

function info_weapons ()
   info_msg( fmt.f(_([["At the #oWeapon Info#0, you can modify the current ship's weapon sets. There are three types of weapon sets:
- #oSwitch#0: sets which weapons fire with the primary weapon key {keyprimary} and secondary weapon key {keysecondary}.
- #oToggle#0: toggles the state between on and off of the outfits in the sets.
- #oHold#0: turns on all the outfits in the set while held down."]]),
      {keyprimary=tut.getKey("primary"), keysecondary=tut.getKey("secondary")} ) )
   hook.rm( mem.hk_info_weapons )
   mem.hk_info_weapons = nil
   info_checkdone()
end

function info_cargo ()
   info_msg( _([["Here, at the #oCargo Info#0, you can see the cargo you are carrying. This includes important information like legal status. Furthermore, you can #ojettison#0 cargo you want to get rid of. Be careful though: jettisoning mission cargo will forfeit the mission."]]) )
   hook.rm( mem.hk_info_cargo )
   mem.hk_info_cargo = nil
   info_checkdone()
end

function info_mission ()
   info_msg( fmt.f(_([["The #oMission Info#0 shows you information of all currently accepted missions. Selecting each mission will centre the map on mission markers if available. Clicking on systems here also allows you to set routes directly. Make sure this mission is selected and try setting a route to {destsys} is marked on your map."]]), {destsys=mem.destsys} ) )
   hook.rm( mem.hk_info_mission )
   mem.hk_info_mission = nil

   mem.hk_target_hyperspace = hook.custom( "target_hyperspace", "target_hyperspace" )
   hook.timer( 0.001, "clear_target_hyperspace" ) -- Clear it right away if the player closes the window
   info_checkdone()
end

function target_hyperspace ()
   local tsys = player.autonavDest()
   if tsys ~= mem.destsys then
      return
   end

   info_msg( _([["Good! Now we are all set to head to our target system. You can click on other tabs in the Info window to get more explanations. When you are done, close the info window and let us head to our goal!"]]) )

   hook.rm( mem.hk_target_hyperspace )
   mem.hk_target_hyperspace = nil
end

function clear_target_hyperspace ()
   hook.rm( mem.hk_target_hyperspace )
   mem.hk_target_hyperspace = nil
   info_checkdone()
end

function info_standing ()
   info_msg( _([["The #oStanding Info#0 informs you at your current standing with the different known factions in the universe. Not everyone is as friendly as me out there!"]]) )
   hook.rm( mem.hk_info_standing )
   mem.hk_info_standing = nil
   info_checkdone()
end

function info_shiplog ()
   info_msg( _([["The #oShiplog#0 is a good way to review missions and tasks you have completed. I sometimes forget exactly what I did and who I should check up on and it comes really in handy. You can also keep a diary and add custom entries to keep track of what you have done or are doing."]]) )
   hook.rm( mem.hk_info_shiplog )
   mem.hk_info_shiplog = nil
   info_checkdone()
end

function outfits ()
   info_msg( fmt.f(_([["Here you can see all the outfits available for sale at this planet. Look for the #o{outfit}#0 and either #oright-click on it#0 or click on it and click the buy button to buy it."]]), {outfit=outfit_tobuy} ) )

   mem.hk_outfit_buy = hook.outfit_buy( "outfit_buy" )

   hook.rm( mem.hk_land_outfits )
   mem.hk_land_outfits = nil
end

function outfit_buy( o, _q )
   local t = o:type()
   if t=="Map" or t=="Local Map" or t=="License" or t=="GUI" or t=="Unknown" then
      return
   end
   if o ~= outfit_tobuy then
      info_msg( fmt.f(_([["You bought a #o{bought}#0 instead of #o{tobuy}#0! Don't worry, you can sell most outfits back for the price you bought them at. Please try to buy a #o{tobuy}#0. It should come in handy later."]]), {bought=o, tobuy=outfit_tobuy}))
      return
   end

   info_msg( fmt.f(_([["Great! You bought the #o{bought}#0. Outfits don't get equipped right away. To be able to use it, please navigate to the #oEquipment Tab#0 of the landing window."]]), {bought=o}))

   hook.rm( mem.hk_outfit_buy )
   mem.hk_outfit_buy = nil

   mem.hk_land_equipment = hook.land( "equipment", "equipment" )
end

function equipment ()
   info_msg( fmt.f(_([["The #oEquipment Tab#0 allows to handle your ships and their equipment. You can have more than one ship and switch between them freely. Try to equip the #o{outfit}#0 in a weapon slot by first clicking on the outfit and then right clicking on the slot you want to equip it at. If you have free slots you can also right click on an outfit to have it directly be assigned to the smallest free slot it fits into. Try to equip the #o{outfit}#0 now."]]), {outfit=outfit_tobuy}) )

   mem.hk_equip = hook.equip( "equip" )
end

function equip ()
   local pp = player.pilot()
   local hasoutfit = false
   for k,o in ipairs(pp:outfitsList()) do
      if o == outfit_tobuy then
         hasoutfit = true
         break
      end
   end

   if not hasoutfit then
      return
   end

   info_msg( fmt.f(_([["Great! Now you have the #o{outfit}#0 equipped. If your ship is set to automatically handle weapons, it should be assigned to a primary weapon. If not, you will have to assign the #o{outfit}#0 to a weapon set so you can use that. You can check by opening the #oInfo Window#0 with {infokey}. Check to make sure that is set up and let us go back to {pnt} in {sys}."]]), {outfit=outfit_tobuy, infokey=tut.getKey("info"), pnt=mem.retpnt, sys=mem.retsys} ))

   hook.rm( mem.hk_equip )
   mem.hk_equip = nil
end
