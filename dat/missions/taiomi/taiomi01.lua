--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Taiomi 1">
 <unique />
 <chance>0</chance>
 <location>None</location>
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

local reward = taiomi.rewards.taiomi01
local title = _("Secrets of the Hypergates")
local base, basesys = spob.getS("One-Wing Goddard")

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
   misn.setReward(reward)

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
   mem.cargo = misn.cargoAdd( c, 0 )

   misn.osdCreate( title, {
      _("Fly near a hypergate to scan it"),
      fmt.f(_("Return to the {spobname} ({spobsys})"), {spobname=base, spobsys=basesys}),
   } )

   hook.enter( "enter" )
   hook.land( "land" )
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
   if hypergate:pos():dist( player.pos() ) < 10e3 then
      player.autonavReset( 3 )
      luaspfx.sfx( nil, nil, beep )
      player.msg( _("The analyzer has collected the hypergate data and become inert."), true )
      mem.state = 1
      misn.osdActive(2)
      misn.markerRm()
      misn.markerAdd( base )

      misn.cargoRm( mem.cargo )
      local c = commodity.new( N_("Hypergate Data"), N_("In-depth scan data collected from a functional hypergate.") )
      mem.cargo = misn.cargoAdd( c, 0 )
      return
   end

   hook.timer( 0.1, "heartbeat" )
end

function land ()
   if mem.state < 1 or spob.cur()~=base then
      return
   end

   vn.clear()
   vn.scene()
   local s = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )
   vn.na(_([[You board the Goddard and by the time you get off the ship Scavenger is waiting for you.]]))
   s(_([["It must have been no easy feat to collect this data. Let us see what secrets the hypergates hide."]]))
   s(_([[Scavenger's lights dim slightly as they begin to process the troves of information.]]))
   s(_([["Interesting. The hypergate seems to be acting as some sort of conduit. It is not yet clear what it is conducting, however, this gives us a good place to start looking into this."]]))
   s(_([["We do not have much in forms of payment, but please take these tokens that we have collected from the debris. I will be outside preparing for our next steps."]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.na(_([[Scavenger once again elegantly exits the ship, leaving you to yourself.]]))
   vn.done( taiomi.scavenger.transition )
   vn.run()

   player.pay( reward )
   taiomi.log.main(_("You helped the robotic citizens of Taiomi collect important information about the functionality and nature of the hypergates."))
   misn.finish(true)
end
