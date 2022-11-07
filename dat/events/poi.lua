--[[
<?xml version='1.0' encoding='utf8'?>
<event name="POI Generator">
 <location>load</location>
 <chance>100</chance>
 <priority>11</priority>
</event>
--]]
local vn = require "vn"
local poi = require "common.poi"
local fmt = require "format"

function create ()
   hook.board( "board" )
end

function board( p )
   local m = p:memory()
   if not m.natural then
      return
   end

   if poi.done() < 1 then
      return
   end

   if rnd.rnd() < 0.95 then
      return
   end

   local poidata = poi.generate()
   if not poidata then -- failed to generate
      return
   end

   local accept = false

   vn.clear()
   vn.scene()
   vn.transition()
   if poidata.sys:known() then
      vn.na(fmt.f(_([[While boarding the ship, you manage to find some interesting data in the navigation log. It looks like you may be able to follow the lead to something of interest in the {sys} system. Do you wish to download the data?]]),
         {sys="#b"..poidata.sys:name().."#0"}))
   else
      vn.na(_([[While boarding the ship, you manage to find some interesting data in the navigation log. It looks like you may be able to follow the lead to something of interest in what appears to be a nearby system. Do you wish to download the data?]]))
   end

   vn.menu{
      {_("Download the data"), "accept"},
      {_("Ignore."), "leave"},
   }

   vn.label("accept")
   vn.func( function () accept = true end )
   vn.na(_([[You download the data and mark the target system on your navigation console. With nothing else to do on the derelict, you leave it behind, and return to your ship.]]))
   vn.jump("done")

   vn.label("leave")
   vn.na(_([[You ignore the information.]]))

   vn.label("done")
   vn.done()

   if accept then
      poi.setup( poidata )
      naev.missionStart("Point of Interest")
   end
end
