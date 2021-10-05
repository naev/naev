--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Travel Log">
  <trigger>enter</trigger>
  <chance>100</chance>
  <flags>
   <unique />
  </flags>
 </event>
 --]]
--[[

   Shiplog Event

   This event records events into a travel log. The travel log is limited
   to a certain number of entries so it doesn't overwhelm the save file.

--]]

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
      shiplog.append( "travel", _("Hostility met in the %s system"):format( system.cur():name() ) )
      attacked = true
   end
end


function jumpin ()
   local s = system.cur()
   shiplog.append( "travel", _("Jumped from the %s system to the %s system"):format( lastsys:name(), s:name() ) )
   lastsys = s
   attacked = false
end


function land ()
   local p = planet.cur()
   local s = p:system()
   shiplog.append( "travel", _("Landed on %s in the %s system"):format( p:name(), s:name() ) )
   evt.finish( false )
end
