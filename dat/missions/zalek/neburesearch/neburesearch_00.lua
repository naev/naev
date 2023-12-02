--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Novice Nebula Research">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <cond>
   if system.get("Regas"):jumpDist() &gt; 3 then
      return false
   end
   return require("misn_test").reweight_active()</cond>
 <notes>
  <campaign>Nebula Research</campaign>
  <tier>2</tier>
 </notes>
</mission>
--]]
--[[
   Mission: Novice Nebula Research

   Description: A Za'lek student asks the player to help him with his research. The player has to visit Doeston and Iris.
   Some minor complications on the way have to be expected.

   Difficulty: Easy
]]--
local fmt = require "format"
local zlk = require "common.zalek"
local nebu_research = require "common.nebu_research"
local vn = require 'vn'

local student_portrait = nebu_research.student.portrait

-- Mission Constants
local t_sys = { system.get("Doeston"), system.get("Iris") }
local homeworld, homeworld_sys = spob.getS("Jorla")
local credits = nebu_research.rewards.credits00
local amount = 5
-- Mission states:
--  nil: mission not accepted yet
--   0: go to doeston
--   1: go to iris
--   2: scan the iris system
--   3: return to jorla
mem.misn_stage = nil

function create()
   -- Spaceport bar stuff
   misn.setNPC(_("A young scientist"),  student_portrait, _("You see a young scientist talking with some pilots, apparently without success.") )
end

function accept()
   local accepted = false
   vn.clear()
   vn.scene()
   local student = vn.newCharacter( nebu_research.vn_student() )
   student:rename(_("Student"))
   vn.transition("fade")

   -- Start mission
   vn.na(_("After being turned down by the pilot he was talking to, the scientist approaches you."))
   student(_([["Hello there! You are a pilot, right? For my project I require a ship that can go to the Nebula. Certainly you must be interested in the proposal of researching the phenomenon that cut us off from mankind's patrimony."]]))

   -- Check for cargo space
   if player.pilot():cargoFree() < amount then
      student(_([["Unfortunately it looks like your ship does not has enough free cargo space."
He leaves the bar. It appears he has given up finding a pilot, at least for now.]]))
      vn.done()
   end
   student(fmt.f(_([[He pauses for a moment.
"As this is the point where any other pilots I asked backed out, I should start by mentioning that due to some unfortunate circumstances the payment for this mission will be only {credits}."]]), {credits=fmt.credits(credits)}))
   student(_([["But rest assured, you will be mentioned in the acknowledgment section of my next paper!"]]))
   vn.menu( {
      { _("Accept the job"), "accept" },
      { _("Decline to help"), "decline" },
   } )
   vn.label( "decline" )
   student(_([["Hold up! Look, the problem is that my grant was not permitted to the extent that I asked for. Those assholes cut my funds because they just don't understand the relevance of my research. Just because I'm still a student they completely underestimate my abilities!"]]))
   student(_([["Now I've spent all my credits on this sensor suite without the ability to use it. You must know how this feels. I mean, your ship obviously could use some work. So why don't you just help me out here?"]]))
   vn.menu( {
      { _("Offer to help him"), "finally_accept" },
      { _("Decline to help"), "really_decline" },
   } )
   vn.label( "really_decline" )
   vn.na(_("You decline. Why would you accept such a dangerous job without a proper payment?"))
   vn.done()
   vn.label( "accept" )
   student(_([["So it is not a problem at all? I'm still a student and spent all funds I got on the sensor suite. Thank you for helping me out here! I'll start to load the sensors into your ship right away. We should be ready to take off soon."
With that said he hurries and leaves the bar.]]))
   vn.func( function () accepted = true end )
   vn.done()
   vn.label( "finally_accept" )
   student(_([["Great! I'll start loading the sensors into your ship right away. We should be ready to take off soon."
With that said he hurries and leaves the bar.]]))
   vn.func( function () accepted = true end )
   vn.done()
   vn.run()

   if not accepted then
      return
   end

   -- Add cargo
   local c = commodity.new( N_("Nebula Sensor Suite"), N_("A heavy suite with lots of fancy looking sensors.") )
   mem.cargo = misn.cargoAdd(c, amount)

   -- Set up mission information
   misn.setTitle( _("Novice Nebula Research") )
   misn.setReward( fmt.f( _("{credits} and the gratitude of a student"), {credits=fmt.credits(credits)} ) )
   misn.setDesc( _("You have been asked by a Za'lek student to fly into the Nebula for some kind of research.") )
   mem.misn_marker = misn.markerAdd( t_sys[1], "low" )

   -- Add mission
   mem.misn_stage = 0
   misn.accept()
   local osd_title = _("Novice Nebula Research")
   local osd_msg = {}
   osd_msg[1] = fmt.f( _("Fly to the {sys} system"), {sys=t_sys[1]} )
   osd_msg[2] = fmt.f( _("Fly to the {sys} system"), {sys=t_sys[2]} )
   osd_msg[3] = fmt.f( _("Return to {pnt} in the {sys} system"), {pnt=homeworld, sys=homeworld_sys} )
   misn.osdCreate(osd_title, osd_msg)

   mem.thook = hook.takeoff("takeoff")
   hook.land("land")
   hook.enter("jumpin")
