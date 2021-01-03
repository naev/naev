--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Station Altercation 1">
 <trigger>none</trigger>
 <chance>0</chance>
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
local imageproc = require 'imageproc'

zalek_holo = portrait.getFullPath( "zalek_thug1" )
zalek_image = "zalek_thug1.png"
zalek_name = _("Za'lek Belligerent")
zalek_colour = {1, 0.4, 0.4}
dvaered_holo = portrait.getFullPath( "dvaered_thug1" )
dvaered_image = "dvaered_thug1.png"
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
   vn.na( _("You hear a large commotion in one of the wings of Minerva Station. As you approach you can make out what seems to be an altercation between a group of Za'lek and Dvaered ruffians.") )
   vn.appear( {zl, dv} )
   dv( _("\"You were using a bloody computation drone to cheat your dirty Za'lek stink!\"") )
   zl( _("\"Your logic makes no sense. Are you sure the excessive amounts of alcohol are making your rotten brain hallucinate?\"") )
   dv( _("\"You won't trick me with your fancy words. I know a cheater when I see one!\"") )
   zl( _("\"I wouldn't trust your judgment on bricks let alone anything that requires minimal intellect.\"") )
   dv( _("\"You Za'lek punk. Bring your ass outside and I will fill your ship with holes just like your lies!\"") )
   zl( _("\"When you are getting eviscerated by my drones, I hope you realize you have brought this upon yourself, Dvaered trash!\"") )
   vn.disappear( {zl, dv} )
   vn.na( _("The Za'lek and Dvaered ruffians storm off to their ships to apparently fight to the death to solve their quarrel. Truly civilized individuals.\nIt seems like it could be an opportunity to curry favour with either one of the factions if you wished to intervene in their fight.") )
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
   -- player has more time to help and such. This should also allow
   -- getting rid of all the ridiculous "attack" cases.
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
   dv:face(zl)
   zl:control()
   zl:face(dv)

   -- Prepare hooks
   hook.pilot( zl, "death", "zl_dead" )
   hook.pilot( dv, "death", "dv_dead" )
   hook.pilot( zl, "attacked", "zl_attacked" )
   hook.pilot( dv, "attacked", "dv_attacked" )

   -- Messages
   dv_msgs = {
      _("*incoherent expletives*"),
      _("Disgusting Za'lek scum!"),
      _("Your head will make a fine trophy on my ships!"),
      _("I shall wash my hull in your blood!"),
      _("Za'lek trash!"),
   }
   zl_msgs = {
      _("My defense protocols will make short work of you!"),
      _("I'll teach you physics Dvaered punk!"),
      _("My ship's hull is less thick than your Dvaered skull!"),
      _("You Dvaereds smell worse than my vials of thiol!"),
   }
   zl_yelling = (rnd.rnd()<0.5)
   timetonextanger = 1000
   angrytimer = hook.timer( timetonextanger, "angrypeople" )
   attacktimer = hook.timer( 5000, "startattack" )
   player_side = nil

   -- Set up hooks when it is over
   hook.jumpout("leave")
   hook.land("leave")
end

function startattack ()
   if not fighting_started then
      dv:taskClear()
      zl:taskClear()
      dv:attack( zl )
      zl:attack( dv )
      fighting_started = true
   end
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
   timetonextanger = timetonextanger + 2000 -- slowly talk less and less
   angrytimer = hook.timer( timetonextanger, "angrypeople" )
end

function zl_attacked( victim, attacker )
   if attacker ~= player.pilot() then return end
   if not fighting_started then
      zl:taskClear()
      zl:attack( player.pilot() )
      dv:taskClear()
      dv:attack( zl )
      fighting_started = true
   end
   if player_side=="zalek" then
      player_side = "neither"
   elseif player_side == nil then
      player_side = "dvaered"
      zl:broadcast( _("Et tu brute?") )
      zl:setHostile(true)
      dv:setFriendly(true)
      dv:taskClear()
      dv:attack( zl )
   end
end

function dv_attacked( victim, attacker )
   if attacker ~= player.pilot() then return end
   if not fighting_started then
      dv:taskClear()
      dv:attack( player.pilot() )
      zl:taskClear()
      zl:attack( zl )
      fighting_started = true
   end
   if player_side=="dvaered" then
      player_side = "neither"
   elseif player_side == nil then
      player_side = "zalek"
      dv:broadcast( _("Et tu brute?") )
      dv:setHostile(true)
      zl:setFriendly(true)
      zl:taskClear()
      zl:attack( dv )
   end
end

function zl_dead ()
   if player_side=="zalek" or player_side=="neither" then
      dv:attack( player.pilot() )
   else
      hook.rm( angrytimer )
      if dv:exists() then
         if player_side=="dvaered" then
            dv:brake()
            dv:hailPlayer()
            hail.pilot( dv, "hail", "dv_hail" )
         else
            dv:broadcast( _("Dust to dust.") )
         end
      end
      pilot.toggleSpawn(true)
   end
end

function dv_hail ()
   local holo = imageproc.hologram( dvaered_holo )
   vn.clear()
   vn.scene()
   vn.fadein()
   local dvc = vn.newCharacter( dvaered_name,
      { image=holo, color=dvaered_colour } )
   dvc( _("\"Thank you for the help with the Za'lek scum. Let us celebrate with a drink in the bar down at Minerva Station!\"") )
   vn.fadeout()
   vn.run()
   var.push( "minerva_altercation_helped", "dvaered" )
   dv:land( planet.get("Minerva Station") )
   player.commClose()
end

function dv_dead ()
   if player_side=="dvaered" or player_side=="neither" then
      zl:attack( player.pilot() )
   else
      hook.rm( angrytimer )
      if zl:exists() then
         if player_side=="zalek" then
            zl:brake()
            zl:hailPlayer()
            hook.pilot( zl, "hail", "zl_hail" )
         else
            zl:broadcast( _("Good riddance.") )
         end
      end
      pilot.toggleSpawn(true)
   end
end

function zl_hail ()
   local holo = imageproc.hologram( zalek_holo )
   vn.clear()
   vn.scene()
   vn.fadein()
   local zlc = vn.newCharacter( zalek_name,
      { image=holo, color=zalek_colour } )
   zlc( _("\"As my computations predicted, the Dvaered scum was no match for the Za'lek superiority. Let us celebrate with a drink down at Minerva Station\"") )
   vn.fadeout()
   vn.run()
   var.push( "minerva_altercation_helped", "zalek" )
   zl:land( planet.get("Minerva Station") )
   player.commClose()
end

function leave () --event ends on player leaving the system or landing
   evt.finish(true)
end

