--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 1">
 <flags>
  <unique />
 </flags>
 <avail>
  <chance>0</chance>
  <location>None</location>
 </avail>
 <notes>
  <campaign>Taiomi</campaign>
  <tier>2</tier>
  <done_evt name="Introducing Taiomi" />
 </notes>
</mission>
--]]
--[[
   Taiomi 01

   Player is asked to scan a hypergate with illegal cargo
]]--
local vn = require "vn"
local fmt = require "format"
local audio = require "love.audio"
local luaspfx = require "luaspfx"
local taiomi = require "common.taiomi"

-- luacheck: globals enter heartbeat land (Hook functions passed by name)

local reward = taiomi.rewards.taiomi01
local title = _("Taiomi")

local beep = audio.newSource( "snd/sounds/computer_lock.ogg" )

--[[
   0: mission started
   1: scanned hypergate
--]]
mem.state = 0

function create ()
   misn.accept()

   -- Mission details
   misn.setTitle( title )
   misn.setDesc( _("You have agreed to help the robotic citizens of Taiomi to scan a hypergate. Given the nature of the scanner, care has to be taken to not be scanned by ships.") )
   misn.setReward( fmt.credits(reward) )

   -- Mark hypergates
   for i,s in ipairs(system.getAll()) do
      for j,p in ipairs(s:spobs()) do
         local t = p:tags()
         if t.hypergate and t.active then
            misn.markerAdd( p )
         end
      end
   end

   local c = commodity.new( N_("Subspace Analyzer"), N_("An amalgam of parts recovered from derelict ships heavily modified to be able to perform in-depth analysis of subspace spectrum.") )
   c:illegalto( {"Empire", "Dvaered", "Soromid", "Sirius", "Za'lek", "Frontier"} )
   misn.cargoAdd( c, 0 )

   misn.osdCreate( title, {
      _("Fly near a hypergate to scan it"),
      _("Return to the One-Wing Goddard at Taiomi"),
   } )

   hook.enter( "enter" )
end

local heartbeat_hook, hypergate
function enter ()
   if heartbeat_hook then
      hook.rm( heartbeat_hook )
      heartbeat_hook = nil
      hypergate = nil
   end

   -- Only interested at first
   if mem.state > 0 then
      return
   end

   for k,v in ipairs(system.cur():spobs()) do
      local t = v:tags()
      if t.hypergate and t.active then
         hypergate = v
         hook.timer( 5, "heartbeat" )
         return
      end
   end
end

function heartbeat ()
   if hypergate:pos():dist( player.pos() ) < 5e3 then
      player.autonavReset( 3 )
      luaspfx.sfx( nil, nil, beep )
      player.msg( _("The analyzer has collected the hypergate data."), true )
      mem.state = 1
      misn.osdActive(2)
      misn.markerRm()
      misn.markerAdd( spob.get("One-Winged Goddard") )
      return
   end

   hook.timer( 0.1, "heartbeat" )
end

function land ()
   if mem.state < 1 then
      return
   end

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   s(_([["TODO"]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.main.log(_("You helped the robotic citizens of Taiomi collect important information about the functionality and nature of the hypergates."))
   misn.finish(true)
end
