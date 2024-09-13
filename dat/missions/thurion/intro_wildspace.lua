--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Wild Space Thurion Intro - Helper">
 <unique/>
 <location>none</location>
 <chance>0</chance>
</mission>
--]]
--[[
   Just used to create an OSD. Doesn't really affect anything.
--]]
local fmt = require "format"
local landspb, landsys = spob.getS("FD-24")
local title = _("Cordial Invitation")
function create ()
   misn.accept()
   misn.setTitle(title)
   misn.setDesc(fmt.f(_([[You have been invited by a mysterious faction to land on {spb} ({sys} system).]]),
      {spb=landspb, sys=landsys}))
   misn.setReward(_("???"))
   misn.osdCreate( title, {
      fmt.f(_([[Land on {spb} ({sys} system)]]), {spb=landspb, sys=landsys}),
   } )
   misn.markerAdd( landspb )
   hook.land("land")
end
function land ()
   if spob.cur()==landspb then
      misn.finish(true)
   end
end
