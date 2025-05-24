--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Arietis Guardian">
 <unique/>
 <location>load</location>
 <chance>100</chance>
</event>
--]]
--[[
   Odd ship guarding a planet, but for what reasons?

   Obviously, it's a Thurion ship that has lost connection with reality by
   being isolated due to some incident (unknown?) and is eternally guarding some
   pristine databases.
--]]
--local vn = require 'vn'
local fmt = require 'format'
local pilotai = require "pilotai"
local strmess = require "strmess"
local equipopt = require "equipopt"

local mainspb, mainsys = spob.getS("Arietis C")

function create ()
   if not evt.claim{ mainsys } then
      warn(fmt.f("Unable to claim {sys} system!",{sys=mainsys}))
      return
   end

   hook.enter( "enter" )
end

local boss, talktimer
function enter ()
   if system.cur()~=mainsys then
      return
   end

   player.landAllow( false )

   -- Create a new faction
   local fct = faction.dynAdd( "Thurion", _("Unknown"), _("Unknown") )

   local waypoints = {}
   local center = mainspb:pos()
   local N = 15
   for i = 1,N do
      table.insert( waypoints, center + vec2.newP( 1800, 2*math.pi / N * (i-1) ) )
   end

   -- Spawn the boss
   boss = pilot.add( "Thurion Apprehension", fct, waypoints[rnd.rnd(1,#waypoints)], _("Guardian of Arietis"), {ai="baddiepatrol", naked=true} )
   equipopt.thurion( boss, {
      type_range = {
         ["Launcher"] = { max = 0 },
      },
   } )
   hook.pilot( boss, "death", "beatboss" )
   hook.pilot( boss, "disable", "beatboss" )
   hook.pilot( boss, "hail", "comm" )
   hook.pilot( boss, "attacked", "attacked" )

   -- You spin me right around baby
   pilotai.patrol( boss, waypoints )
   local mem = boss:memory()
   mem.enemyclose = nil -- Thurions currently have a really short range for some reason
   mem.aggressive = true
   mem.norun = true
   mem.loiter_compensate_vel = false -- Looks much nicer

   talktimer = rnd.rnd(20,40)
   hook.timer( 1, "heartbeat" )
end

local done = false
function beatboss ()
   if boss:exists() then
      boss:setDisable()
   end
   done = true
   player.landAllow(true)
   evt.finish(true)
end

local messages = {
   "Stay away!",
   "Mine, all mine!",
   "Where are they?",
   "All silence...",
}
local message_id, closedist, closerdist
function heartbeat ()
   if not boss:exists() or done then return end

   local dist = mainspb:pos():dist( player.pos() )
   if dist < 5e3 then
      boss:setHostile(true)
      player.autonavReset( 5 )
      return
   elseif not closerdist and dist < 7e3 then
      closerdist = true
      boss:broadcast( strmess.tobinary("Stop", true).._("!") )
      player.autonavReset( 1 )
   elseif  not closedist and dist < 10e3 then
      closedist = true
      talktimer = rnd.rnd( 30, 40 )
      boss:broadcast( strmess.tobinary("Don't get closer", true).._("!") )
      player.autonavReset( 1 )
   else
      talktimer = talktimer - 1
      if talktimer <= 0 then
         message_id = (message_id or rnd.rnd(1,#messages)) % #messages + 1
         boss:broadcast( strmess.tobinary(messages[message_id], true) )
         talktimer = rnd.rnd( 30, 40 )
      end
   end

   hook.timer( 1, "heartbeat" )
end

function attacked ()
   done = true
end

function comm ()
   boss:comm( [["]]..strmess.tobinary("Leave this place, and you shall be forgotten.", true)..[["]] )
   player.commClose()
end
