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
   -- Set up system
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Create the ships
   local pos = planet.get("Minerva Station"):pos()
   local zlpos = pos + vec2.newP( 700, 120 )
   local dvpos = pos + vec2.newP( 500, 25 )
   zl = pilot.addRaw("Za'lek Sting", "zalek", pos, "Za'lek" )
   dv = pilot.add("Dvaered Phalanx", "dvaered", pos, "Dvaered" )
   zl:rename( _("Za'lek Belligerent") )
   zl:control()
   zl:attack( dv )
   zl:setNoDisable( true )
   dv:rename( _("Dvaered Hooligan") )
   dv:control()
   dv:attack( zl )
   dv:setNoDisable( true )

   -- Prepare hooks
   angrytimer = hook.timer( 3000, "angrypeople" )
   hook.pilot( zl, "death", "zl_dead" )
   hook.pilot( dv, "death", "dv_dead" )

   -- Set up hooks when it is over
   hook.jumpout("leave")
   hook.land("leave")
end

function angrypeople ()
   -- Take turns yelling
   if not zl_yelling then
      zl_yelling = true
      local msg = "asshole"
      dv:broadcast( msg )
   else
      zl_yelling = false
      local msg = "asshole"
      zl:broadcast( msg )
   end
   angrytimer = hook.timer( 2000, "angrypeople" )
end

function zl_dead ()
   hook.rm( angrytimer )
   dv:broadcast( "yay" )
end

function dv_dead ()
   hook.rm( angrytimer )
   zl:broadcast( "yay" )
end

function leave () --event ends on player leaving the system or landing
   evt.finish(true)
end

