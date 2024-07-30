--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Test of Purification">
 <location>enter</location>
 <chance>100</chance>
 <system>Test of Purification</system>
 <priority>0</priority>
</event>
--]]
--[[
   Trial that provides Cleansing Flames from Kal Vora Obelisk.
   Meant to be the 3rd Obelisk the player does.

   Escape a maze without touching the walls.
--]]
local textoverlay = require "textoverlay"
local chakra = require "luaspfx.chakra_explosion"
local srs = require "common.sirius"

local prevship
local reward = outfit.get("Cleansing Flames")
local obelisk = spob.get("Kal Vora Obelisk")

local drange = 50
local dplace = 80
local dradius = 200

local hook_done
function create ()
   srs.obeliskEnter( obelisk )

   -- Swap player's ship
   local player_ship = player.shipAdd( "Astral Projection Normal", _("Psyche"), _("Psychic powers."), true )
   prevship = player.pilot():name() -- Ship to go back to
   player.shipSwap( player_ship, true )

   -- Set up position
   local pp = player.pilot()
   pp:effectAdd("Astral Projection")
   pp:setDir( math.pi*0.5 )
   pp:setPos( vec2.new(0,-200) )
   pp:intrinsicSet( { -- Ship is too fast otherwise
      accel_mod      = -30,
      speed_mod      = -30,
      turn_mod       = -30,
   }, true ) -- overwrite all

   textoverlay.init( "#y".._("Test of Purification").."#0",
      "#y".._("Escape the Maze"),
      { length=8 } )

   local maze = {
      { type="arc", r=3, a1=2.297, a2=7.894 },
      { type="arc", r=5, a1=-1.545, a2=-0.360 },
      { type="arc", r=5, a1=0.406, a2=1.931 },
      { type="arc", r=5, a1=2.716, a2=4.334 },
      { type="arc", r=7, a1=-1.155, a2=4.694 },
      { type="line", r1=5, r2=7, a=-1.150 },
      { type="line", r1=3, r2=5, a=0.032 },
      { type="line", r1=5, r2=7, a=0.404 },
      { type="line", r1=3, r2=5, a=2.309 },
      { type="line", r1=3, r2=5, a=-2.356 },
      { type="line", r1=5, r2=7, a=-1.952 },
   }
   for k,m in ipairs(maze) do
      -- Do some sampling of positions
      local samples = {}
      if m.type=="arc" then
         for t = 0, 1, 0.01 do
            local a = m.a1*t + m.a2*(1-t)
            local r = m.r * dradius
            samples[#samples+1] = vec2.newP( r, a )
         end
      else
         for t = 0, 1, 0.01 do
            local a = m.a
            local r = (m.r1*t  + m.r2*(1-t)) * dradius
            samples[#samples+1] = vec2.newP( r, a )
         end
      end

      -- Try to place orbs
      for i,s in ipairs(samples) do
         -- Only place nearby ones
         local r = pilot.getInrange( s, dplace )
         if #r <= 0 then
            local p = pilot.add("Psychic Orb", "Independent", s, nil, {ai="dummy"} )
            p:setInvincible(true)
            p:setHostile(true)
            p:effectAdd("Psychic Orb On")
         end
      end
   end

   hook.timer( 0.1, "heartbeat" )

   -- Anything will finish the event
   hook_done = hook.enter( "done" )
end

local end_hook
function heartbeat ()
   local enemies = player.pilot():getEnemies( drange, nil, true )
   if #enemies > 0 then
      for k,e in ipairs(enemies) do
         chakra( e:pos(), vec2.new(), 100 )
         e:rm()
      end

      if not end_hook then
         textoverlay.init( "#r".._("Test Failed").."#0", nil, {length=6})
         end_hook = hook.timer( 6, "cleanup" )
      end
      hook.timer( 0.1, "heartbeat" )
      return
   end

   if player.pos():dist() > 8 * dradius then
      -- All done, so give ability
      srs.sfxGong()
      if player.outfitNum( reward ) > 0 then
         textoverlay.init( "#y".._("Test Completed").."#0", nil, {length=6})
      else
         player.outfitAdd( reward )
         textoverlay.init( "#y"..reward:name().."#0",
            "#y".._("New Flow Ability Unlocked").."#0",
            {length=6})
      end
      hook.timer( 6, "cleanup" )
      return
   end

   hook.timer( 0.1, "heartbeat" )
end

local function cleanup_player ()
   -- Restore previous ship
   player.shipSwap( prevship, true, true )
end

function done ()
   cleanup_player()
   srs.obeliskExit()

   evt.finish()
end

function cleanup ()
   hook.rm( hook_done )
   srs.obeliskCleanup( cleanup_player, evt.finish )
end
