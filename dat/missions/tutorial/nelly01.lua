--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Helping Nelly Out 1">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>1</priority>
  <chance>100</chance>
  <cond>require("common.pirate").systemPresence() &lt;= 0</cond>
  <location>Bar</location>
  <faction>Dvaered</faction>
  <faction>Empire</faction>
  <faction>Frontier</faction>
  <faction>Goddard</faction>
  <faction>Independent</faction>
  <faction>Sirius</faction>
  <faction>Soromid</faction>
  <faction>Za'lek</faction>
 </avail>
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
local tutnel= require "common.tut_nelly.lua"
local tut   = require "common.tutorial"
local neu   = require "common.neutral"
local pir   = require "common.pirate"
local portrait = require 'portrait'
local vn    = require 'vn'
local fmt   = require "format"
local lmisn = require "lmisn"

--[[
   Mission States:
   0: accepted and heading to destpnt
   1: Told about info window
   2: Cargo delivered and head back to retpnt
   3: Ship boarded
--]]
misn_state = nil

cargo_type  = "Food"
cargo_q     = 5
reward_amount = 40e3
outfit_tobuy = outfit.get("Ion Cannon")

function create ()
   -- Save current system to return to
   retpnt, retsys = planet.cur()
   if not misn.claim( retsys ) then
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
   destpnt, destsys = lmisn.getRandomPlanetAtDistance( system.cur(), 1, 1, "Independent", false, pntfilter )
   if not destpnt then
      warn("No destpnt found")
      misn.finish()
   end

   misn.setNPC( tutnel.nelly.name, tutnel.nelly.portrait, _("You see a young individual sitting alone at a table.") )

   misn.setTitle( _("Helping Nelly Out") )
   misn.setDesc( fmt.f(_("Help Nelly deliver {tonnes} of {cargo} to {destpnt} in the {destsys} system."), {tonnes=fmt.tonnes(cargo_q), cargo=cargo_type, destpnt=destpnt:name(), destsys=destsys:name()} ))
   misn.setReward( fmt.credits(reward_amount) )
end


function accept ()
   local doaccept
   vn.clear()
   vn.scene()
   local nel = tutnel.vn_nelly()

   nel(fmt.f(_([[The lone individual lightens up when you near.
"Say, you look like a pilot with a working ship. I'm in a bit of a mess. You see, I was supposed to deliver some {cargoname} to {pntname} in the {sysname} system, but my ship broke down and I don't think I'll be able to deliver it any time soon. Would you be willing to help me take the cargo there and come back? I'll pay you your fair share."]]),
      {cargoname=cargo_type, pntname=destpnt:name(), sysname=destsys:name()}))

   vn.menu{
      {_("Help them out"), "accept"},
      {_("Decline to help"), "decline"},
   }

   vn.label("decline")
   nel(_([[They look dejected.
"That's a shame. If you change your mind I'll be waiting here."]]))
   vn.done()

   vn.label("nofreespace")
   nel(fmt.f(_([["You don't have enough free space to help me out. You'll need to carry {amount} of {cargo}. Please make free space by either selling unwanted cargo, or getting rid of it from the info menu which you can open with {infokey}."]]),
         {amount=fmt.tonnes(cargo_q), cargo=cargo_type, infokey=tut.getKey("info")}))
   vn.done()

   vn.label("accept")
   vn.func( function ()
      if player.pilot():cargoFree() < cargo_q then
         vn.jump("nofreespace")
      end
   end )
   nel(_([["Great! My name is Nelly. Glad to make your acquaintance. I'll have the dock workers load up your ship and we can be off. This should be a piece of cake."
They cock their head a bit at you.
"Say, you wouldn't happen to be a novice pilot?"]]))
   vn.menu{
      {_([["Yes"]]), "novice_yes"},
      {_([["No"]]), "novice_no"},
   }

   vn.label("novice_yes")
   nel(_([["I knew it! You seem to have a nice fresh aura around you. Reminds me of back in the day when I was starting out. Starting out can be a bit tricky, so I hope you don't mind if I give you some advice on the road."]]))

   vn.label("novice_no")
   nel(_([["Weird. I could have sworn you had some sort of new pilot aura around you. Must have been my imagination. Let's get going!"]]))

   vn.run()

   -- Check to see if truly accepted
   if not doaccept then return end

   misn.accept()

   misn_state = 0

   cargo_id = misn.cargoAdd( cargo_type, cargo_q )

   misn.osdCreate( _("Helping Nelly Out"), {
      fmt.f(_("Deliver cargo to {pntname} in {sysname}"), {sysname=destsys:name(), pntname=destpnt:name()} ),
      fmt.f(_("Return to {pntname} in {sysname}"), {syname=retsys:name(), pntname=retpnt:name()} ),
   } )

   hook.enter("enter")
   hook.land("land")
end

