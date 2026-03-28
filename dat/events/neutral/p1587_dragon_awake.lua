--[[
<?xml version='1.0' encoding='utf8'?>
<event name="P-1587 Dragon Awake">
 <location>load</location>
 <chance>100</chance>
 <cond>player.evtDone("P-1587 Dragon Intro")</cond>
 <unique />
</event>
--]]
local SPB, SYS = spob.getS("P-1587")
local ferals = require "common.ferals"
local ai_setup = require "ai.core.setup"
local pilotai = require "pilotai"
local luaspfx = require 'luaspfx'

function create ()
   if not evt.claim( SYS, true ) then
      return evt.finish(false)
   end

   hook.enter("enter")
end

local dragon
function spawn_dragon()
   local fct = ferals.faction()
   dragon = pilot.add( "Kauweke", fct, SPB, _("Haze Leviathan") )
   dragon:outfitAddSlot( outfit.get("Plasma Burst"), "plasma_burst", true)
   dragon:outfitAddSlot( outfit.get("The Bite - Improved"), "the_bite", true)
   dragon:outfitAddIntrinsic("Corrosion I")
   dragon:outfitAddIntrinsic("Paralyzing Plasma")
   dragon:outfitAddIntrinsic("Crippling Plasma")
   dragon:outfitAddIntrinsic("Corrosion II")
   pilotai.guard( dragon, vec2.newP( 200, rnd.angle() ) )
   ai_setup.setup( dragon ) -- So plasma burst gets used
   -- Make more dragon-y
   dragon:intrinsicSet( "armour_regen", 80 )
   dragon:intrinsicSet( "armour_mod", 30 )
   dragon:intrinsicSet( "shield_mod", 30 )
   dragon:intrinsicSet( "absorb", 20 ) -- The scales have 20 absorb over S&K war plating
   dragon:intrinsicSet( "cooldown_mod", -75) -- Keep your distance if you wanna live
   dragon:intrinsicSet( "mass_mod", 35)
   dragon:intrinsicSet( "jam_chance", 74 ) -- I assume some of the trinkets it's covered in have a scrambling/jamming effect
   dragon:intrinsicSet( "weapon_firerate", -60 ) -- To make up for the cooldown_mod
   dragon:intrinsicSet( "weapon_speed", -60 )
   dragon:intrinsicSet( "weapon_range", 15 )
   dragon:intrinsicSet( "weapon_energy", -50 )
   dragon:intrinsicSet( "energy_mod", 900 )
   dragon:intrinsicSet( "energy_regen_mod", 220 )
   dragon:intrinsicSet( "ew_detect", 100 ) -- To make stealth bombing slightly harder

   dragon:setEnergy( 6 ) -- The faster you take it down, the shorter the volley in phase 2 will be
   dragon:cargoAdd("Luxury Goods", dragon:cargoFree())
   dragon:setHostile(true)
   dragon:setNoDisable(true)
   hook.pilot( dragon, "death", "dragon_died" )
   luaspfx.sfx( dragon:pos(), dragon:vel(), ferals.sfx.spacewhale1 )
   dragon_health()
end

function dragon_health ()
   if not dragon:exists() then
      return
   elseif dragon:armour() < 29 or (dragon:armour() < 66 and player.pilot():ship():size() < 4) then -- If player is in a small ship then something fishy is going on (stealth bomber?) and phase 2 is hardened against stealth bombers, so trigger it earlier)
      luaspfx.sfx( dragon:pos(), dragon:vel(), ferals.sfx.spacewhale2 )
      dragon:effectClear(false, true, true) -- Clear debuffs and jam lockons to live a bit longer
      dragon:jamLockons()
      dragon:intrinsicSet( "armour_regen_mod", -97 ) -- Basically no more armour regen
      dragon:intrinsicSet( "absorb", 60, true ) -- But what remains of the armour will be putting up a fight
      dragon:intrinsicSet( "weapon_firerate", 467, true ) -- Volley of hellfire be upon ye
      dragon:intrinsicSet( "ew_track", -50 ) -- Dodging it is an option though!
      dragon:intrinsicSet( "jam_chance", 173, true ) -- ~3/4 chance for Imperators to be jammed
      dragon:intrinsicSet( "shield_regen_mod", 425 )
      dragon:intrinsicSet( "shielddown_mod", 100 ) -- So in a proper fight the shield regen boost won't do much, but if you're a stealth bomber, then uh oh
      dragon:intrinsicSet( "weapon_speed", 10, true )
      dragon:intrinsicSet( "weapon_energy", 70, true )
      dragon:intrinsicSet( "energy_regen_mod", 50, true )
      dragon:intrinsicSet( "cooldown_mod", 50, true )
      return
   end
   hook.timer(0.1, "dragon_health" )
end

function enter ()
   if system.cur()~=SYS then return end

   SPB:landDeny(true)
   hook.timer( rnd.rnd(3,5), "spawn_dragon" )
end

function dragon_died ()
   SPB:landDeny(false)
   evt.finish(true)
end
