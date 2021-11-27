--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Particle Physics 5">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <planet>Katar I</planet>
  <location>Bar</location>
  <done>Za'lek Particle Physics 4</done>
 </avail>
 <notes>
  <campaign>Za'lek Particle Physics</campaign>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Za'lek Particle Physics 05

   Mainly a cutscene where the player takes the materials and does an experiment which goes weird.
]]--
local vn = require "vn"
local fmt = require "format"
local zpp = require "common.zalek_physics"
local sokoban = require "minigames.sokoban"

-- luacheck: globals land enter drone_board heartbeat update renderbg (Hook functions passed by name)

--local reward = zpp.rewards.zpp05 -- No reward
local mainpnt, mainsys = planet.getS("Katar I")

function create ()
   if not misn.claim( mainsys ) then
      misn.finish(false)
   end
   misn.setNPC( _("Noona"), zpp.noona.portrait, zpp.noona.description )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_([[You approach Noona who seems giddy with excitement.]]))
   n(_([["It's almost ready! Thanks to you I finally have everything ready for the test. However, after revising the drone firmware, I think this time it would be best to not rely on drones and manually perform the experiment. Sometimes it's just best to not rely on all the fancy technology and do it old fashion style. Would you be willing to perform the experiment for me? Your ship should be sturdy enough that I don't think there is any possibility for any harm."]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   n(_([["I see. You must be busy with other things."]]))
   vn.done( zpp.noona.transition )

   vn.label("accept")
   n(_([["Thanks! So what you brought back last time were some nebula crystals. Nothing is really known about them, but sometimes, they are found in old wrecks or asteroids in the nebula. We know that these crystals seem to have some weird properties, but nothing conclusive has been found about them. While most Za'lek prefer to take a theoretical approach before trying anything experimental, I am rather the opposite. It is much easier to figure out the theory after you have seen how things work."]]))
   n(_([["The experiment plan is fairly simple, I have set up an amplifier to create an energy focal point at the particle physics testing site. The idea is pretty straight-forward: you will just have to the nebula crystals to the focal point, and we can leave the rest to particle physics. I have also set up some recording drones that should capture anything and everything that happens out there."]]))
   vn.na(_([[You are about to ask about your safety, but seeing how excited Noona is getting, you decide to bet on her word that your ship will shield you from any potential particle physics mishaps.]]))
   vn.func( function () accepted = true end )
   vn.done( zpp.noona.transition )
   vn.run()

   -- Must be accepted beyond this point
   if not accepted then return end

   misn.accept()

   -- mission details
   misn.setTitle( _("Particle Physics") )
   misn.setReward( _("Science!") )
   misn.setDesc(fmt.f(_("Perform the particle physics experiment at {sys}."),
      {sys=mainsys}))

   mem.mrk = misn.markerAdd( system.cur() )

   misn.osdCreate( _("Particle Physics"), {
      _("Perform the experiment"),
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
   vn.na(_([[You land after the dizzying event. What was that about?]]))
   n(_([["You don't look so good. Did something happen?"]]))
   vn.na(_([[You don't know how to start explaining and give up on mentioning anything.]]))
   n(_([["The experiment looks like it collected a ton of data I'll need to process and analyze. I was sort of expecting a more violent reaction, but you're lucky that did not happen. You never know how things will go with particle physics."
She gives you a not too reassuring smile.]]))
   n(_([["I have to process some data real quick, but meet me up at the spaceport bar in a bit. I should have another task for you."
She starts to hum and skips off towards her laboratory space.]]))
   vn.sfxVictory()
   --vn.na( fmt.reward(reward) )
   vn.done( zpp.noona.transition )
   vn.run()

   --player.pay( reward )
   zpp.log(_("You helped Noona conduct her particle physics experiment. However, you saw something you couldn't really make out during the experiments."))
   misn.finish(true)
end

local pexp, pbeam, shader
function enter ()
   if mem.state~=1 then
      return
   end
   if system.cur() ~= mainsys then
      player.msg(_("#rMISSION FAILED: You were supposed to perform the experiment!"))
      misn.finish(false)
   end

   -- Spawn the drones
   -- TODO better location once testing center object is created
   local pkatar = planet.get("Katar"):pos()
   local pkatari = mainpnt:pos()
   local pos = (pkatar - pkatari) + pkatar
   -- Hostile drone
   pexp = pilot.add( "Za'lek Heavy Drone", "Za'lek", pos, _("Amplifier") )
   pexp:control(true)
   pexp:brake()
   pexp:setInvincible(true)
   pexp:setDir( math.pi/4 )
   hook.pilot( pexp, "board", "drone_board" )

   pbeam = pos + vec2.newP( 100, math.pi/4 )

   system.mrkAdd( pos, _("Experiment Site") )

   hook.timer( 5, "heartbeat" )
   hook.update( "update" )
   hook.renderbg( "renderbg" )

   shader = zpp.shader_focal()
end

function update( dt )
   shader:update( dt )
end

function renderbg ()
   local z = camera.getZoom()
   local x, y = gfx.screencoords( pbeam, true ):get()

   shader:render( x, y, 75 / z )
end

local fixed = false
function drone_board ()
   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(_([[You access the amplifiers control panel and jack in.]]))

   -- TODO maybe do another minigame?
   sokoban.vn{ levels={6,7}, header="Amplifier Control Panel"}
   vn.func( function ()
      if sokoban.completed() then
         mem.state = 2
         vn.jump("sokoban_done")
         fixed = true
         pexp:setActiveBoard(false)
         pexp:setHilight(false)
         return
      end
      vn.jump("sokoban_fail")
   end )

   vn.label([[sokoban_done]])
   vn.na(_([[You manage to adjust the settings and the amplifier turns on agian.]]))
   vn.done()

   vn.label("sokoban_fail")
   vn.na(_([[You failed to turn the amplifier on.]]))

   vn.run()

   player.unboard()
end

local function inrange ()
   return pbeam:dist( player.pilot():pos() ) < 75
end

local stage = 0
function heartbeat ()
   local delay = 3
   if stage==0 then
      pilot.comm(_("Noona"), _("Get close to the experiment site!"))
      stage = 1
   elseif stage==1 and pexp:pos():dist( player.pilot():pos() ) < 500 then
      pilot.comm(_("Noona"), _("Carefully push the crystals into the beam."))
      stage = 2
   elseif stage==2 and inrange() then
      pilot.comm(_("Noona"), _("Hmmm. It looks like the amplifier needs some adjustments. Could you try to fix it?"))
      -- Turn off power beam here
      pexp:setActiveBoard()
      pexp:setHilight()
      stage = 3
   elseif stage==3 and fixed then
      pilot.comm(_("Noona"), _("It looks everything is OK now."))
      -- Turn on beam power
      stage = 4
      delay = 6
   elseif stage==4 then
      pilot.comm(_("Noona"), _("Now push the crystals into the beam."))
      stage = 5
   elseif stage==5 and inrange() then
      -- CUTSCENE START
      player.cinematics( true )
      stage = 6
   elseif stage==6 then
      -- pulse
      stage = 7
   elseif stage==7 then
      -- pulse
      stage = 8
   elseif stage==8 then
      -- pulse
      stage = 9
      delay = 2
   elseif stage==9 then
      -- pulse
      stage = 10
      delay = 2
   elseif stage==10 then
      -- pulse
      stage = 11
      delay = 1
   elseif stage==11 then
      stage = 12
      delay = 10
   elseif stage==12 then
      stage = 13
   elseif stage==13 then
      -- CUTSCENE END
      player.cinematics( false )
      stage = 14
   elseif stage==14 then
      pilot.comm(_("Noona"), _("Are you alright? Communication cut off for a bit."))
      stage = 15
      delay = 7
   elseif stage==15 then
      pilot.comm(_("Noona"), _("Come back to Katar I."))
      misn.osdActive(2)
      mem.state = 2
      return
   end

   if stage < 5 then
      hook.timer( delay, "heartbeat" )
   end
end