function land ()
   if hk_info_timer then
      hook.rm( hk_info_timer )
      hk_info_timer = nil
   end

   local pcur = planet.cur()
   if misn_state==1 and pcur == destsys then
      -- Delivered Cargo
      vn.clear()
      vn.scene()
      local nel = tutnel.vn_nelly()
      nel(_([[You land on ]]))
      -- TODO try get the player to buy the outfit
      vn.run()

      -- Get rid of cargo
      misn.cargoRm( cargo_id )
      misn_state = 2
      misn.osdActive(2)

      -- Hook the outfits
      hk_land_outfits = hook.land( "outfits", "outfits" )

   elseif misn_state==3 and pcur == retsys then
      -- Finished mission
      vn.clear()
      vn.scene()
      local nel = tutnel.vn_nelly()
      vn.run()

   end
end

function enter ()
   if hk_info_timer then
      hook.rm( hk_info_timer )
      hk_info_timer = nil
   end
   if hk_land_outfits then
      hook.rm( hk_land_outfits )
      hk_land_outfits = nil
   end
   if hk_outfit_buy then
      hook.rm( hk_outfit_buy )
      hk_outfit_buy = nil
   end
   if hk_land_equipment then
      hook.rm( hk_land_equipment )
      hk_land_equipment = nil
   end
   if hk_equip then
      hook.rm( hk_equip )
      hk_equip = nil
   end

   if misn_state==0 then
      vn.clear()
      vn.scene()
      local nel = tutnel.vn_nelly()
      vn.na(fmt.f(_("After the dock workers load the cargo on your ship, you take off with Nelly aboard. On to {sysname}!"), {sysname:name()}))
      nel(_([[Just after taking off Nelly pipes up.
"Say, are you familiar with the information window? It shows all the important things about your ship and current missions."]]))
      vn.menu{
         {_("Hear about the Info window"), "info_yes"},
         {_("Already know"), "info_no"},
      }

      vn.label("info_no")
      nel(_([["OK, great! Then let's get going!"]]))
      vn.done()

      vn.label("info_yes")
      nel(fmt.f(_([["The information window, which you can open with {infokey}, is critical to managing your ship and finding out where to go. Try opening the info menu with {infokey} and I will show you around it."]]),{infokey=tut.getKey("info")}))
      vn.func( function ()
         hk_info = hook.info( "info" )
         hk_info_timer = hook.timer( 15, "info_reminder" )
      end )
      vn.run()

      -- Advance so it deosn't do the same convo
      misn_state = 1

   elseif misn_state==2 and system.cur() == retsys then
      hook.timer( 5e3, "talk_derelict" )

   end
end

function talk_derelict ()
   -- TODO Show and talk about disabled ship
   vn.clear()
   vn.scene()
   local nel = tutnel.vn_nelly()
   vn.run()
end

function info_reminder ()
   player.pilot():comm(fmt.f(_("Try opening the info menu with {infokey}."),{infokey=tut.getKey("info")}))
   times_said = (times_said or 0) + 1
   if times_said < 4 then
      hk_info_timer = hook.timer( 15, "info_reminder" )
   else
      hk_info_timer = nil
   end
end

function info_msg( msg )
   vntk.msg( tutnel.nelly.name, msg )
end

function info ()
   if hk_info_timer then
      hook.rm( hk_info_timer )
      hk_info_timer = nil
   end
   if hk_info then
      hook.rm( hk_info )
      hk_info = nil
   end

   info_msg( _([["Ah, the info menu in all it's glory. In the main window, you can see overall statistics of your gameplay and license information. Try to navigate to the #oMissions#0 tab. Feel free to click the other tabs for more information."]]) )

   hk_info_ship      = hook.info( "info_ship",     "ship" )
   hk_info_weapons   = hook.info( "info_weapons",  "weapons" )
   hk_info_cargo     = hook.info( "info_cargo",    "cargo" )
   hk_info_mission   = hook.info( "info_mission",  "mission" )
   hk_info_standing  = hook.info( "info_standing", "standing" )
   hk_info_shiplog   = hook.info( "shiplog",       "shiplog" )
end

function info_checkdone ()
   if hk_info_ship or
         hk_info_weapons or
         hk_info_cargo or
         hk_info_mission or
         hk_info_standing or
         hk_info_ship then
      return
   end
   -- TODO not sure we need more message annoyances here
end

function info_ship ()
   info_msg( _([["The #oShip Info#0 tab contains information relevant to your ship including all active bonuses and equipped outfits."]]) )
   hook.rm( hk_info_ship )
   hk_info_ship = nil
   info_checkdone()
end

