local aisetup = require "ai.core.setup"

local cooldown = 20
local active = 15
local dist = 3000 -- Distance to possible change target of hostiles
local hologram = outfit.get("Combat Hologram Projector")


local function turnon( p, po )
   -- Make sure pilot has a target
   local t = p:target()
   if t == nil then return false end

   -- Set outfit state
   mem.timer = active
   po:state("on")
   po:progress(1)

   -- Create hologram at the same position
   -- TODO hologram-specific AI?
   local s = p:ship()
   local pos = p:pos() + vec2.newP( 0.1, rnd.angle() )
   local np = pilot.add( s:nameRaw(), p:faction(), pos, p:name(), {ai="escort", naked=true} )
   mem.p = np
   np:setHealth( p:health() ) -- Copy health
   np:setNoDeath( true ) -- Dosen't die
   -- Copy outfits
   np:outfitRm("all")
   for k,v in ipairs(p:outfitsList()) do
      -- We don't want recursive holograms
      if v ~= hologram then
         np:outfitAdd( v, 1, true )
      end
   end
   aisetup.setup( np ) -- Initialize AI
   -- No damage and low health
   np:intrinsicSet( {
      launch_damage  = -1000,
      fbay_damage    = -1000,
      fwd_damage     = -1000,
      tur_damage     = -1000,
      armour_mod     = -50,
      shield_mod     = -50,
   }, true ) -- overwrite all
   -- Don't let player attack their own hologram
   if mem.isp then
      np:setInvincPlayer(true)
   end
   -- Exact same position and direction as pilot
   np:setDir( p:dir() )
   np:setVel( p:vel() )
   np:control( true )
   --[[
   local bt = s:class()
   if bt=="Yacht" or bt=="Luxury Yacht" or bt=="Cruise Ship" or bt=="Courier" or bt=="Freighter" or bt=="Bulk Freighter" then
      -- Run away for civilian ships
      np:runaway( t, true )
   else
      -- Attacks pilot's target for non civilian ships
      np:attack( t )
   end
   --]]
   np:attack( t )

   -- Modify randomly targetting of hostiles (probably don't have to go over all ships)
   for k,v in ipairs(p:getEnemies(dist)) do
      if v:target()==p and rnd.rnd() > 0.5 then
         -- Note that this does not currently work with the AI, as it stores
         -- the target in the tasks. Would need some support to change it.
         v:setTarget(np)
      end
   end

   return true
end

local function removehologram()
   -- get rid of hologram if exists
   if mem.p and mem.p:exists() then
      -- Remove all potential escorts
      for k,p in ipairs(mem.p:followers()) do
         p:rm()
      end
      mem.p:rm()
   end
   mem.p = nil
end

local function turnoff( p, po )
   removehologram()

   -- Set outfit state
   mem.timer = cooldown * p:shipstat("cooldown_mod",true)
   po:state("cooldown")
   po:progress(1)
end

function init( p, po )
   removehologram()
   po:state("off")
   mem.timer = 0
   mem.isp = player.pilot()==p -- is player?
end

function update( p, po, dt )
   mem.timer = mem.timer - dt
   if mem.p then
      po:progress( mem.timer / active )
      if not mem.p:exists() or mem.timer < 0 or mem.p:health(true) < 5 then
         turnoff( p, po )
      end
   else
      po:progress( mem.timer / cooldown )
      if mem.timer < 0 then
         po:state("off")
      end
   end
end

-- This should trigger when the pilot is disabled or killed and destroy the
-- hologram if it is up
function ontoggle( p, po, on )
   if on then
      -- Not ready yet
      if mem.timer > 0 then return end

      return turnon( p, po )
   else
      if mem.p then
         turnoff( p, po )
         return true
      end
   end
   return false
end
