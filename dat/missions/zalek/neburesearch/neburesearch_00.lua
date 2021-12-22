--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Novice Nebula Research">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Jorla</planet>
 </avail>
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

-- luacheck: globals beginFirstScan beginSecondScan drainShields endSecondScan jumpin land noticeProblems startProblems stopProblems takeoff (Hook functions passed by name)

-- Mission Constants
local t_sys = { system.get("Doeston"), system.get("Iris") }
local homeworld, homeworld_sys = planet.getS("Jorla")
local credits = 300e3


function create()
   -- mission variables
   mem.misn_stage = 0

   -- Spaceport bar stuff
   misn.setNPC(_("A young scientist"),  "zalek/unique/student.webp", _("You see a young scientist talking with some pilots, apparently without success.") )
end

function accept()
   local bar_title = _("Science Needs You")

   -- Check for cargo space
   if player.pilot():cargoFree() <  5 then
      tk.msg( "", _([["Sorry, I need a ship with more cargo space than you have."]]) )
      return
   end

   if not tk.yesno(bar_title, fmt.f(_([["Hello there! You are a pilot, right? For my project I require a ship that can go to the Nebula. Certainly you must be interested in the proposal of researching the phenomenon that cut us off from mankind's patrimony.
   "As this is the point where any other pilots I asked backed out, I should start by mentioning that due to some unfortunate circumstances the payment for this mission will be only {credits}. But rest assured, you will be mentioned in the acknowledgment section of my next paper!"]]), {credits=fmt.credits(50e3)})) then
      if not tk.yesno(bar_title, _([["Hold up! Look, the problem is that my grant was not permitted to the extent that I asked for. Those assholes cut my funds because they just don't understand the relevance of my research. Just because I'm still a student they completely underestimate my abilities!
   "Now I've spent all my credits on this sensor suit without the ability to use it. You must know how this feels. I mean, your ship obviously could use some work. So why don't you just help me out here?"]]) ) then
         return
      else
         tk.msg( bar_title,
            _([["Great! I'll start loading the sensors into your ship right away. We should be ready to take off soon."]])
            )
      end
   else
      tk.msg( bar_title,
         _([["So it is not a problem at all? I'm still a student and spent all funds I got on the sensor suit. Thank you for helping me out here! I'll start to load the sensors into your ship right away. We should be ready to take off soon."]])
         )
   end

   -- Add cargo
   local c = commodity.new( N_("Nebula Sensor Suit"), N_("A heavy suit with lots of fancy looking sensors.") )
   mem.cargo = misn.cargoAdd(c, 5)

   -- Set up mission information
   misn.setTitle( _("Novice Nebula Research") )
   misn.setReward( fmt.f(
      _("{credits} and the gratitude of a student"), {credits=fmt.credits(50e3)} ) )
   misn.setDesc( _("You have been asked by a Za'lek student to fly into the Nebula for some kind of research.") )
   mem.misn_marker = misn.markerAdd(t_sys[1], "low")

   -- Add mission
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
   mem.landed = planet.cur()
   if mem.misn_stage == 3 and mem.landed == homeworld then
      tk.msg( "", fmt.f(_([[The student has already removed all the cables and sensors inside your ship during the flight back to {pnt}. Everything is packed into a couple of crates by the time you land.
   "Once again, thank you for your help. I still have to analyze the data but it looks promising so far. With these results no one is going to question my theories anymore! Also, I decided to increase your reward to compensate for the trouble I caused."
   He gives you a credit chip worth {credits} and heads off. The money is nice, but not worth as much as the insight that working for the Za'lek will be dangerous and tiresome.]]),
         {pnt=homeworld, credits=fmt.credits(credits)} ) )
      player.pay(credits)
      zlk.addNebuResearchLog(
         _("You helped a Za'lek student to collect sensor data in the Nebula.") )
      misn.finish(true)
   end
end

function takeoff()
   local title = _("A Mess On Your Ship")
   tk.msg(title, fmt.f(_([[As you enter your ship you notice dozens of cables of various colours stretched across your ship's corridors. It is a complete mess. You follow the direction most of the cables seem to lead to and find the culprit.
   "Oh, hello again, Captain! I'm done with my work here, so we can take off whenever you're ready. I have to calibrate the sensors during the flight so there is no need to rush. Our first destination is {sys}." You try to maintain composure as you ask him what he has done to your ship. "Oh, I just installed the sensors. It should have no unwanted side effects on your ship.
   "A mess, you say? Haven't you noticed the colour coding? Don't worry, I know exactly what I'm doing!" His last words are supposed to be reassuring but instead you start to think that accepting this mission was not the best idea.]]), {sys=t_sys[1]}))
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
   tk.msg("", fmt.f(_([[The student enters your cockpit as you arrive in the {sys1} system. "Greetings, Captain! I realize I forgot to introduce myself. My name is Robert Hofer, student of Professor Voges himself! I'm sure you must have heard of him?" You tell him that the name doesn't sound familiar to you. "How can that be? Well, you would understand if you were a Za'lek.
   "Anyway, I will now start with the measurements. The density of the nebula is lower in this sector, so it's not particularly volatile. For the real measurements we have to enter {sys2}. I will let you know when we're done here."]]), {sys1=t_sys[1], sys2=t_sys[2]}))
   mem.shook = hook.timer(30.0, "startProblems")
end

function startProblems()
   -- Cancel autonav.
   player.cinematics(true)
   player.cinematics(false)
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
   tk.msg("", _([[Suddenly you lose control of your ship. Apparently most core systems were shut down. Something drains your ship's energy and there are black outs in several parts of your ship.
   You realize that your shields are down as well. In an environment like this... That's it, you're going to die here! You knew accepting this mission was a mistake from the very first moment.]]))
   hook.timer(10.0, "stopProblems")
end

function stopProblems()
   local ps = player.pilot()
   ps:control(false)
   ps:setEnergy(100)
   if zlk.hasZalekShip() then
     tk.msg("", fmt.f(_([[You breathe a sigh of relief. It seems you're still alive. You try not to glare at Robert Hofer, but apparently aren't particularly successful considering his response. "Sorry for causing trouble. I seem to have underestimated the polarity feedback loop granularity. If it weren't for your Za'lek ship the problem would have been much worse!"
    "I should investigate the damage it caused to the armour once we land. But first we must go to the {sys} system. Don't worry, the blackout will not occur again!"]]), {sys=t_sys[2]}))
   else
     tk.msg("", fmt.f(_([[You breathe a sigh of relief. It seems you're still alive. You try not to glare at Robert Hofer, but apparently aren't particularly successful considering his response. "Sorry for causing trouble. I'm not quite familiar with the electronics of this ship type. You really should fly a Za'lek ship instead. Those are so much better!"
   "I should investigate the damage it caused to the armour once we land. But first we must go to the {sys} system. Don't worry, the blackout will not occur again!"]]), {sys=t_sys[2]}))
   end
   mem.misn_stage = 1
   misn.markerMove(mem.misn_marker, t_sys[2])
   misn.osdActive(2)
   hook.rm(mem.phook)
   hook.enter("jumpin")
end

function beginSecondScan()
   tk.msg( "", fmt.f(
      _("You arrive in the {sys} system and Robert Hofer tells you that he will let you know when his scan is complete. This had better not cause another blackout..."),
	{sys=t_sys[2]} ) )
   hook.timer(30.0, "endSecondScan")
end

function endSecondScan()
   tk.msg( "", fmt.f(
      _([["OK, my measurements are complete! Let's go back to {pnt}."]]),
	{pnt=homeworld} ) )
   mem.misn_stage = 3
   misn.markerMove(mem.misn_marker, homeworld)
   misn.osdActive(3)
end
