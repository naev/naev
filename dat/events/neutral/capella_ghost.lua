--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Capella Ghost">
 <location>enter</location>
 <chance>100</chance>
 <system>Capella</system>
</event>
--]]
--[[
   Hostile ghost in hidden system Capella (https://en.wikipedia.org/wiki/Capella)

   Pretty obviously of Sirius origin; my headcanon is a strong awakened got stranded in there
   after the Incident and after their death left an echo that was further empowered by the Nebula.

   Every cycle, it conjures another semi-illusory star to shine together with the four true ones.

   Could be expanded into a mini-campaign at some point; perhaps after the player's first visit,
   the ghost starts going on outings and blowing up ships in adjacent systems, which prompts action
   from either the Sirius or the Dreamers to pacify it somehow.

   As is, it's a neat secret and a gimmicky Vixilium mining spot to reward the player with.
--]]
local pilotai = require "pilotai"
local ai_setup = require "ai.core.setup"
local vn = require "vn"
local tut = require "common.tutorial"
local ghost
function create ()
   local fct = faction.dynAdd("Dummy", "metaphysical", _("Metaphysical"))
   ghost = pilot.add( "Astral Projection Greater", fct, vec2.new(), _("Starlighter"), {ai="baddiepos"} )
   ghost:effectAdd("Astral Projection")
   local m = ghost:memory()
   m.comm_no = _("No response.")
   pilotai.guard( ghost, vec2.new() )
   ghost:setInvincible()
   ghost:setInvisible()
   --ghost:setNoRender()
   ghost:intrinsicSet("weapon_energy", 99999) -- Pew pew is beneath us here
   ghost:intrinsicSet("action_speed", 300)
   ghost:intrinsicSet("cooldown_mod", -95)
   ghost:intrinsicSet("speed_mod", -40)
   ghost:intrinsicSet("accel_mod", -10)
   ghost:intrinsicSet("turn_mod", -30)
   ghost:outfitAdd("Feather Drive")
   ghost:outfitAdd("Cleansing Flames")
   ai_setup.setup( ghost )
   ghost:setHostile()
   hook.update("update")
   hook.enter("enter")
   hook.timer(4, "timer")
end
local timer_done = false
function timer()
   timer_done = true
   if not var.peek("capella_visited") then
      local sai = tut.vn_shipai()
      vn.clear()
      vn.scene()
      vn.transition()

      vn.na(_([[As you're getting your bearings after the hyperspace jump, an eerie feeling washes over you. There's something peculiar about this place, and the sensor feeds confirm it is anything but mundane.]]))
      vn.appear( sai, tut.shipai.transition )
      sai(_([["There are several abnormalities in the ambient radiation and gravitational readings. Also, a system with this many stars would not be statistically possible under ordinary physics."]]))
      sai(_([["Something almost like another ship is nearby, but I am unable to pinpoint the exact location, nor assess how much danger we could be in. I would suggest flying in stealth mode and exercising heavy caution as long as we are here.]]))
      vn.na(_([[You can feel a presence start to stir against your very being, as if you were being watched. It may not be wise to linger.]]))
      vn.disappear( sai, tut.shipai.transition )

      vn.run()
      var.push("capella_visited", true)
   end
end
function update( dt )
   if not ghost:exists() then return end
   for k,p in ipairs(ghost:getEnemies(960, nil, nil, true, true)) do
      local dmg = (p:shipstat("shield_regen", true) + p:shipstat("armour_regen", true) + 1) * dt
      p:damage( dmg * 5.5, dmg * 3.5, 5., "energy", ghost )
   end
   if timer_done then
      ghost:intrinsicSet("ew_detect", ghost:intrinsicGet("ew_detect") + 0.160 * dt, true) -- some risk/reward over time: being unstealthed/getting to mine more
   end
end
function enter ()
   evt.finish()
end
