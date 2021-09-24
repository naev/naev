--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Helping Nelly Out 1">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>1</priority>
  <location>None</location>
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
outfit_buy = outfit.get("Ion Cannon")

function create ()
   -- Save current system to return to
   retpnt, retsys = planet.cur()
   if not misn.claim( retsys ) then
      misn.finish()
   end

   -- Find destination system that sells ion cannons
   local pntfilter = function( p )
      if p:services().outfits == nil then
         return false
      end
      for k,o in ipairs(p:outfitsSold()) do
         if o == outfit_buy then
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
   local pcur = planet.cur()
   if misn_state==1 and pcur == destsys then
      -- Delivered Cargo
      vn.clear()
      vn.scene()
      local nel = tutnel.vn_nelly()
      vn.run()

      -- Get rid of cargo
      misn.cargoRm( cargo_id )
      misn_state = 2
      misn.osdActive(2)

   elseif misn_state==3 and pcur == retsys then
      -- Finished mission
      vn.clear()
      vn.scene()
      local nel = tutnel.vn_nelly()
      vn.run()

   end
end

function enter ()
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
      nel(_([["The information window, which you can open with {infokey}, is critical to managing your ship and finding out where to go."]]))
      -- TODO hooks for info menu plus walkthrough
      vn.run()

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
