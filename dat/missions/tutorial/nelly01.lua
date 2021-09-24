--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Tutorial">
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

cargo_type = "Food"
cargo_q = 5
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

   nel(_([[]]))
   vn.menu{
      {_("Help Nelly out"), "accept"},
      {_("Decline to help"), "decline"},
   }

   vn.label("decline")
   nel(_([[]]))
   vn.done()
   
   vn.label("accept")
   nel(_([[]]))

   vn.run()

   -- Check to see if truly accepted
   if not doaccept then return end

   misn.accept()

   misn_state = 0

   misn.cargoAdd( "Food"

   hook.enter("enter")
   hook.land("land")
end

function land ()
   local pcur = planet.cur()
   if misn_state==1 and pcur == destsys then
      vn.clear()
      vn.scene()
      local nel = tutnel.vn_nelly()
      vn.run()

   elseif misn_state==3 and pcur == retsys then
      vn.clear()
      vn.scene()
      local nel = tutnel.vn_nelly()
      vn.run()

   end
end

function enter ()
   if misn_state==0 then
      -- TODO add explanation about info window
   end
end
