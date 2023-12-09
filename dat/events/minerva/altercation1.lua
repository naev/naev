--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Station Altercation 1">
 <location>none</location>
 <chance>0</chance>
 <notes>
  <campaign>Minerva</campaign>
  <requires name="Minerva Station">Random event when gained more than 10 tokens</requires>
  <provides name="Minerva Altercation 1" />
 </notes>
</event>
--]]

--[[
-- Some Za'lek and Dvaered get in a scuffle and go outside to fight.
--
-- Is repeatable!
--
-- Triggered from station.lua
--]]
local vn = require 'vn'
local love_shaders = require 'love_shaders'
local minerva = require 'common.minerva'

local zalek_holo = "zalek_thug1.png"
local zalek_image = "zalek_thug1.png"
local zalek_colour = {1, 0.4, 0.4}
local dvaered_holo = "dvaered_thug1.png"
local dvaered_image = "dvaered_thug1.png"
local dvaered_colour = {1, 0.7, 0.3}

-- Non-persistent state
local dv, zl, dv_msgid, zl_msgid, zl_yelling, attacktimer, angrytimer, timetonextanger, fighting_started, player_side

-- Messages
local dv_msgs = {
   _("*incoherent expletives*"),
   _("Disgusting Za'lek scum!"),
   _("Your head will make a fine trophy on my ships!"),
   _("I shall wash my hull in your blood!"),
   _("Za'lek trash!"),
}
local zl_msgs = {
   _("My defence protocols will make short work of you!"),
   _("I'll teach you physics Dvaered punk!"),
   _("My ship's hull is less thick than your Dvaered skull!"),
   _("You Dvaereds smell worse than my vials of thiol!"),
}

function create ()
   if not evt.claim( system.get("Limbo") ) then evt.finish( false ) end

   -- Create scuffle
   local vn_zl = vn.Character.new( _("Za'lek Belligerent"),
         { image=zalek_image, colour=zalek_colour, pos="left" } )
   local vn_dv = vn.Character.new( _("Dvaered Hooligan"),
         { image=dvaered_image, colour=dvaered_colour, pos="right" } )
   vn.clear()
   vn.scene()
   vn.music( minerva.loops.conflict )
   vn.transition()
   vn.na( _("You hear a large commotion in one of the wings of Minerva Station. As you approach you can make out what seems to be an altercation between a group of Za'lek and Dvaered ruffians.") )
   vn.appear( {vn_zl, vn_dv} )
   vn_dv( _([["You were using a bloody computation drone to cheat, you dirty Za'lek stink!"]]) )
   vn_zl( _([["Your logic makes no sense. Are you sure the excessive amounts of alcohol aren't making your rotten brain hallucinate?"]]) )
   vn_dv( _([["You won't trick me with your fancy words. I know a cheater when I see one!"]]) )
   vn_zl( _([["I wouldn't trust your judgment on bricks let alone anything that requires minimal intellect."]]) )
   vn_dv( _([["You Za'lek punk. Bring your ass outside and I will fill your ship with holes just like your lies!"]]) )
   vn_zl( _([["When you are getting eviscerated by my drones, I hope you realize you have brought this upon yourself, Dvaered trash!"]]) )
   vn.disappear( {vn_zl, vn_dv} )
   vn.na( _("The Za'lek and Dvaered ruffians storm off to their ships to apparently fight to the death to solve their quarrel. Truly civilized individuals.\nIt seems like it could be an opportunity to curry favour with either one of the factions if you wished to intervene in their fight.") )
   vn.run()

   hook.takeoff( "takeoff" )
end

-- Scuffle between Za'lek and Dvaered
function takeoff ()
   -- Set up system
   pilot.clear()
   pilot.toggleSpawn(false)

   -- Meta factions
   local fdv = faction.dynAdd( "Dvaered", "dv_thug", _("Dvaered Thug") )
   local fzl = faction.dynAdd( "Za'lek", "zl_thug", _("Za'lek Thug") )
   faction.dynEnemy( fdv, fzl )
   -- Create the ships
   local pos = spob.get("Minerva Station"):pos()
   local dvpos = pos + vec2.newP( 700, math.rad(120) )
   local zlpos = pos + vec2.newP( 500, math.rad(300) )
   local _dvdist, dvface = (zlpos-dvpos):polar()
   local _zldist, zlface = (dvpos-zlpos):polar()
   dv = pilot.add("Dvaered Phalanx", "dv_thug", dvpos, _("Dvaered Hooligan"), {ai="dvaered"} )
   local aimem = dv:memory()
   aimem.doscans = false
   dv:setDir( dvface )
   dv:setNoDisable( true )
   zl = pilot.add("Za'lek Sting", "zl_thug", zlpos, _("Za'lek Belligerent"), {ai="zalek"} )
   aimem = zl:memory()
   aimem.doscans = false
   zl:setDir( zlface )
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

   zl_yelling = (rnd.rnd()<0.5)
   timetonextanger = 1.0
   angrytimer = hook.timer( timetonextanger, "angrypeople" )
   attacktimer = hook.timer( 5.0, "startattack" )
   player_side = nil

   -- Set up hooks when it is over
   hook.jumpout("leave")
   hook.land("leave")
