--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Anxious Merchant">
 <priority>3</priority>
 <chance>5</chance>
 <location>Bar</location>
 <cond>
   local f = spob.cur():faction()
   if f then
      local ft = f:tags()
      if ft.generic or ft.misn_cargo then
         return true
      end
   end
   return false
 </cond>
 <notes>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[

   Anxious Merchant
   Author: PhoenixRiver (from an idea on the wiki)

   A merchant with a slow ship suddenly realizes he can't make the delivery and
   implores the player to do it for him. Since he has to look good with his
   employers he'll pay the player a bonus if he does it.

   Note: Variant of the Drunkard and Rush Cargo missions combined

]]--
local car = require "common.cargo"
local fmt = require "format"
local vn = require "vn"
local vni = require "vnimage"

local npc_name = _("Anxious Merchant")
local npc_portrait, npc_image
function create()
   -- Note: this mission does not make any system claims.

   -- Calculate the route, distance, jumps and cargo to take
   local _risk, tier
   mem.dest_planet, mem.dest_sys, mem.num_jumps, mem.travel_dist, mem.cargo, _risk, tier = car.calculateRoute()
   if mem.dest_planet == nil or mem.dest_sys == system.cur() or mem.cargo == nil then
      misn.finish(false)
   end

   npc_image, npc_portrait = vni.trader()
   misn.setNPC(npc_name, npc_portrait, _("You see a merchant at the bar in a clear state of anxiety.")) -- creates the merchant at the bar

   local stu_distance = 0.2 * mem.travel_dist
   local stu_jumps = 10300 * mem.num_jumps
   local stu_takeoff = 10300
   mem.time_limit = time.get() + time.new(0, 0, stu_distance + stu_jumps + stu_takeoff)

   -- Allow extra time for refuelling stops.
   local jumpsperstop = 3 + math.min(tier, 3)
   if mem.num_jumps > jumpsperstop then
      mem.time_limit:add(time.new( 0, 0, math.floor((mem.num_jumps-1) / jumpsperstop) * stu_jumps ))
   end

   mem.payment = 20 * (stu_distance + (stu_jumps / 10))

   -- Range of 5-10 tons for tier 0, 21-58 for tier 4.
   mem.cargo_size = rnd.rnd( 5 + 4 * tier, 10 + 12 * tier )
end

