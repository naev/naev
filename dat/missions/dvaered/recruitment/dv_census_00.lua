--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Census 0">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>30</chance>
  <location>Bar</location>
  <faction>Dvaered</faction>
 </avail>
 <notes>
  <tier>1</tier>
  <campaign>Dvaered Recruitment</campaign>
 </notes>
</mission>
--]]
--[[
-- Dvaered Census 0
-- This is the first mission of the Dvaered Recruitment arc
-- (facultative missions that make the player close to the Dvaered
-- and end up allowing them to purchase heavy weapons and ships licenses).
-- The player has to get in range of a given amount of Dvaered Ships in a given system.

   Stages :
   0) Way to system and in-system traffic
   1) Way to any Dvaered Spob to get paid
--]]

local cens   = require "common.census"
local tutnel = require "common.tut_nelly"
local vn     = require 'vn'
local vntk   = require 'vntk'
local fmt    = require "format"
local dv     = require "common.dvaered"
local pir    = require "common.pirate"

-- luacheck: globals enter land testInRange (Hook functions passed by name)
-- luacheck: globals approach_nelly

local detected

function create ()
   mem.sys = system.get("Dvaer")
   if not misn.claim(mem.sys) then misn.finish(false) end

   mem.nbships, mem.credits = cens.calculateNb( mem.sys, {faction.get("Dvaered")} )

   if player.misnDone( 'Helping Nelly Out 1' ) then
      misn.setNPC( tutnel.nelly.name, tutnel.nelly.portrait, _("Nelly has noticed you. Wait? Is she trying to have a sausage infuse in her Vodka?") )
   else
      misn.setNPC( _("Pilot"), tutnel.nelly.portrait, _("Wait? Is this woman trying to have a sausage infuse in her Vodka?") )
   end
end

