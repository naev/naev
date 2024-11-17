--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Travel Log">
  <location>enter</location>
  <chance>100</chance>
  <unique />
 </event>
 --]]
--[[

   Shiplog Event

   This event records events into a travel log. The travel log is limited
   to a certain number of entries so it doesn't overwhelm the save file.

--]]

local fmt = require "format"

local attacked, lastsys -- Non-persistent state

function create ()
   shiplog.create( "travel", _("Travel Log"), _("Travel"), false, 20 )

   lastsys = system.cur()
   attacked = false

   hook.pilot( player.pilot(), "attacked", "player_attacked" )
   hook.jumpin( "jumpin" )
   hook.land( "land" )
end


function player_attacked ()
   if not attacked then
      shiplog.append( "travel", fmt.f(_("Hostility met in the {sys} system"), {sys=system.cur()} ) )
      attacked = true
   end
end


function jumpin ()
   local s = system.cur()
   shiplog.append( "travel", fmt.f(_("Jumped from the {1} system to the {2} system"), {lastsys, s} ) )
   lastsys = s
   attacked = false
end


function land ()
   local p = spob.cur()
   local s = p:system()
   shiplog.append( "travel", fmt.f(_("Landed on {pnt} in the {sys} system"), {pnt=p, sys=s} ) )
   evt.finish( false )
end
