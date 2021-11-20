--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Restricted zone">
 <trigger>enter</trigger>
 <chance>100</chance>
 <cond>system.cur():tags().restricted~=nil</cond>
</event>
--]]
--[[
   Establishes zones that are off limits to the player unless they are friendly with the faction.
--]]
local fmt = require "format"

local sysfct
-- luacheck: globals endevent make_hostile msg_buoy (Hook functions passed by name)

function create ()
   local csys = system.cur()

   -- We assume dominant faction is the one we want here
   sysfct = csys:faction()

   hook.timer( 20, "make_hostile" )
   hook.timer( 5, "msg_buoy" )
   hook.land( "endevent" )
   hook.jumpout( "endevent" )
end

local msg_list = {
   ["Za'lek"] = _("#rWARNING: Entering militarized zone. Unauthorized access will be met with force.#0"),
   ["Dvaered"] = _("#rWARNING: YOU HAVE ENTERED A RESTRICTED ZONE. LEAVE IMMEDIATELY OR FACE THE CONSEQUENCES.#0"),
   ["Empire"] = _("#rWARNING: This is a restricted military system. Unauthorized ships will be shot on sight.#0"),
}
local msg_delay
function msg_buoy ()
   local msg = msg_list[ sysfct:nameRaw() ]
   if not msg then
      -- Gneeric message
      msg = _("#rWARNING: Unauthorized entry to restricted area will be met with force. Leave immediately.#0")
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
   player.msg(fmt.f(_([[#{col}{fct} Message Buoy>#0 "{msg}"]]), {col=col, fct=sysfct, msg=msg} ))
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

