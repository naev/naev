--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Assault on Unicorn">
 <priority>3</priority>
 <cond>player.numOutfit("Mercenary License") &gt; 0 and faction.playerStanding("Dvaered") &gt; 5 and system.cur() == system.get("Amaroq") and not var.peek("assault_on_unicorn_check")</cond>
 <chance>36</chance>
 <location>Computer</location>
 <done>Empire Shipping 3</done>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
 --]]
--[[

   MISSION: Assault on Unicorn
   DESCRIPTION: Kill some pirates!

--]]
local pir = require 'common.pirate'
local fmt = require "format"
local dv  = require "common.dvaered"

-- Mission constants
local misn_target_sys = system.get("Unicorn")
-- local misn_return_sys = system.get("Amaroq")

local function update_osd()
   local osd_msg = {}
   osd_msg[1] = _("Fly to the Unicorn system.")
   if mem.bounty_earned == mem.max_payment then
      osd_msg[2] = fmt.f(_("You have reached your maximum payment. Return to {pnt}."), {pnt=mem.planet_start})
   else
      osd_msg[2] = fmt.f(_("Destroy some pirates! You have killed {n} and have earned {credits}. If finished, return to {pnt}."),
                         {n=mem.pirates_killed, credits=fmt.credits(mem.bounty_earned), pnt=mem.planet_start})
   end
   misn.osdCreate(_("Assault on Unicorn"), osd_msg)
end

function create ()
   local rep = faction.playerStanding("Dvaered")
   -- Round the payment to the nearest thousand.
   mem.max_payment = rep * 50e3
   misn.setTitle(dv.prefix.._("Assault on Unicorn"))
   misn.setReward(_("Variable"))
   misn.setDesc(fmt.f(_("It is time to put a dent in the pirates' forces. We have detected a strong pirate presence in the system of Unicorn. We are offering a small sum for each pirate killed. The maximum we will pay you is {credits}."), {credits=fmt.credits(mem.max_payment)} ))

   mem.marker = misn.markerAdd( misn_target_sys, "computer" )
end

function accept ()
   -- This mission makes no system claims.
   if misn.accept() then
      mem.pirates_killed = 0
      -- Makes sure only one copy of the mission can run.
      var.push( "assault_on_unicorn_check", true)
      mem.planet_start = spob.cur()
      mem.marker2 = misn.markerAdd( mem.planet_start, "low" )
      mem.pirates_killed = 0
      mem.bounty_earned = 0
      mem.misn_stage = 0
      update_osd()
      misn.osdActive(1)

      hook.enter("jumpin")
      hook.land("land")
   end
end

function jumpin()
   if system.cur() == misn_target_sys then
      misn.osdActive(2)
      hook.pilot(nil, "death", "death")
   end
end

function death(pilot,killer)
   if pir.factionIsPirate( pilot:faction() )
         and (killer == player.pilot()
            or killer:leader() == player.pilot()) then

      local reward_earned = pilot:ship():price()/10
      mem.pirates_killed = mem.pirates_killed + 1
      mem.bounty_earned = math.min( mem.max_payment, mem.bounty_earned + reward_earned )
      update_osd()
      misn.osdActive(2)
   end
end

function land()
   if spob.cur() == mem.planet_start and mem.pirates_killed > 0 then
      var.pop( "assault_on_unicorn_check" )

      tk.msg(_("Mission accomplished"), fmt.f(_("As you land, you see a Dvaered military official approaching. Thanking you for your hard and diligent work, he hands you the bounty you've earned, a number of chips worth {credits}."), {credits=fmt.credits(mem.bounty_earned)}))
      player.pay(mem.bounty_earned)
      faction.modPlayerSingle( "Dvaered", math.pow( mem.bounty_earned, 0.5 ) / 100 )
      misn.finish(true)
   end
end

function abort ()
   var.pop( "assault_on_unicorn_check" )
end
