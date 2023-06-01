--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Surano Pirates">
 <unique/>
 <location>load</location>
 <chance>100</chance>
</event>
--]]
--[[
   Small blockade of pirates in the Surano system that persist until
   eliminated
--]]
local fmt = require 'format'
local lmisn = require "lmisn"

local mainsys = system.get("Surano")
local piratename = _("Black Goatee")

function create ()
   if not evt.claim{ mainsys } then
      warn(fmt.f(_("Unable to claim {sys} system!"),{sys=mainsys}))
      return
   end

   hook.enter( "enter" )
end

local pirboss
local baddies = {}
local bosses = {}
function enter ()
   if system.cur()~=mainsys then
      return
   end

   -- Get rid of spawns
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Spawn pirates
   local function spawn_pirate( shipname, pos, boss )
      local p = pilot.add( shipname, "Marauder", pos + vec2.newP( 100*rnd.rnd(), rnd.angle() ), nil, {ai="guard"} )
      p:setHostile(true)
      local m = p:memory()
      m.bribe_no = _([["You ain't payin' yer wait outta this one!"]])
      m.refual_no = _([["Do I look like a fuel station?"]])
      if boss then
         p:setLeader( boss )
      end
      hook.pilot( p, "exploded", "pir_gone" )
      hook.pilot( p, "jump",     "pir_gone" )
      hook.pilot( p, "land",     "pir_gone" )
      table.insert( baddies, p )
      return p
   end

   local mainpos = spob.get("Wormhole Surano"):pos() + vec2.newP( 300, 1 )
   pirboss = spawn_pirate( "Pirate Kestrel", mainpos )
   pirboss:rename(piratename)
   pirboss:setVisplayer(true)
   for k,s in ipairs{ "Pirate Starbridge", "Pirate Phalanx", "Pirate Ancestor", "Pirate Admonisher", "Pirate Hyena", "Pirate Shark"} do
      spawn_pirate( s, mainpos, pirboss )
   end

   local pos1 = vec2.new( -8.5e3, 7e3 )
   local boss1 = spawn_pirate( "Pirate Starbridge", pos1 )
   for k,s in ipairs{ "Pirate Admonisher", "Pirate Admonisher", "Pirate Ancestor", "Pirate Vendetta"} do
      spawn_pirate( s, pos1, boss1 )
   end

   local pos2 = vec2.new( 7e3, 9e3 )
   local boss2 = spawn_pirate( "Pirate Rhino", pos2 )
   for k,s in ipairs{ "Pirate Phalanx", "Pirate Ancestor", "Pirate Ancestor", "Pirate Vendetta"} do
      spawn_pirate( s, pos2, boss2 )
   end

   local pos3 = vec2.new( 2e3, -14e3 )
   local boss3 = spawn_pirate( "Pirate Starbridge", pos3 )
   for k,s in ipairs{ "Pirate Shark", "Pirate Shark", "Pirate Hyena", "Pirate Hyena", "Pirate Hyena", "Pirate Ancestor", "Pirate Vendetta" } do
      spawn_pirate( s, pos3, boss3 )
   end

   bosses = { pirboss, boss1, boss2, boss3 }

   hook.timer( 5, "pirate_check" )
end

-- Check to see if player is spotted
function pirate_check ()
   local spotted = false
   local spotter
   for i,b in ipairs(bosses) do
      for k,p in ipairs(b:getVisible()) do
         if p:withPlayer() then
            spotted = true
            spotter = b
            break
         end
      end
   end

   if spotted then
      player.autonavReset( 5 )
      if spotter==pirboss then
         spotter:comm(_("This system is under my control! Eat hot plasma!"))
      else
         spotter:comm(_("This system is under {pirate} control! Scram!"))
      end
      return
   end

   hook.timer( 3, "pirate_check" )
end


function pir_gone ()
   -- Check if done
   local left = 0
   for k,p in ipairs(baddies) do
      if p:exists() then
         left = left+1
      end
   end
   if left > 0 then
      player.msg(fmt.f(n_(
         [[{left} pirate left to clear the blockade.]],
         [[{left} pirates left to clear the blockade.]], left),{left=left}))
      return
   end

   lmisn.sfxVictory()
   player.msg(_("You have cleared the marauder blockade!"))
   evt.finish(true)
end
