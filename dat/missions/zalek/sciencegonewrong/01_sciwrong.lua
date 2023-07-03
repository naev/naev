--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The one with the Visit">
 <unique />
 <priority>4</priority>
 <done>The one with the Shopping</done>
 <chance>100</chance>
 <location>Bar</location>
 <faction>Za'lek</faction>
 <cond>spob.cur() == require("common.sciencegonewrong").getCenterOperations()</cond>
</mission>
--]]
--[[
--
-- MISSION: The one with the Visit
-- DESCRIPTION: Dr. Geller asks you to disable a Soromid ship to retrieve some technology
-- that he needs for his prototype. Za'lek science can obviously not fall behind Soromid.
-- Obviously Soromid will not like this and will try to attack the player.
--
-- Difficulty: Easy to Medium?
--
-- Author: fart but based on Mission Ideas in wiki: wiki.naev.org/wiki/Mission_Ideas
--]]
local fmt = require "format"
local sciwrong = require "common.sciencegonewrong"
local vn = require "vn"
local vntk = require "vntk"
local lmisn = require "lmisn"

local target -- Non-persistent state

-- mission variables
mem.t_sys = {}
mem.t_pla = {}
--mem.t_pla[1], mem.t_sys[1] = spob.getS("Gastan")
mem.t_sys[2] = system.get("Shikima")
local reward = 1e6
local shpnm = _("Tokera")

function create ()
   -- Have to be at center of operations.
   mem.t_pla[1], mem.t_sys[1] = sciwrong.getCenterOperations()
   if spob.cur() ~= mem.t_pla[1] then
      misn.finish(false)
   end

   -- Spaceport bar stuff
   misn.setNPC( _("Dr. Geller"),  sciwrong.geller.portrait, _("You see Dr. Geller waving you over. Apparently, he has another job for you.") )