function accept()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )
   local doaccept = false

   vn.transition( tutnel.nelly.transition )
   if player.misnDone( 'Helping Nelly Out 1' ) then
      nel(fmt.f(_([["Hi, {player}! How do you do? How long have you been here in Dvaered space? Not so long, eh?"]]), {player=player.name()}))
   else
      nel(_([["Hi, I'm Nelly! Oh! I guess you are new to Dvaered space, right?"]]))
   end
   nel(_([[Nelly puts her hands on the table and lifts herself up.
"Do you want a good tip from an old skipper who has been sailing among the stars for cycles?"]]))
   nel(_([[A smile appears on her face and she sits down again.
"Okay, just kidding. I'm not an old skipper. However, I still have found a good way to make money by doing missions for the Dvaered.
"It's not dangerous, the pay is decent, and it does not put you in trouble with any other faction. Do you want me to explain?"]]))
   vn.menu{
      {_("Yes"), "details"},
      {_("No"), "decline"},
   }

   vn.label("decline")
   nel(_([["As you wish."]]))
   vn.func( function () doaccept = false end ) -- Just in case
   vn.done( tutnel.nelly.transition )

   vn.label("details")
   nel(_([["I will tell you what I have understood.
"Most of the Dvaered warships you see out there do belong to private armies, that obey to generals, called the 'Warlords'. Those Warlords have many privileges, but they also have some obligations. One of those obligations is to patrol routes around the worlds they own.
But as the Dvaered are a bit… primitive, their administration is not able to control that they actually protect the trade lanes. That is why they need private pilots, like us, to count Dvaered ships."]]))
   nel(_([["Here is how it works: You simply have to browse a given stellar system, and to get in sensor range of a given number of Dvaered ships. The data is automatically acquired and processed.
The Dvaered authorities can then deduce from the transponder IDs what ship belongs to which warlord.
Once you have seen enough ships, you can land on any Dvaered-controlled planet to transmit your data and collect your pay."]]))
   nel(fmt.f(_([["The last one I chose happened in {system}, you have to get in range of {nb} ships, and the pay is {credits}. Those missions appear at the mission computer to pilots who have required so to the authorities. But, you know what? I can transfer mine to you. I needed to abort it anyways because this freaking sausage has not finished to infuse yet!"]]), {system=mem.sys, nb=mem.nbships, credits=fmt.credits( mem.credits )}))
   vn.menu{
      {_("Accept the mission"), "start"},
      {_("Refuse the mission"), "decline"},
   }

   vn.label("start")
   vn.func( function () doaccept = true end )
   nel(_([["Good luck with that mission, then!"]]))
   vn.done( tutnel.nelly.transition )

   vn.run()

   -- Check to see if truly accepted
   if not doaccept then return end

   misn.accept()
   mem.misn_state = 0
   mem.misn_marker = misn.markerAdd( mem.sys )

   -- Create NPC for casual chatting
   misn.npcAdd( "approach_nelly", tutnel.nelly.name, tutnel.nelly.portrait, _("Nelly is still there.") )

   -- Mission details
   misn.setTitle(fmt.f(_("Monitoring of Warlords activity in {sys}"), {sys=mem.sys}))
   misn.setReward( fmt.credits( mem.credits ) )
   misn.setDesc( fmt.f(_("Dvaered High Command requires a pilot to go to {sys} and detect {nb} Dvaered ships"), {sys=mem.sys, nb=mem.nbships}))
   cens.osd( _("Dvaered Census"), mem.sys, mem.nbships, 0, "Dvaered", "Dvaered" )

   hook.enter("enter")
   hook.land("land")
end

function enter()
   -- Enters in the mission's system
   if mem.misn_state == 0 then
      if system.cur() == mem.sys then
         detected = {}
         testInRange()
      end
   end
   -- Note: there is no test on the player leaving the system before having scanned enough ships.
   -- This behaviour is totally allowed.
end

function land()
   --Pay the player
   if spob.cur():faction() == faction.get( "Dvaered" ) and mem.misn_state == 1 then
      vntk.msg( _("Reward"), _("You land and transmit a datapad to the local Dvaered liaison officer. They unlock for you the Dvaered Census missions, that you can now find at the mission computer under the label DC.") )

      player.pay( mem.credits )
      dv.addStandardLog( _([[You accomplished a patrol census mission for the Dvaered. They seem disposed to entrust you with more missions of that kind in the future.]]) )
      faction.modPlayerSingle("Dvaered",1) -- Not so big reputation growth (Dvaered kill cap is high)
      pir.reputationNormalMission(3)
      misn.finish(true)
   end
end

-- Test pilots in range.
function testInRange()
   if system.cur() ~= mem.sys then
      return -- Wrong system: abort
   end
   detected = cens.testInRange( detected, {faction.get("Dvaered")} )
   -- Test if mission is complete
   if mem.nbships <= #detected then
      misn.osdActive(2)
      mem.misn_state = 1
      player.msg( _("You have acquired data on enough Dvaered ships") )
      return
   end
   cens.osd( _("Dvaered Census"), mem.sys, mem.nbships, #detected, "Dvaered", "Dvaered" )
   hook.timer(1, "testInRange") -- Recursivity 1 s
end

-- Facultative talk to Nelly
function approach_nelly()
   vn.clear()
   vn.scene()
   local nel = vn.newCharacter( tutnel.vn_nelly() )

   local dvaered = {_("How did you end up in Dvaered Space?"), "nelly_story"}
   local warlord = {_("Isn't it a bit odd, how the Dvaered society works, with those Warlords?"), "warlords"}
   local sausage = {_("Suggest Nelly to prick the sausage with a fork to make it infuse quicker"), "sausage"}
   local leave   = {_("Leave Nelly"), "leave"}

   vn.label("details")
   nel(_([["How do you do?"]]))
   vn.menu{ dvaered, warlord, sausage, leave }

   vn.label("nelly_story")
   nel(_([["Ah. Long story."
Nelly raises her eyes and lets her gaze wander on the filthy ceiling.
"I had just landed on Brooks, in Arcturus, you know? For some reason, there are often children playing soccer on the spaceport, there. While workers were unloading goods from my ship, I was wondering why there had never been an accident with those children. And suddenly, an accident occurred!
"A kid shot the ball in the wrong direction, and it crashed into the side of my poor Llama, breaking through the plating!"]]))
   vn.menu{
      {_("If a soccer ball can break through your ship, what will happen with a blaster?"), "blaster"},
      {_("Was it easy to repair?"), "repair"},
   }

   vn.label("blaster")
   nel(_([[Nelly looks at you, surprised.
"That's exactly what that cyber-grandpa told me. And then, he said the problem was that my ship's absorption was not high enough. He spoke about a coating that is only used by Dvaered pilots, the 'Impacto-Plastic Coating', that can make your ship's armor much more resilient.
"So, after the reparations were completed, I went to Dvaered space. But I didn't find anybody who could install this coating on my ship yet. And what is more, I suspect this will cost much more that I can afford. So I do missions for the Dvaered in order to make money and explore a bit."]]))
   vn.menu{ warlord, sausage, leave }

   vn.label("warlords")
   nel(_([["Of course it is. But you know, they are not as advanced administratively as we at the core Empire are.
The Dvaered don't have an administrative procedure to follow in every situation of life, as we do in the core Imperial systems. So instead they obey the strongest of them, the Warlords. They exist because the administration here is not advanced enough to prevent violence in the society.
It is actually sad, but I guess it is like that. They chose independence after all…"]]))
   vn.menu{ dvaered, sausage, leave }

   vn.label("sausage")
   nel(_([["What? But of course not! It is the worst mistake possible. Didn't you read any book on sausage infusion in your life? Everyone knows that you must never, NEVER prick the sausage with a fork. At any cost!"]]))
   vn.menu{ dvaered, warlord, leave }

   vn.label("leave")
   vn.done( tutnel.nelly.transition )

   vn.run()
end
