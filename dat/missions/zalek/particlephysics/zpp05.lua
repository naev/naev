--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Za'lek Particle Physics 5">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <spob>Katar I</spob>
 <location>Bar</location>
 <done>Za'lek Particle Physics 4</done>
 <notes>
  <campaign>Za'lek Particle Physics</campaign>
 </notes>
</mission>
--]]
--[[
   Za'lek Particle Physics 05

   Mainly a cutscene where the player takes the materials and does an experiment which goes weird.
]]--
local vn = require "vn"
local fmt = require "format"
local lmisn = require "lmisn"
local zpp = require "common.zalek_physics"
local sokoban = require "minigames.sokoban"
local audio = require 'love.audio'
local love = require "love"


--local reward = zpp.rewards.zpp05 -- No reward
local mainpnt, mainsys = spob.getS("Katar I")

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
   n(_([["It's almost ready! Thanks to you, I finally have everything ready for the test. However, after reviewing the drone firmware, I think it would be best to perform this experiment manually. Sometimes, it's better not to rely on all the fancy technology and instead do things the old-fashioned way. Would you be willing to perform the experiment for me? Your ship should be sturdy enough; I don't think there's any possibility of harm."]]))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   n(_([["I see. You must be busy with other things."]]))
   vn.done( zpp.noona.transition )

   vn.label("accept")
   n(_([["Thanks! So what you brought back last time were some nebula crystals. Nothing is really known about them, but, sometimes, they are found in old wrecks or asteroids in the nebula. We know that these crystals seem to have some weird properties, but nothing conclusive has been found about them. While most Za'lek prefer to take a theoretical approach before trying anything experimental, I am rather the opposite. It is much easier to figure out the theory after you have seen how things work."]]))
   n(_([["The plan for the experiment is fairly simple. I have set up an amplifier to create an energy focal point at the particle physics testing site. The idea is pretty straightforward: you will just have to fly the nebula crystals to the focal point, and we can leave the rest to particle physics. I have also set up some recording drones that should capture anything and everything that happens out there."]]))
   vn.na(_([[You are about to ask about your safety, but seeing how excited Noona is getting, you decide to rely on her word that your ship will shield you from any potential particle physics mishaps.]]))
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
   if mem.state==1 or spob.cur() ~= mainpnt then
      return
   end

   vn.clear()
   vn.scene()
   local n = vn.newCharacter( zpp.vn_noona() )
   vn.transition( zpp.noona.transition )
   vn.na(_([[You land after the dizzying event. What was that about?]]))
   n(_([["You don't look so good. Did something happen?"]]))
   vn.na(_([[You don't know how to start explaining and give up on mentioning anything.]]))
   n(_([["The experiment looks like it collected a ton of data I'll need to process and analyse. I was sort of expecting a more violent reaction, but you're lucky that did not happen. You never know how things will go with particle physics."
She gives you a not too reassuring smile.]]))
   n(_([["I have to process some data real quick, but meet me up at the spaceport bar in a bit. I should have another task for you."
She starts to hum and skips off towards her laboratory space.]]))
   vn.sfxVictory()
   --vn.na( fmt.credits(reward) )
   vn.done( zpp.noona.transition )
   vn.run()

   faction.modPlayer("Za'lek", zpp.fctmod.zpp05)
   --player.pay( reward )
   zpp.log(_("You helped Noona conduct her particle physics experiment. However, you saw something you couldn't really make out during the experiments."))
   misn.finish(true)
end

local pexp, pbeam, shader, shader_nebu, cutscene
local heartbeat_single, heartbeat_long
function enter ()
   if mem.state~=1 then
      return
   end
   if system.cur() ~= mainsys then
      lmisn.fail(_("You were supposed to perform the experiment!"))
   end

   -- Spawn the drones
   -- TODO better location once testing center object is created
   local pkatar = spob.get("Katar"):pos()
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

   local n = 7
   for i=1,n do
      local a = math.pi*2/7*i
      local p = pbeam + vec2.newP( 200, math.pi*2/7*i )
      local pextra = pilot.add( "Za'lek Scout Drone", "Za'lek", p )
      pextra:control(true)
      pextra:brake()
      pextra:setInvincible(true)
      pextra:setInvisible(true)
      pextra:setDir( a+math.pi )
   end

   system.markerAdd( pos, _("Experiment Site") )

   hook.timer( 5, "heartbeat" )
   hook.update( "update" )
   hook.renderbg( "renderbg" )

   shader = zpp.shader_focal()
   shader_nebu = zpp.shader_nebula()
   cutscene = false

   heartbeat_single = audio.newSource( 'snd/sounds/heartbeat_single.ogg' )
   heartbeat_long   = audio.newSource( 'snd/sounds/heartbeat_long.ogg' )
end

function update( dt )
   shader:update( dt )
   if cutscene then
      shader_nebu:update( dt )
   end
end

local fixed = true
function renderbg ()
   if fixed then
      local z = camera.getZoom()
      local x, y = gfx.screencoords( pbeam, true ):get()
      shader:render( x, y, 75 / z )
   end
   if cutscene then
      shader_nebu:render()
   end
end

function drone_board ()
   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(_([[You access the amplifier's control panel and jack in.]]))

   -- TODO maybe do another minigame?
   sokoban.vn{ levels={6,7}, header=_("Amplifier Control Panel") }
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
   vn.na(_([[You manage to adjust the settings and the amplifier turns on again.]]))
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
      fixed = false
      pexp:setActiveBoard()
      pexp:setHilight()
      player.pilot():setTarget( pexp )
      stage = 3
   elseif stage==3 and fixed then
      pilot.comm(_("Noona"), _("It looks like everything is OK now."))
      stage = 4
      delay = 6
   elseif stage==4 then
      pilot.comm(_("Noona"), _("Now push the crystals into the beam."))
      stage = 5
   elseif stage==5 and inrange() then
      -- CUTSCENE START
      player.cinematics( true )
      player.allowLand( false )
      love.origin()
      music.stop()
      stage = 6
   elseif stage==6 then
      -- pulse
      cutscene = true
      shader_nebu:reset()
      heartbeat_single:play()
      stage = 7
   elseif stage==7 then
      -- pulse
      shader_nebu:reset()
      heartbeat_single:play()
      stage = 8
   elseif stage==8 then
      -- pulse
      shader_nebu:reset()
      heartbeat_single:play()
      stage = 9
      delay = 2.5
   elseif stage==9 then
      -- pulse
      shader_nebu:reset()
      heartbeat_single:play()
      stage = 10
      delay = 2
   elseif stage==10 then
      -- pulse
      shader_nebu:reset()
      heartbeat_single:play()
      stage = 11
      delay = 1.5
   elseif stage==11 then
      -- pulse
      shader_nebu:reset()
      heartbeat_single:play()
      stage = 12
      delay = 1.25
   elseif stage==12 then
      -- pulse
      shader_nebu:reset()
      heartbeat_single:play()
      stage = 13
      delay = 1
   elseif stage==13 then
      stage = 14
      shader_nebu:send("u_mode", 1)
      shader_nebu:reset()
      heartbeat_long:play()
      delay = 10
   elseif stage==14 then
      shader_nebu:send("u_mode", 2)
      shader_nebu:reset()
      heartbeat_long:stop()
      stage = 15
   elseif stage==15 then
      -- CUTSCENE END
      player.cinematics( false )
      player.allowLand( true )
      music.play("ambient1.ogg")
      cutscene = false
      mem.state = 2
      love.origin()
      stage = 16
   elseif stage==16 then
      pilot.comm(_("Noona"), _("Are you alright? Communication cut off for a bit."))
      stage = 17
      delay = 7
   elseif stage==17 then
      pilot.comm(_("Noona"), _("Come back to Katar I."))
      misn.osdActive(2)
      system.markerClear()
      misn.markerAdd( mainpnt )
      return
   end

   hook.timer( delay, "heartbeat" )
end