end
function accept()
   local accepted = false
   vn.clear()
   vn.scene()
   local geller = vn.newCharacter( sciwrong.vn_geller() )

   -- Mission details:
   geller(_([["Ah, there you are! I've got a job for you. Specifically, some... acquisition... of technology from the Soromid, who haven't been very cooperative. Are you up for it?" It sounds like he wants you to do something that would get you in trouble with Soromid authorities. Do you accept the job?]]))
   vn.menu( {
      { _("Accept the job"), "accept" },
      { _("Decline to help"), "decline" },
   } )

   vn.label("decline")
   geller(_("But I really thought you were into science..."))
   vn.done()

   vn.label("accept")
   geller(fmt.f(_([["Excellent. From what I have been told, it looks like this." He gestures with his hands to no merit. "You will recognize it; it should be in a box that's kept separately from the remaining stuff and labeled "Top Secret". Oh, and you might need this." He hands you a handheld device. "The ship is called the {plt}."]]), {plt=shpnm}))
   vn.na(_([[So he wants you to steal something top secret from the Soromid. Quirky people, those Za'leks. With the coordinates, the signature of the target ship, and the handheld, which you hope helps you detect the box, you set off on your way.]]))
   vn.func( function () accepted = true end )
   vn.run()

   if not accepted then
      return
   end

   misn.accept()
   misn.osdCreate(_("The one with the Visit"), {
      fmt.f(_("Go to the {sys} system and find the {plt}"), {sys=mem.t_sys[2], plt=shpnm}),
      fmt.f(_("Board the {plt} and retrieve the secret technology"), {plt=shpnm}),
      fmt.f(_("Return to {pnt} in the {sys} system"), {pnt=mem.t_pla[1], sys=mem.t_sys[1]}),
   })
   misn.setDesc(_("You've been hired by Dr. Geller to retrieve technology he urgently needs to build his prototype."))
   misn.setTitle(_("The one with the Visit"))
   misn.setReward(_("The gratitude of science and a bit of compensation"))
   misn.osdActive(1)
   mem.misn_mark = misn.markerAdd( mem.t_sys[2], "high" )
   mem.targetalive = true
   hook.enter("sys_enter")
end


function sys_enter ()
   if system.cur() == mem.t_sys[2] and mem.targetalive then
      local dist = rnd.rnd() * system.cur():radius() *1/2
      local location = vec2.newP(dist, rnd.angle())
      target = pilot.add( "Soromid Odium", "Soromid", location, shpnm )
      target:control()
      target:setFaction("Soromid")
      target:memory().aggressive = true
      target:setHilight(true)
      target:setVisplayer(true)
      mem.hidle = hook.pilot(target, "idle", "targetIdle")
      mem.hexp = hook.pilot(target, "exploded", "targetExploded")
      mem.hboard = hook.pilot(target, "board", "targetBoard")
      targetIdle()
   end
end

function targetIdle()
   if not pilot.exists(target) then -- Tear down now-useless hooks.
      hook.rm(mem.hidle)
      return
   end
   if target:hostile() then
      hook.rm(mem.hidle)
      target:control(false)
      return
   end
   local location = target:pos()
   local dist = 750
   local newlocation = vec2.newP(dist, rnd.angle())
   target:taskClear()
   target:moveto(location + newlocation, false, false)
   hook.timer(5.0, "targetIdle")
end

function targetExploded()
   hook.timer( 2.0, "targetDeath" )
end

function targetDeath()
   local sor = faction.get("Soromid")
   hook.rm(mem.hexp)
   if mem.boarded then
      sor:modPlayer(-5)
      return
   end
   vntk.msg(_([[What have you done?]]),_([[The ship explodes before your eyes and you realize that you will never be able to get the secret tech now.]]))
   lmisn.fail(_("you destroyed the secret tech!"))
end

function targetBoard()
   player.unboard()
   vntk.msg(_([[In the ship]]), {
      _([[You make your way through the living ship after taking care of its crew. You note the feeling that the ship is personally angry at you which, given the rumours that Soromid ships are alive, gives you the creeps. In any case, you begin to search through the ship and the handheld in your pocket starts beeping.]]),
      _([[You manage to locate a box on a table in the crews' chambers. Apparently nobody expected anyone to be foolish enough to try to do what you are doing. You grab the box and head back to your ship. You should make sure to avoid any Soromid patrols on the way back. You don't think they will be too happy with you if they manage to scan your ship.]])
   })
   target:setHilight(false)
   target:setVisplayer(false)
   local c = commodity.new(N_("Secret Technology"), N_("A mysterious box of stolen Soromid technology."))
   c:illegalto( "Soromid" )
   mem.cargoID = misn.cargoAdd(c, 0)
   misn.osdActive(3)
   misn.markerMove( mem.misn_mark, mem.t_pla[1], "high" )
   mem.hland = hook.land("land")
   mem.boarded = true
   hook.rm(mem.hboard)
end

function land()
   if spob.cur() == mem.t_pla[1] then
      vn.clear()
      vn.scene()
      local geller = vn.newCharacter( sciwrong.vn_geller() )
      geller(_([["How'd it go?" asks Dr. Geller. You show him the box. "Ah, marvelous! Do you know what this is? This is a quantum sharpener. It's like a quantum eraser, but instead of erasing, it sharpens. This is exactly what I needed. With this, I should be able to finish my prototype." He tosses you a credit chip before walking off, smiling.]]))
      vn.sfxVictory()
      vn.func( function ()
         player.pay(reward)
      end )
      vn.na(fmt.reward(reward))
      vn.run()

      hook.rm(mem.hland)
      sciwrong.addLog( _([[You stole something called a "quantum sharpener" from a Soromid ship for Dr. Geller.]]) )
      misn.finish(true)
   end
end

function abort ()
   if target then
      target:setHilight(false)
      target:setVisplayer(false)
   end
   misn.finish(false)
end