end

function land()
   mem.landed = spob.cur()
   if mem.misn_stage == 3 and mem.landed == homeworld then
      vn.clear()
      vn.scene()
      local student = vn.newCharacter( nebu_research.vn_student() )
      student:rename(_("Student"))
      vn.transition("fade")
      vn.na(fmt.f(_("The student has already removed all the cables and sensors inside your ship during the flight back to {pnt}. Everything is packed into a couple of crates by the time you land."),{pnt=homeworld}))
      student(_([["Once again, thank you for your help. I still have to analyze the data but it looks promising so far. With these results no one is going to question my theories anymore! Also, I decided to increase your reward to compensate for the trouble I caused."]]))
      vn.na(fmt.f(_("He gives you a credit chip worth {credits} and heads off. The money is nice, but not worth as much as the insight that working for the Za'lek will be dangerous and tiresome."),{credits=fmt.credits(credits)}))
      vn.done()
      vn.run()
      misn.cargoRm(mem.cargo)
      player.pay(credits)
      misn.markerRm(mem.misn_marker)
      nebu_research.log(_("You helped a Za'lek student to collect sensor data in the Nebula."))
      misn.finish(true)
   end
end

function takeoff()
   vn.clear()
   vn.scene()
   local student = vn.newCharacter( nebu_research.vn_student() )
   student:rename(_("Student"))
   vn.transition("fade")
   vn.na(_("As you enter your ship you notice dozens of cables of various colors stretched across your ship's corridors. It is a complete mess. You follow the direction most of the cables seem to lead to and find the culprit."))
   student(fmt.f(_([["Oh, hello again, Captain! I'm done with my work here, so we can take off whenever you're ready. I have to calibrate the sensors during the flight, so there is no need to rush. Our first destination is {sys}."]]), {sys=t_sys[1]}))
   vn.na(_("You try to maintain composure as you ask him what he has done to your ship."))
   student(_([["Oh, I just installed the sensors. It should have no unwanted side effects on your ship."]]))
   student(_([["A mess, you say? Haven't you noticed the color coding? Don't worry, I know exactly what I'm doing!"
His last words are supposed to be reassuring but instead you start to think that accepting this mission was not the best idea.]]))
   vn.done()
   vn.run()
   hook.rm(mem.thook)
end

function jumpin()
   mem.sys = system.cur()
   if mem.misn_stage == 0 and mem.sys == t_sys[1] then
      hook.timer(5.0, "beginFirstScan")
   elseif mem.misn_stage == 1 and mem.sys == t_sys[2] then
      mem.misn_stage = 2
      hook.timer(5.0, "beginSecondScan")
   end
end