function accept()
   local accepted = false

   vn.clear()
   vn.scene()
   local m = vn.newCharacter( npc_name, {image=npc_image} )
   m(_([[As you sit down, the merchant looks up at you with a panicked expression, "Ahh! What do you want? Can't you see I've enough on my plate as it is?" You tell the merchant to calm down and offer a drink. "Jeez, that's nice of you… Ha, maybe I can get a break today!"]]))
   m(_([[You grab a couple of drinks and hand one to the slightly more relaxed looking merchant as they start to talk. "So, I work for the Space Traders Society. I transport stuff for them and they pay me. Only problem is, I kinda strained my engines running from pirates on the way to the pick-up and now I'm realising that my engines just don't have the speed to get me back to beat the deadline. And to top it all off, I'm late on my bills as is; I can't afford new engines now! It's like I'm in the Sol nebula without a shield generator."]]))
   m(fmt.f(_([[You attempt to reassure the merchant by telling them that, surely, the company will cut them some slack. "Like hell they will! I've already been scolded by management for this exact same thing before! If I don't get this shipment of {tonnes} of {cargo} to {pnt}… I really need this job, you know? I don't know what to do…" The merchant pauses. "Unless… Say, you wouldn't be able to help me out here, would you? I'd just need you to take the cargo to {pnt} in the {sys} system. Could you? I'll give you the payment for the mission if you do it; it means a lot!"]]),
      {tonnes=fmt.tonnes(mem.cargo_size), cargo=_(mem.cargo), pnt=mem.dest_planet, sys=mem.dest_sys}))
   vn.menu{
      {_([[Accept]]), "accept"},
      {_([[Decline]]), "decline"},
   }

   vn.label("decline")
   vn.done()

   vn.label("nospace")
   vn.na(_([[You don't have enough cargo space to accept this mission.]]))
   vn.done()

   local player_best
   vn.label("tooslow")
   vn.na( function ()
      return fmt.f(_([[The goods have to arrive in {time_limit} but it will take {time} for your ship to reach {pnt}. Accept the mission anyway?]]),
         {time_limit=(mem.time_limit - time.get()), time=(player_best - time.get()), pnt=mem.dest_planet})
   end )
   vn.menu{
      {_([[Accept]]), "accept_force"},
      {_([[Decline]]), "decline"},
   }

   local pp = player.pilot()
   vn.label("accept")
   vn.func( function ()
      if pp:cargoFree() < mem.cargo_size then
         vn.jump("nospace")
         return
      end
      pp:cargoAdd( mem.cargo, mem.cargo_size )
      player_best = car.getTransit( mem.num_jumps, mem.travel_dist )
      pp:cargoRm( mem.cargo, mem.cargo_size )
      if mem.time_limit < player_best then
         vn.jump("tooslow")
         return
      end
   end )
   vn.label("accept_force")
   vn.func( function () accepted = true end )
   m(fmt.f(_([[The merchant sighs in relief. "Thank you so much for this. Just bring the cargo to the cargo guy at {pnt}. They should pay you {credits} when you get there. Don't be late, OK?"]]),
      {pnt=mem.dest_planet, credits=fmt.credits(mem.payment)}))

   vn.run()

   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle(_("Anxious Merchant"))
   misn.setReward(mem.payment)
   misn.setDesc(fmt.f(_("You decided to help a fraught merchant by delivering some goods to {pnt}."), {pnt=mem.dest_planet}))
   mem.marker = misn.markerAdd( mem.dest_planet, "low" ) -- destination
   mem.cargo_ID = misn.cargoAdd( mem.cargo, mem.cargo_size ) -- adds cargo

   mem.intime = true
   mem.land_hook = hook.land("land")
   mem.date_hook = hook.date(time.new(0, 0, 42), "tick") -- 42STU per tick

   -- OSD
   tick()
end

function land()
   if spob.cur() ~= mem.dest_planet then
      return
   end

   vn.clear()
   vn.scene()
   vn.transition()

   if mem.intime then
      vn.na(_([[As you touch down at the spaceport you see the Traders Society depot surrounded by a hustle and bustle. The cargo inspector looks at you with surprise and you explain to him what happened as the cargo is unloaded from your ship. "Wow, thanks for the help! You definitely saved us a ton of grief. Here's your payment. Maybe I can buy you a drink some time!" You laugh and part ways.]]))
      vn.sfxVictory()
      vn.func( function ()
         faction.modPlayerSingle("Traders Society", 1)
         player.pay(mem.payment)
      end )
      vn.na(fmt.reward(mem.payment))
   else
      vn.na(_([[Landing at the spaceport you see the Traders Society depot surrounded by a fraught hum of activity. The cargo inspector looks at you with surprise and then anger, "What the hell is this?! This shipment was supposed to be here ages ago! We've been shifting stuff around to make up for it and then you come waltzing in here… where the hell is the employee who was supposed to deliver this stuff?" A group of workers rushes along with you and the inspector and you as you try to explain what happened. "That fool has been causing us all sorts of problems, and passing on the job to someone as incompetent as you is the last straw! I swear!"]]))
      vn.na(_([[You wait to one side as the cargo is hauled off your ship at breakneck speed and wonder if you should have just dumped the stuff in space. Just as the last of the cargo is taken off your ship, the inspector, who has clearly cooled off a bit, comes up to you and says "Look, I know you were trying to do us a favour but next time don't bother if you can't make it on time. I'm glad you didn't just dump it all into space like some people have done, but I can't pay you for this." He shakes his head and walks away. "That pilot is so fired…"]]))
      vn.func( function ()
         faction.modPlayerSingle("Traders Society", -1)
      end )
   end

   vn.run()
   misn.finish(true)
end

function tick()
   local osd_msg
   if mem.time_limit >= time.get() then -- still in time
      osd_msg = {fmt.f(_("Drop off the goods at {pnt} in the {sys} system (You have {time} remaining)"), {pnt=mem.dest_planet, sys=mem.dest_sys, time=(mem.time_limit - time.get())})}
   else -- missed deadline
      osd_msg = {fmt.f(_("Drop off the goods at {pnt} in the {sys} system (You are late)"), {pnt=mem.dest_planet, sys=mem.dest_sys})}
      mem.intime = false
      hook.rm(mem.date_hook)
   end
   misn.osdCreate(_("Help the Merchant"), osd_msg)
end