end

function startattack ()
   if not fighting_started then
      dv:control(false)
      zl:control(false)
      fighting_started = true
   end
end

local function cycle_messages( msgs, id )
   id = id or rnd.rnd(1,#msgs)-1
   id = (id+1) % #msgs
   return id+1
end

function angrypeople ()
   -- Take turns yelling
   if not zl_yelling then
      dv_msgid = cycle_messages( dv_msgs, dv_msgid )
      local msg = dv_msgs[ dv_msgid ]
      if dv and dv:exists() then
         dv:broadcast( msg )
      end
   else
      zl_msgid = cycle_messages( zl_msgs, zl_msgid )
      local msg = zl_msgs[ zl_msgid ]
      if zl and zl:exists() then
         zl:broadcast( msg )
      end
   end
   zl_yelling = not zl_yelling
   timetonextanger = timetonextanger + 2.0 -- slowly talk less and less
   angrytimer = hook.timer( timetonextanger, "angrypeople" )
end

function zl_attacked( _victim, attacker )
   if not attacker or not attacker:withPlayer() then return end
   startattack()
   if player_side=="zalek" then
      player_side = "neither"
   elseif player_side == nil then
      player_side = "dvaered"
      zl:broadcast( _("Et tu brute?") )
      zl:setHostile(true)
      dv:setFriendly(true)
   end
end

function dv_attacked( _victim, attacker )
   if not attacker or not attacker:withPlayer() then return end
   startattack()
   if player_side=="dvaered" then
      player_side = "neither"
   elseif player_side == nil then
      player_side = "zalek"
      dv:broadcast( _("Et tu brute?") )
      dv:setHostile(true)
      zl:setFriendly(true)
   end
end

function zl_dead ()
   if player_side=="zalek" or player_side=="neither" then
      dv:setHostile(true)
      dv:control(false)
   else
      hook.rm( angrytimer )
      hook.rm( attacktimer )
      if dv:exists() then
         if player_side=="dvaered" then
            dv:control()
            dv:brake()
            dv:hailPlayer()
            hook.pilot( dv, "hail", "dv_hail" )
         else
            dv:broadcast( _("Dust to dust.") )
         end
      end
      pilot.toggleSpawn(true)
   end
end

function dv_hail ()
   vn.clear()
   vn.scene()
   local dvc = vn.newCharacter( _("Dvaered Hooligan"),
      { image=dvaered_holo, colour=dvaered_colour, shader=love_shaders.hologram() } )
   vn.transition("electric")
   dvc( _([["Thank you for the help with the Za'lek scum. Let us celebrate with a drink in the bar down at Minerva Station!"]]) )
   vn.done("electric")
   vn.run()
   var.push( "minerva_altercation_helped", "dvaered" )
   dv:control(true)
   dv:land( spob.get("Minerva Station") )
   dv:setInvincible(true)
   minerva.log.misc(_("You helped a Dvaered pilot get rid of a Za'lek pilot they had a quarrel with."))
   player.commClose()
end

function dv_dead ()
   if player_side=="dvaered" or player_side=="neither" then
      zl:setHostile(true)
      zl:control(false)
   else
      hook.rm( angrytimer )
      hook.rm( attacktimer )
      if zl:exists() then
         if player_side=="zalek" then
            zl:control()
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
   vn.clear()
   vn.scene()
   local zlc = vn.newCharacter( _("Za'lek Belligerent"),
      { image=zalek_holo, colour=zalek_colour, shader=love_shaders.hologram() } )
   vn.transition("electric")
   zlc( _([["As my computations predicted, the Dvaered scum was no match for the Za'lek superiority. Let us celebrate with a drink down at Minerva Station"]]) )
   vn.done("electric")
   vn.run()
   var.push( "minerva_altercation_helped", "zalek" )
   zl:control(true)
   zl:land( spob.get("Minerva Station") )
   zl:setInvincible(true)
   minerva.log.misc(_("You helped a Za'lek pilot get rid of a Dvaered pilot they had a quarrel with."))
   player.commClose()
end

function leave () --event ends on player leaving the system or landing
   local prob = var.peek("minerva_altercation_probability") or 0.4
   var.push( "minerva_altercation_probability", prob/1.5 )
   evt.finish(true)
end