function beginFirstScan()
   vn.clear()
   vn.scene()
   local student = vn.newCharacter( nebu_research.vn_student() )
   vn.transition("fade")
   student(fmt.f(_([[The student enters your cockpit as you arrive in the {sys} system.
"Greetings, Captain! I realize I forgot to introduce myself. My name is Robert Hofer, student of Professor Voges himself! I'm sure you must have heard of him?"]]), {sys=t_sys[1]}))
   student(_([[You tell him that the name doesn't sound familiar to you.
"How can that be? Well, you would understand if you were a Za'lek."]]))
   student(fmt.f(_([["Anyway, I will now start with the measurements. The density of the nebula is lower in this sector, so it's not particularly volatile. For the real measurements we have to enter {sys}. I will let you know when we're done here."]]), {sys=t_sys[2]}))
   vn.done()
   vn.run()
   mem.shook = hook.timer(30.0, "startProblems")
end

function startProblems()
   -- Cancel autonav.
   player.autonavAbort()
   local ps = player.pilot()
   ps:control()
   mem.phook = hook.timer(0.1, "drainShields")
   hook.timer(4.0, "noticeProblems")
end

function drainShields()
   local ps = player.pilot()
   local armour = ps:health()
   ps:setHealth(armour, 0)
   ps:setEnergy(0)
   mem.phook = hook.timer(0.1, "drainShields")
end

function noticeProblems()
   vn.clear()
   vn.scene()
   vn.transition("fade")
   vn.na(_("Suddenly you lose control of your ship. Apparently most core systems were shut down. Something drains your ship's energy and there are black outs in several parts of your ship."))
   vn.na(_("You realize that your shields are down as well. In an environment like this... That's it, you're going to die here! You knew accepting this mission was a mistake from the very first moment."))
   vn.done()
   vn.run()
   hook.timer(10.0, "stopProblems")
end

function stopProblems()
   local ps = player.pilot()
   ps:control(false)
   ps:setEnergy(100)
   vn.clear()
   vn.scene()
   local student = vn.newCharacter( nebu_research.vn_student() )
   vn.transition("fade")
   vn.na(_("You breathe a sigh of relief. It seems you're still alive. You try not to glare at Robert Hofer, but apparently aren't particularly successful, considering his response."))
   if zlk.hasZalekShip() then
      student(_([["Sorry for causing trouble. I seem to have underestimated the polarity feedback loop granularity. If it weren't for your Za'lek ship the problem would have been much worse!"]]))
   else
      student(_([["Sorry for causing trouble. I'm not quite familiar with the electronics of this ship type. You really should fly a Za'lek ship instead. Those are so much better!"]]))
   end
   student(fmt.f(_([["I should investigate the damage it caused to the armor once we land. But first we must go to the {sys} system. Don't worry, the blackout will not occur again!"]]), {sys=t_sys[2]}))
   vn.done()
   vn.run()
   mem.misn_stage = 1
   misn.markerMove(mem.misn_marker, t_sys[2])
   misn.osdActive(2)
   hook.rm(mem.phook)
   hook.enter("jumpin")
end

function beginSecondScan()
   vn.clear()
   vn.scene()
   local student = vn.newCharacter( nebu_research.vn_student() )
   vn.transition("fade")
   student(fmt.f(_("You arrive in the {sys} system and Robert Hofer tells you that he will let you know when his scan is complete. This had better not cause another blackout..."),
	{sys=t_sys[2]}))
   vn.done()
   vn.run()
   hook.timer(30.0, "endSecondScan")
end

function endSecondScan()
   vn.clear()
   vn.scene()
   local student = vn.newCharacter( nebu_research.vn_student() )
   vn.transition("fade")
   student(fmt.f(_([["OK, my measurements are complete! Let's go back to {pnt}."]]), {pnt=homeworld}))
   vn.done()
   vn.run()
   mem.misn_stage = 3
   misn.markerMove(mem.misn_marker, homeworld)
   misn.osdActive(3)
end
