--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Particle Physics 3">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <planet>Katar I</planet>
  <location>Bar</location>
  <done>Za'lek Particle Physics 2</done>
 </avail>
 <notes>
  <campaign>Za'lek Particle Physics</campaign>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Za'lek Particle Physics 03

   Player has to go check a droid malfunction. There will be two drones one
   disable and one hostile. Player must destroy hostile and "hack" the disabled
   one to get the black box.
]]--
local vn = require "vn"
local fmt = require "format"
local zpp = require "common.zalek_physics"
local sokoban = require "minigames.sokoban"

-- luacheck: globals land enter drone_board heartbeat (Hook functions passed by name)

local reward = zpp.rewards.zpp03
local mainpnt, mainsys = planet.getS("Katar I")

function create ()
   misn.finish(true)
   misn.setNPC( _("Noona"), zpp.noona.portrait, zpp.noona.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_([[You approach Noona who doesn't seem too happy.]]))
   n(_([["I have no idea what went wrong."
She has trouble keeping her composure.
"I double checked everything and still it seems like the drones failed to perform the procedure. It should have been a walk in the park!"]]))
   n(_([[She looks at you with hope gleaming in her eyes.
"Say, could you fly out and see what happened? The drones shouldn't be far from here and they don't seem to be moving. If you can, try to access the black box by opening the hull component. Could you look into this for me?"]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   n(_([["I see. You must be busy with other things."
She goes back to ruminating on what to do.]]))
   vn.done( zpp.noona.transition )

   vn.label("accept")
   n(_([["Thanks again. I'll give you the password to the hull so you can access the black box control panel. Depending on the damage, you may have to realign the memory to be able to retrieve the data, but I heard that you mastered that from my colleague. I've sent you the coordinates for the drones so you should be able to find them easy. Best of luck!"]]))
   vn.func( function () accepted = true end )
   vn.done( zpp.noona.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Particle Physics") )
   misn.setReward( fmt.reward(reward) )
   misn.setDesc(fmt.f(_("Investigate the issue with the drones near the particle physics testing site at {sys}."),
      {sys=mainsys}))

   mem.mrk = misn.markerAdd( system.cur() )

   misn.osdCreate( _("Particle Physics"), {
      _("Investigate the drones"),
      _("Return to Katar I"),
   } )

   mem.state = 1

   hook.land( "land" )
   hook.enter( "enter" )
end

function land ()
   if mem.state==1 or planet.cur() ~= mainpnt then
      return
   end

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_([[You land and find Noona waiting outside your ship expectantly.]]))
   n(_([["That was scary! I have no idea what happened with the drone powering up and attacking you."]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.done( zpp.noona.transition )
   vn.run()

   player.pay( reward )
   zpp.log(_("You helped Noona retrieve the black box from one of her drones that malfunctioned."))
   misn.finish(true)
end

local pdis, phost
function enter ()
   if mem.state~=1 or system.cur() ~= mainsys then
      return
   end

   -- Temp faction
   local fdrone = faction.dynAdd( "Za'lek", "haywire_drone", _("Za'lek"), {clear_allies=true, clear_enemies=true} )

   -- Spawn the drones
   -- TODO better location once testing center object is created
   local pkatar = planet.get("Katar"):pos()
   local pkatari = mainpnt:pos()
   local pos = (pkatar - pkatari)*3 + pkatar
   -- Disabled drone
   pdis = pilot.add( "Za'lek Light Drone", fdrone, pos, _("Malfunctioned Drone") )
   pdis:setFriendly(true)
   pdis:setInvincible(true)
   pdis:setHilight(true)
   pdis:setVisplayer(true)
   pdis:control(true)
   pdis:brake()
   hook.pilot( pdis, "disable", "drone_board" )
   -- Hostile drone
   pos = pos + vec2.newP( 100*rnd.rnd(), rnd.angle() )
   phost = pilot.add( "Za'lek Heavy Drone", fdrone, pos, _("Malfunctioned Drone") )
   phost:control(true)
   phost:brake()
   phost:setInvincible(true)
   phost:setVisplayer(true)
   phost:setHilight(true)

   hook.timer( 5, "heartbeat" )
end

local stage = 0
function heartbeat ()
   if stage==0 then
      pilot.comm(_("Noona"), _("I've sent you the drone positions, please get close to investigate."))
      stage = 1
   elseif stage==1 and  pdis:dist( player.pilot() ) < 500 then
      pilot.comm(_("Noona"), _("That is weird, maybe a firmware bug? Wait… I'm detecting a power fluctuation!"))
      stage = 2
   elseif stage==3 then
      stage = 3
   elseif stage==4 then
      pilot.comm(_("Noona"), _("Watch out!"))
      phost:control(false)
      phost:setInvincible(false)
      phost:setNoDisable(true)
      phost:rename(_("Haywire Drone"))
      stage = 5
   elseif stage==5 and not phost:exists() then
      pilot.comm(_("Noona"), _("That was close. Can you try boarding the other drone?") )
      pdis:setActiveBoard(true)
      stage = 6
   end

   if stage < 6 then
      hook.timer( 3, "heartbeat" )
   end
end

local hacked = false
function drone_board ()
   if hacked then
      player.msg(_("The drone's black box has already been extracted."))
      return
   end

   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(_([[You access the drone control panel and jack into the black box.]]))

   sokoban.vn{ levels={4,5}, header="Drone Black Box"}
   vn.func( function ()
      if sokoban.completed() then
         mem.state = 2
         vn.jump("sokoban_done")
         hacked = true
         return
      end
      vn.jump("sokoban_fail")
   end )

   vn.label([[sokoban_done]])
   vn.na(_([[You manage recover the entire black box intact and load the information on your ship.]]))

   vn.label("sokoban_fail")
   vn.na(_([[You failed to access the black box.]]))

   vn.run()

   if hacked then
      local c = misn.cargoNew( N_("Drone Black Box"), N_("The recovered black box of a Za'lek drone.") )
      misn.cargoAdd(c, 0)
      mem.state = 2
   end

   player.unboard()
end
