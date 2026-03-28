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
local ghost
function create ()
   local fct = faction.dynAdd("Dummy", "metaphysical", _("Metaphysical"))
   ghost = pilot.add( "Astral Projection Greater", fct, vec2.new(), _("Starlighter"), {ai="baddiepos"} )
   local m = ghost:memory()
   m.comm_no = "No response."
   pilotai.guard( ghost, vec2.new() )
   ghost:setInvincible()
   ghost:setInvisible()
   ghost:setNoRender()
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
   hook.timer(5, "timer")
end
local intrinsicd = {}
local timer_done = false
function timer()
   timer_done = true
end
function update( dt )
   if not ghost:exists() then return end
   for k,p in ipairs(ghost:getEnemies(960, nil, nil, true, true)) do
      local dmg = (p:shipstat("shield_regen", true) + p:shipstat("armour_regen", true) + 1) * dt
      p:damage( dmg * 5.5, dmg * 3.5, 5., "energy", ghost )
   end
   if timer_done then
      ghost:intrinsicSet("ew_detect", ghost:intrinsicGet("ew_detect") + 0.155 * dt, true) -- some risk/reward over time: being unstealthed/getting to mine more
      for k,p in ipairs(pilot.get()) do
         p:intrinsicSet("mining_bonus", p:intrinsicGet("mining_bonus") + 0.155 * dt, true)
         p:intrinsicSet("jump_warmup", p:intrinsicGet("jump_warmup") + 0.310 * dt, true) -- ...and a tiny sprinkle of evil
         if not inlist(intrinsicd, p) then table.insert(intrinsicd, p) end
      end
   end
end
function enter ()
   for k,p in ipairs(intrinsicd) do if p:exists() then 
   p:intrinsicSet("mining_bonus", 0, true) 
   p:intrinsicSet("jump_warmup", 0, true) 
   end end
   evt.finish()
end
