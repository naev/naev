--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Station Altercation 1">
 <trigger>land</trigger>
 <chance>100</chance>
 <flags>
  <unique />
 </flags>
</event>
--]]

--[[
-- Some Za'lek and Dvaered get in a scuffle and go outside to fight.
--
-- Triggered from station.lua
--]]
local portrait = require "portrait"
local vn = require 'vn'

zalek_image = portrait.getFullPath( "none" )
dvaered_image = portrait.getFullPath( portrait.get("Dvaered") )

function create ()
   if not evt.claim( system.get("Limbo") ) then evt.finish( false ) end

   -- Create scuffle
   local zl = vn.Character.new( _("Za'lek"),
         { image=zalek_image, color={1, 0.4, 0.4} } )
   local dv = vn.Character.new( _("Dvaered"),
         { image=dvaered_image, color={1, 0.7, 0.3} } )
   vn.clear()
   vn.scene()
   vn.fadein()
   vn.na( _("BLARG") )
   vn.appear( {zl, dv} )
   dv( _("\"BLARG\"") )
   zl( _("\"BLARG\"") )
   vn.fadeout()
   vn.run()

   hook.takeoff( "takeoff" )
end

-- Scuffle between Za'lek and Dvaered
function takeoff ()
end

