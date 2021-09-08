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

attacked_text = _("Hostility met in the %s system")
jump_text = _("Jumped from the %s system to the %s system")
land_text = _("Landed on %s in the %s system")


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
      shiplog.append( "travel", attacked_text:format( system.cur():name() ) )
      attacked = true
   end
end


function jumpin ()
   local s = system.cur()
   shiplog.append( "travel", jump_text:format( lastsys:name(), s:name() ) )
   lastsys = s
   attacked = false
end


function land ()
   local p = planet.cur()
   local s = p:system()
   shiplog.append( "travel", land_text:format( p:name(), s:name() ) )
   evt.finish( false )
end
