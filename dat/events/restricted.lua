--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Restricted zone">
 <location>enter</location>
 <chance>100</chance>
 <cond>system.cur():tags().restricted~=nil</cond>
</event>
--]]
--[[
   Establishes zones that are off limits to the player unless they are friendly with the faction.
--]]
local fmt = require "format"
local sm = require "luaspfx.spacemine"
local careful = require "ai.core.misc.careful"
local lanes = require "ai.core.misc.lanes"

local sysfct

function create ()
   local csys = system.cur()

   -- We assume dominant faction is the one we want here
   sysfct = csys:faction()

   -- Collective systems are restricted to avoid missions, but we don't want mines and stuff
   if sysfct == faction.get("Collective") then
      return
   end

   -- Add space mines
   local L = lanes.get( sysfct, "non-hostile" )
   for i = 1,rnd.rnd(10,30) do
      -- The sqrt here makes it so the samples are uniform in Euclidean coordinates
      local rad = system.cur():radius()*0.9*math.sqrt(rnd.rnd())
      local pos = careful.getSafePointL( L, nil, vec2.new(), rad, 2e3, 2e3, 2e3 )
      if pos then
         sm( pos, nil, sysfct, {
            hostile = true,
         } )
      end
   end

   hook.timer( 20, "make_hostile" )
   hook.timer( 5, "msg_buoy" )
   hook.land( "endevent" )
   hook.jumpout( "endevent" )
end

local msg_list = {
   ["Za'lek"] = _("WARNING: Entering militarized zone. Unauthorized access will be met with force."),
   ["Dvaered"] = _("WARNING: YOU HAVE ENTERED A RESTRICTED ZONE. LEAVE IMMEDIATELY OR FACE THE CONSEQUENCES."),
   ["Empire"] = _("WARNING: This is a restricted military system. Unauthorized ships will be shot on sight."),
   ["Proteron"] = _("WARNING: Violating travel restrictions endangers both you and the State. Leave now, before we enforce the death penalty.")
}
local msg_delay
function msg_buoy ()
   local msg = msg_list[ sysfct:nameRaw() ]
   if not msg then
      -- Gneeric message
      msg = _("WARNING: Unauthorized entry to a restricted area will be met with lethal force. Leave immediately.")
   end
   if not msg_delay then
      -- Probably going to die but be nice and add reset autonav the first time
      player.autonavReset( 5 )
   end
   local pf = player.pilot():faction()
   local col
   if sysfct:areAllies( pf ) then
      col = "F"
   elseif sysfct:areEnemies( pf ) then
      col = "H"
   else
      col = "N"
   end
   pilot.broadcast( fmt.f(_("{faction} Message Buoy"),{faction=sysfct}), "#r"..msg.."#0", col )
   msg_delay = (msg_delay or 8) * 2
   hook.timer( msg_delay, "msg_buoy" )
end

function make_hostile ()
   if not sysfct:areAllies( player.pilot():faction() ) then
      for _k,p in ipairs(pilot.get{sysfct}) do
         p:setHostile(true)
      end
   end
   -- Keep on repeating as the spawn
   hook.timer( 20, "make_hostile" )
end

function endevent ()
   evt.finish()
end
