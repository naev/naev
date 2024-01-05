--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 6">
 <unique />
 <priority>3</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Antlejos V</spob>
 <cond>require('common.antlejos').unidiffLevel() &gt;= 6</cond>
 <done>Terraforming Antlejos 5</done>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Have to destroy PUAAA supply ship
--]]
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local ant = require "common.antlejos"
local lmisn = require "lmisn"

local reward = ant.rewards.ant06

local retpnt, retsys = spob.getS("Antlejos V")
local mainsys = system.get("Knave")

function create ()
   if not misn.claim(mainsys) then misn.finish() end
   misn.setNPC( _("Verner"), ant.verner.portrait, ant.verner.description )
end

local firsttime = true
function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   if firsttime then
      vn.na(_("You find Verner relaxing at the bar. He calls you towards him the moment he sees you."))
      vn.na(_("While you walk towards him, you suddenly hear a commotion towards the entrance of the bar. You hear what sounds like an anti-terraforming slogan when Verner suddenly runs past you towards disturbance. You crouch as you hear a shot being fired and can see Verner grabbing an individual before flinging them across the bar where they crash into the wall and fall down to the ground."))
      vn.na(_("Quickly a security team runs over and apprehends the guy, while Verner walks over to you barely breaking a sweat."))
      v(fmt.f(_([["It seems like the PUAAA is getting braver these days. We heard of a supply ship in {sys} providing them with more resources to continue their senseless violence, it's not the first time we have something like that."
He points to the unconscious individual being carried away.]]), {sys=mainsys}))
      v(fmt.f(_([["Would you be willing to go over to {sys} and take out the supply ship? I would be able to pay you {credits} for your trouble."]]),{sys=mainsys, credits=fmt.credits(reward)}))
      firsttime = false
   else
      v(_([["Have you changed your mind about dealing with the PUAAA supply ship?"]]))
   end
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(_([["Great! Just take out the supply ship with any means necessary, and we should be all good. I've got to go take care of this now."
He points towards a glancing burn mark on his abdomen. It looks like the shot fired during the commotion grazed him, but it doesn't look life-threatening. He chugs down his drink, and gets up before heading off.
"Take care!"]]))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   misn.accept()
   misn.setTitle( _("Eliminate the PUAAA Supply Ship") )
   misn.setDesc(fmt.f(_("Eliminate the PUAAA supply ship at {sys}."),{sys=mainsys}))
   misn.setReward(reward)
   misn.osdCreate(_("Eliminate the PUAAA Supply Ship"), {
      fmt.f(_("Go to {sys}"),{sys=mainsys}),
      _("Destroy the PUAAA supply ship"),
      fmt.f(_("Return to {pnt} ({sys} system)"),{pnt=retpnt, sys=retsys}),
   })
   mem.mrk = misn.markerAdd( mainsys )
   mem.state = 0

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==2 and spob.cur() == retpnt then
      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      vn.na(_("You land and go meet up with Verner, who is overseeing the terraforming."))
      v(_([["You got rid of the pesky supply ship? Great to hear that. Hopefully with fewer supplies the PUAAA will less active, and we'll be able to progress more. It's almost starting to look like a place I can call home."
He looks over the terraforming site, and you can see a mix of happiness and sorrow on his face.]]))
      v(_([[Without turning to you, he continues.
"It's going to be great. Thanks for your hard work."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      player.pay( reward )
      ant.log(fmt.f(_("You eliminated a PUAAA supply ship bringing more security to {pnt}."),{pnt=retpnt}))
      misn.finish(true)
   end
end

local supplyship
function enter ()
   if mem.state==2 then
      -- Nothing to do here
      return
   end
   -- Wrong system
   if mem.state==1 and system.cur() ~= mainsys then
      lmisn.fail(fmt.f(_("You were not supposed to leave {sys}!"),{sys=mainsys}))
      return
   end
   if system.cur() ~= mainsys then
      return
   end

   pilot.clear()
   pilot.toggleSpawn(false)

   mem.state = 1
   misn.osdActive(2)

   -- Initialize ship stuff
   local puaaa = ant.puaaa()
   local pos = vec2.new( -4500, 4500 )

   local protestors = {}
   supplyship = pilot.add( "Mule", puaaa, pos, _("PUAAA Supply Ship"), {ai="baddiepos"} )
   supplyship:setVisplayer(true)
   supplyship:setHilight(true)
   hook.pilot( supplyship, "death", "supplydeath" )
   hook.pilot( supplyship, "board", "supplyboard" )
   table.insert( protestors, supplyship )

   for k,s in ipairs{ "Lancelot", "Shark", "Shark" } do
      local p = pilot.add( s, puaaa, pos+vec2.newP( 100+rnd.rnd(100), rnd.angle() ), _("PUAAA Escort"), {ai="baddiepos"} )
      p:setLeader( supplyship )
      table.insert( protestors, p )
   end

   for k,p in ipairs(protestors) do
      p:setHostile(true)
   end
end

function supplydeath ()
   if mem.state == 2 then return end
   player.msg("#g".._("You eliminated the target!").."#0")
   mem.state = 2
   misn.osdActive(3)
   misn.markerMove( mem.mrk, retpnt )
end

function supplyboard ()
   vntk.msg(_("No more supplies"), _([[You board the ship and blast out the entire control panel rendering the ship unrepairable and not operational. That should satisfy Verner.]]))
   supplyship:disable() -- Permanently disable
   mem.state = 2
   misn.osdActive(3)
   misn.markerMove( mem.mrk, retpnt )
   player.unboard()
end
