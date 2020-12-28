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
zalek_name = _("Za'lek Belligerent")
zalek_colour = {1, 0.4, 0.4}
dvaered_image = portrait.getFullPath( portrait.get("Dvaered") )
dvaered_name = _("Dvaered Hooligan")
dvaered_colour = {1, 0.7, 0.3}

function create ()
   if not evt.claim( system.get("Limbo") ) then evt.finish( false ) end

   -- Create scuffle
   local zl = vn.Character.new( zalek_name,
         { image=zalek_image, color=zalek_colour } )
   local dv = vn.Character.new( dvaered_name,
         { image=dvaered_image, color=dvaered_colour } )
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
   -- TODO add meta-factions so we can get them to kill each other
   -- without manual control. This would allow adding more ships so the
   -- player has more time to help and such
   -- Maybe look at warlords_battle.lua
   local pos = planet.get("Minerva Station"):pos()
   local dvpos = pos + vec2.newP( 700, 120 )
   local zlpos = pos + vec2.newP( 500, 300 )
   local unused, dvface = (zlpos-dvpos):polar()
   local unused, zlface = (dvpos-zlpos):polar()
   dv = pilot.addRaw("Dvaered Phalanx", "dvaered", dvpos, "Dvaered" )
   dv:setDir( dvface )
   dv:rename( dvaered_name )
   dv:setNoDisable( true )
   zl = pilot.addRaw("Za'lek Sting", "zalek", zlpos, "Za'lek" )
   zl:setDir( zlface )
   zl:rename( zalek_name )
   zl:setNoDisable( true )
   dv:control()
   dv:attack( zl )
   zl:control()
   zl:attack( dv )

   -- Prepare hooks
   hook.pilot( zl, "death", "zl_dead" )
   hook.pilot( dv, "death", "dv_dead" )
   hook.pilot( zl, "attacked", "zl_attacked" )
   hook.pilot( dv, "attacked", "dv_attacked" )

   -- Messages
   zl_msgs = {
      _("*incoherent expletives*"),
      _("Disgusting Za'lek scum!"),
      _("Your head will make a fine trophy on my ships!"),
      _("I shall wash my hull in your blood!"),
      _("Za'lek trash!"),
   }
   dv_msgs = {
      _("My defense protocols will make short work of you!"),
      _("I'll teach you physics Dvaered punk!"),
      _("My ship's hull is less thick than your Dvaered skull!"),
      _("You Dvaereds smell worse than my vials of thiol!"),
   }
   zl_yelling = (rnd.rnd()<0.5)
   angrytimer = hook.timer( 3000, "angrypeople" )

   -- Set up hooks when it is over
   hook.jumpout("leave")
   hook.land("leave")
end

function cycle_messages( msgs, id )
   id = id or rnd.rnd(1,#msgs)-1
   id = (id+1) % #msgs
   return id+1
end

function angrypeople ()
   -- Take turns yelling
   if not zl_yelling then
      dv_msgid = cycle_messages( dv_msgs, dv_msgid )
      local msg = dv_msgs[ dv_msgid ]
      dv:broadcast( msg )
   else
      zl_msgid = cycle_messages( zl_msgs, zl_msgid )
      local msg = zl_msgs[ zl_msgid ]
      zl:broadcast( msg )
   end
   zl_yelling = not zl_yelling
   angrytimer = hook.timer( 2000, "angrypeople" )
end


function zl_attacked ()
   player_side = "dvaered"
   zl:broadcast( _("Et tu brute?") )
   zl:setHostile(true)
end

function dv_attacked ()
   player_side = "zalek"
   dv:broadcast( _("Et tu brute?") )
   dv:setHostile(true)
end

function zl_dead ()
   if player_side=="zalek" then
      dv:attack( player.pilot() )
   else
      hook.rm( angrytimer )
      if player_side=="dvaered" then
         local holo = imageproc.hologram( dvaered_image )
         vn.clear()
         vn.scene()
         vn.fadein()
         local dv = vn.newCharacter( dvaered_name,
            { image=holo, color=dvaered_colour } )
         dv( _("\"Thank you for the help with the Za'lek scum. Let us celebrate with a drink in the bar down at Minerva Station!\"") )
         vn.fadeout()
         vn.run()
         var.push( "minerva_altercation_helped", "dvaered" )
      else
         dv:broadcast( _("Dust to dust.") )
      end
      dv:land( planet.get("Minerva Station") )
      pilot.toggleSpawn(true)
   end
end

function dv_dead ()
   if player_side=="dvaered" then
      zl:attack( player.pilot() )
   else
      hook.rm( angrytimer )
      if player_side=="dvaered" then
         local holo = imageproc.hologram( zalek_image )
         vn.clear()
         vn.scene()
         vn.fadein()
         local zl = vn.newCharacter( zalek_name,
            { image=holo, color=zalek_colour } )
         zl( _("\"As my computations predicted, the Dvaered scum was no match for the Za'lek superiority. Let us celebrate with a drink down at Minerva Station\"") )
         vn.fadeout()
         vn.run()
         var.push( "minerva_altercation_helped", "zalek" )
      else
         zl:broadcast( _("Good riddance.") )
      end
      zl:land( planet.get("Minerva Station") )
      pilot.toggleSpawn(true)
   end
end

function leave () --event ends on player leaving the system or landing
   evt.finish(true)
end