function info_weapons ()
   info_msg( fmt.f(_([["At the #oWeapon Info#0, you can modify the current ship's weapon sets. There are three types of weapon sets:
- #oWeapons - Switched#0: when activated define the weapons you should with the primary weapon key {keyprimary} and secondary weapon key {keysecondary}.
- #oWeapons - Instant#0: immediately fires the weapon set weapons when the set hotkey is pressed. Enable this for a set by checking the #oEnable instant mode#0 checkbox.
- #oAbilities - Toggled#0: toggles the state of the non-weapon outfits in the set.
"]]), {keyprimary=tut.getKey("primary"), keysecondary=tut.getKey("secondary")} ) )
   hook.rm( hk_info_weapons )
   hk_info_weapons = nil
   info_checkdone()
end

function info_cargo ()
   info_msg( _([["Here, at the #oCargo Info#0, you can see the cargo you are carrying. This includes important information like legal status. Furthermore, you can #ojettison#0 cargo you want to get rid of. Jettisoning mission cargo will forfeit the mission."]]) )
   hook.rm( hk_info_cargo )
   hk_info_cargo = nil
   info_checkdone()
end

function info_mission ()
   info_msg( fmt.f(_([["The #oMission Info#0 shows you information of all currently accepted missions. Selecting each mission will center the map on mission markers if available. Clicking on systems here also allows you to set routes directly. Make sure this mission is selected and try setting a route to {destsys} is marked on your map."]]), {destsys=destsys:name()} ) )
   hook.rm( hk_info_mission )
   hk_info_mission = nil

   hk_target_hyperspace = hook.custom( "target_hyperspace", "target_hyperspace" )
   hk.timer( 0, clear_target_hyperspace ) -- Clear it right away if the player closes the window
   info_checkdone()
end

function target_hyperspace ()
   local tsys, jumps = player.autonavDest()
   if tsys ~= destsys then
      return
   end

   info_msg( _([["Good! Now we are all set to head to our target system. You can click on other tabs in the Info window to get more explanations. When you are done, close the info window and let us head to our goal!"]]) )

   hook.rm( hk_target_hyperspace )
   hk_target_hyperspace = nil
end

function clear_target_hyperspace ()
   hook.rm( hk_target_hyperspace )
   hk_target_hyperspace = nil
   info_checkdone()
end

function info_standing ()
   info_msg( _([["The #oStanding Info#0 informs you at your current standing with the different known factions in the universe. Not everyone is as friendly as me out there!"]]) )
   hook.rm( hk_info_standing )
   hk_info_standing = nil
   info_checkdone()
end

function info_shiplog ()
   info_msg( _([["The #oShiplog#0 is a good way to review missions and tasks you have completed. I sometimes forget exactly what I did and who I should check up on and it comes really in handy. You can also keep a diary and add custom entries to keep track of what you have done or are doing."]]) )
   hook.rm( hk_info_shiplog )
   hk_info_shiplog = nil
   info_checkdone()
end

function outfits ()
   info_msg( fmt.f(_([["Here you can see all the outfits available for sale at this planet. Look for the #o{outfit}#0 and either #oright-click on it#0 or click on it and click the buy button to buy it."]]), {outfit=outfit_tobuy:name()} ) )

   hk_outfit_buy = hook.outfit_buy( "outfit_buy" )

   hook.rm( hk_land_outfits )
   hk_land_outfits = nil
end

function outfit_buy( name, q )
   local o = outfit.get( name )
   local t = o:type()
   if t=="Map" or t=="Local Map" or t=="License" or t=="GUI" or t=="Unknown" then
      return
   end
   if o ~= outfit_tobuy then
      info_msg( fmt.f(_([["You bought a #o{bought}#0 instead of #o{tobuy}#0! Don't worry, you can sell most outfits back for the price you bought them at. Please try to buy a #o{tobuy}#0. It should come in handy later."]]), {bought=o:name(), tobuy=outfit_tobuy:name()}))
      return
   end

   info_msg( fmt.f(_([["Great! You bought the #o{bought}#0. Outfits don't get equipped right away. To be able to use it, please navigate to the #oEquipment Tab#0 of the landing window."]]), {bought=o:name()}))

   hook.rm( hk_outfit_buy )
   hk_outfit_buy = nil

   hk_land_equipment = hook.land( "equipment", "equipment" )
end

function equipment ()
   info_msg( fmt.f(_([["The #oEquipment Tab#0 allows to handle your ships and their equipment. You can have more than one ship and switch between them freely. Try to equip the #o{outfit}#0 in a weapon slot by first clicking on the outfit and then right clicking on the slot you want to equip it at. If you have free slots you can also right click on an outfit to have it directly be assigned to the smallest free slot it fits into. Try to equip the #o{outfit}#0 now."]]), {outfit=outfit_tobuy:name()}) )

   hk_equip = hook.equip( "equip" )
end

function equip ()
   local pp = player.pilot()
   local hasoutfit = false
   for k,o in ipairs(pp:outfits()) do
      if o == outfit_tobuy then
         hasoutfit = true
         break
      end
   end

   if not hasoutfit then
      return
   end

   info_msg( fmt.f(_([["Great! Now you have the #o{outfit}#0 equipped. If your ship is set to automatically weapons, it should be assigned to a primary weapon. If not, you will have to assign the #o{outfit}#0 to a weapon set so you can use that. You can check by opening the #oInfo Window#0 with {infokey}. Check to make sure that is set up and let us go back to {pntname} in {sysname}."]]), {outfit=outfit_tobuy:name(), infokey=tut.getKey("info"), pntname=destpnt:name(), sysname=destsys:name()} ))

   hook.rm( hk_equip )
   hk_equip = nil
end
