--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Wild Space Marker">
 <unique />
 <chance>0</chance>
 <location>None</location>
</mission>
--]]
--[[
   Just adds a Marker that goes away when the player enters Wild Space
]]--
local jmpsys = system.get("Jommel")
local tgtsys = system.get("Syndania")
local _jmp = jump.get( jmpsys, tgtsys )

function create ()
   misn.accept()
   misn.markerAdd( tgtsys, "low" )

   local title = _("Way to Wild Space")
   misn.setTitle( title )
   misn.setDesc(_("You obtained information about a jump to Wild Space. Maybe you should see what is there."))
   misn.setReward(_("???"))
   misn.osdCreate( title, {
      _("Jump to Wild Space"),
   } )

   hook.jumpin( "jumpin" )
end

function jumpin ()
   if system.cur()==tgtsys then
      misn.finish(true)
   end
end
