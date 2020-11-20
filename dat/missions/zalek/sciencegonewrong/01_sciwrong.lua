--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The one with the Visit">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <done>The one with the Shopping</done>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Gastan</planet>
 </avail>
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

require "missions/zalek/common.lua"


-- mission variables
t_sys = {}
t_pla = {}
t_pla[1] = "Gastan"
t_sys[1] = "Seiben"
t_sys[2] = "Shikima"
reward = 1000000
shpnm = _("Tokera")
-- Mission details
title = {}
text = {}
osd_msg = {}
misn_title = _("The one with the Visit")
misn_reward = _("The gratitude of science and a bit of compensation")
misn_desc = _("You've been hired by Dr. Geller to retrieve technology he urgently needs to build his prototype.")
bar_desc = _("You see Dr. Geller waving you over. Apparently he has another job for you.")


title[1] = _([[In the bar]])
text[1]  = _([["Ah, there you are! I've got a job for you. Specifically, some... acquisition... of technology from the Soromid, who haven't been very cooperative. Are you up for it?" It sounds like he wants you to do something that would get you in trouble with Soromid authorities. Do you accept the job?]])
text[2] = _([["Excellent. From what I have been told it looks like this." He gestures with his hands to no merit. "You will recognize it; it should be in a box that's kept separately from the remaining stuff and labeled "Top Secret". Oh, and you might need this." He hands you a handheld device. "The ship is called the %s."
    So he wants you to steal something top secret from the Soromid. Quirky people, those Za'leks. With the coordinates, the signature of the target ship and the handheld, which you hope helps you detect the box, you set off on your way.]])
-- msgs by Soromid forces
title[3] = _([[In the ship]])
text[4] = _([[You make your way through the living ship after taking care of its crew. You note the feeling that the ship is personally angry at you which, given the rumors that Soromid ships are alive, gives you the creeps. In any case, you begin to search through the ship and the handheld in your pocket starts beeping.
    You manage to locate a box on a table in the crew's chambers. Apparently nobody expected anyone to be foolish enough to try to do what you are doing. You grab the box and head back to your ship.]])

text[5] = _([["How'd it go?" asks Dr. Geller. You show him the box. "Ah, marvelous! Do you know what this is? This is a quantum sharpener. It's like a quantum eraser, but it does not erase but sharpen. This is exactly what I needed. I think with this I should be able to finish my prototype." He tosses you a credit chip before walking off, smiling.]])
-- if the player kills the ship before getting the tech
title[4] = _([[What have you done?]])
text[6] = _([[The ship explodes before your eyes and you realize that you will never be able to get the secret tech now.]])
OSDtitle = _("The one with the Visit")
osd_msg = {}
osd_msg[1] = _("Go to the %s system and find the %s")
osd_msg[2] = _("Board the %s and retrieve the secret technology")
osd_msg[3] = _("Return to %s in the %s system")

-- refuestext 
refusetitle = _("No Science Today")
refusetext = _("But I really thought you were into science...")

log_text = _([[You stole something called a "quantum sharpener" from a Soromid ship for Dr. Geller.]])


function create ()
   -- Spaceport bar stuff
   misn.setNPC( _("Dr. Geller"),  "zalek/unique/geller")
   misn.setDesc( bar_desc )
end
function accept()
   -- Mission details:
   if not tk.yesno( title[1], text[1] ) then
      tk.msg(refusetitle, refusetext)
      misn.finish()
   end
   tk.msg( title[1], text[2]:format(shpnm) )
   misn.accept()
   misn.osdCreate(misn_title, {osd_msg[1]:format(t_sys[2],shpnm),osd_msg[2]:format(shpnm),osd_msg[3]:format(t_pla[1],t_sys[1])})
   misn.setDesc(misn_desc)
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn.osdActive(1)
   misn_mark = misn.markerAdd( system.get(t_sys[2]), "high" )
   targetalive = true
   hook.enter("sys_enter")
end


function sys_enter ()
   if system.cur() == system.get(t_sys[2]) and targetalive then
      dist = rnd.rnd() * system.cur():radius() *1/2
      angle = rnd.rnd() * 2 * math.pi
      location = vec2.new(dist * math.cos(angle), dist * math.sin(angle)) -- Randomly spawn the Ship in the system
      target = pilot.add("Soromid Odium", nil, location)[1]
      target:control()
      target:rename(shpnm)
      target:setFaction("Soromid")
      target:memory().aggressive = true
      target:setHilight(true)
      target:setVisplayer(true)
      hidle = hook.pilot(target, "idle", "targetIdle")
      hexp = hook.pilot(target, "exploded", "targetExploded")
      hboard = hook.pilot(target, "board", "targetBoard")
      targetIdle()
   end
end

function targetIdle()
   if not pilot.exists(target) then -- Tear down now-useless hooks.
      hook.rm(hidle)
      return
   end
   if target:hostile() then
      hook.rm(hidle)
      target:control(false)
      return
   end
   location = target:pos()
   dist = 750
   angle = rnd.rnd() * 2 * math.pi
   newlocation = vec2.new(dist * math.cos(angle), dist * math.sin(angle)) -- New location is 750px away in a random direction
   target:taskClear()
   target:goto(location + newlocation, false, false)
   hook.timer(5000, "targetIdle")
end

function targetExploded()
   hook.timer( 2000, "targetDeath" )
end

function targetDeath()
   sor= faction.get("Soromid")
   hook.rm(hexp)
   if boarded then
      sor.modPlayer(-5)
      return
   end
   tk.msg(title[4],text[6])
   sor.modPlayer(-5)
   misn.finish(false)
end

function targetBoard()
   player.unboard()
   tk.msg(title[3], text[4])
   target:setHilight(false)
   target:setVisplayer(false)
   cargoID = misn.cargoAdd("Secret Technology",0)
   misn.osdActive(3)
   misn.markerRm(misn_mark)
   misn_mark = misn.markerAdd( system.get(t_sys[1]), "high" )
   hland = hook.land("land")
   boarded = true
   hook.rm(hboard)
end

function land()
   if planet.cur() == planet.get(t_pla[1]) then
      tk.msg(title[1], text[5])
      hook.rm(hland)
      misn.markerRm(misn_mark)
      player.pay(reward)
      zlk_addSciWrongLog( log_text )
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
