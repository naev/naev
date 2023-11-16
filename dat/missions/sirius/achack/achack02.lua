--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Harja's Vengeance">
 <unique />
 <priority>3</priority>
 <cond>
   if spob.get("Violin Monastery"):system():jumpDist() &gt;= 4 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <done>Sirian Bounty</done>
 <chance>100</chance>
 <location>Bar</location>
 <faction>Sirius</faction>
 <notes>
  <campaign>Academy Hack</campaign>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[
-- This is the second mission in the Academy Hack minor campaign.
--]]
local lmisn = require "lmisn"
local fleet = require "fleet"
require "proximity"
local srs = require "common.sirius"
local fmt = require "format"
local pir = require "common.pirate"
local achack = require "common.achack"
local vn = require "vn"
local vntk = require "vntk"
local love_shaders = require "love_shaders"

local ambush, joanneship -- Non-persistent state
local ambushSet -- Forward-declared functions

-- Mission constants
local reward = 750e3
-- This is the route Joanne will take.
local route = {"Violin Monastery", "Fyruse Monastery", "Inios Monastery", "Tankard Cloister", "Sroolu"}

local stoptext = _("You dock with {pnt}, and the spacedock personnel immediately begin to refuel your ship. You spend a few hectoseconds going through checklists and routine maintenance operations. Then you get a ping on your comms from Joanne. She tells you that she has finished her business on this station, and that she's taking off again. You follow suit.")

function create()
   if not misn.claim ( {system.get("Druss"), system.get("Humdrum"), system.get("Lapis")} ) then
      misn.finish(false)
   end

   if var.peek("achack02repeat") then
      misn.setNPC(achack.joanne.name, achack.joanne.image, _("This is Joanne, the woman you were hired to assassinate."))
   else
      misn.setNPC(_("The Serra military officer"), achack.joanne.image, _("You know this woman. She's the military officer from before: the one you were hired to assassinate."))
   end
end

local function player_has_fast_ship()
   local stats = player.pilot():stats()
   mem.playershipspeed = stats.speed_max
   return (mem.playershipspeed > 200)
end

function accept()
   local accepted = false

   vn.clear()
   vn.scene()
   local joanne = vn.newCharacter( achack.vn_joanne() )
   vn.transition()

   if var.peek("achack02repeat") then
      joanne(fmt.f(_([["Hello again, {player}," Joanne greets you. "I'm afraid I still find myself under threat from mercenary assassins. Have you reconsidered my offer? Let me tell you again what I need."]]),
         {player=player.name()}))
   else
      joanne(fmt.f(_([[When you approach her, the officer greets you with a smile. "What a surprise that we should run into each other again," she says. "I'm afraid to say I don't remember your name. What was it again? Ah yes, {player}. I don't think I introduced myself last time. My name is Joanne. Well met. As you can see, I'm still doing quite well. No poison in my wine or snakes in my bed or anything."]]),
         {player=player.name()}))
      joanne(_([[Then her expression turns more serious. "Actually, about that. I think our friend Harja still has it in for me. You had the common sense to use your head, but I'm afraid not everyone is like that. I'm convinced Harja will try to hire more assassins to do what you didn't, so I'm in considerable danger."]]))
      vn.na(_([[You sympathize with Joanne, but you wonder aloud why she hasn't asked the local authorities for protection. Joanne gives you a somewhat uncomfortable look.]]))
      joanne(_([["Pride, I suppose. I'm a military officer. It wouldn't look too tactful if I asked for personal protection when the navy is already stretched thin out there, trying to protect our civilians from pirates and other criminals every day. Besides, my conflict with Harja is a personal matter. I feel I should resolve this with my own resources." She gives you a tired smile. "That being said, I wouldn't say no to a helping hand."]]))
   end
   vn.func( function ()
      var.push("achack02repeat", true)
   end )

   joanne(_([["See, this is the situation," she continues. "My duties as a Sirian administration officer require me to pay visits to several military outposts throughout Sirius space. It's a long trek every time, but that just comes with the job. Now, the trouble is that Harja's assassins might have many opportunities to ambush me along the way. I've had combat training, of course, just like every other Serra soldier, but I'm afraid I have little actual fighting experience, and I'm not sure I could survive an attack unassisted."]]))
   joanne(fmt.f(_([[[You, however, seem to have seen quite some action. If you were to escort me while I make my rounds, I would feel a lot more secure. I can reward you, of course. Let's see... Another {reward} seems a fair sum. What do you think, are you willing to do it?"]]),
      {reward=fmt.credits(reward)}))
   vn.menu{
      {_([[Accept the job.]]),"accept"},
      {_([[Decline.]]),"decline"},
   }

   vn.label("decline")
   vn.na(_([[You decide you can't handle the task of protecting Joanne right now. She's going to have to find for herself.]]))
   vn.done()

   vn.label("accept")
   joanne(_([["That's wonderful! Thank you so much. As I said, my job requires that I travel between Sirian military bases. You're going to need fuel to make the necessary jumps, of course, up to 7 jumps maximum. Now that you have agreed to be my personal escort, I can give you clearance to dock with those bases if you don't have it already so you can automatically refuel, but either way I won't be on station long, so you won't have time to disembark and do other things while you're there."]]))
   joanne(_([["Also, I think this goes without saying, but I need you to stick close to me so your ship must be fast enough. Also don't jump to any systems before I do, don't jump to the wrong systems, and don't land on any planets I don't land on. If we meet any aggressors along the way I will rely on you to help me fight them off. That's about it. I'll finish up a few things here, and then I'll head to my ship. I'll be in the air when you are."]]))
   vn.na(_([[Joanne leaves the bar. You will meet up with her in orbit.]]))
   vn.run()

   if not accepted then return end

   mem.stage = 1

   mem.destplanet, mem.destsys = spob.getS(route[mem.stage])
   mem.nextsys = system.cur() -- This variable holds the system the player is supposed to jump to NEXT. This is the current system when taking off.
   mem.origin = spob.cur() -- Determines where Joanne spawns from. Can be a planet or system.
   mem.joannejumped = true -- Determines if Joanne has jumped. Needs to be set when landed.
   mem.mark = misn.markerAdd( mem.destsys, "low" )
   mem.warnFuel = true

   misn.accept()
   misn.setDesc(_("Joanne needs you to escort her ship and fight off mercenaries sent to kill her."))
   misn.setReward(reward)
   misn.osdCreate(_("Harja's Vengeance"), {
      _("Follow Joanne's ship"),
      _("Defeat Joanne's attackers"),
   })

   hook.land("land")
   hook.enter("enter")
   hook.load("on_load")
end

local function laststop_vn ()
   vn.clear()
   vn.scene()
   local joanne = vn.newCharacter( achack.vn_joanne{shader=love_shaders.hologram()})
   vn.transition("electric")
   vn.na(_([[You go through the now familiar routine of waiting for Joanne. She soon hails you on the comms.]]))
   joanne(_([["That's it, {player}! This was the final stop. You've been a great help. This isn't a good place to wrap things up though. Tell you what, let's relocate to Sroolu and meet up in the spaceport bar there. I need to give you your payment, of course, but I also want to talk to you for a bit. See you planetside!"]]))
   vn.na(_([[The comm switches off. You prepare to take off and set a course for Sroolu.]]))
   vn.done("electric")
   vn.run()
end

-- Land hook.
function land()
   if spob.cur() == mem.destplanet and mem.joannelanded and mem.stage < 4 then
      mem.stage = mem.stage + 1
      mem.destplanet, mem.destsys = spob.getS(route[mem.stage])
      misn.markerMove( mem.mark, mem.destsys )
      mem.origin = spob.cur()
      player.refuel()
      vntk.msg(_("Another stop successfully reached"), fmt.f(stoptext, {pnt=spob.cur()}))
      mem.joannejumped = true -- She "jumped" into the current system by taking off.
      player.takeoff()

   elseif spob.cur() == mem.destplanet and mem.joannelanded and mem.stage == 4 then
      laststop_vn()
      mem.stage = mem.stage + 1
      mem.origin = spob.cur()
      mem.destplanet, mem.destsys = spob.getS(route[mem.stage])
      misn.markerMove( mem.mark, mem.destsys )
      mem.joannejumped = true -- She "jumped" into the current system by taking off.
      misn.osdCreate(_("Harja's Vengeance"), {_("Land on Sroolu to get your reward")})
      player.takeoff()

   elseif mem.stage < 4 then
      vntk.msg(_("You didn't follow Joanne!"), _("You landed on a planet Joanne didn't land on. Your mission is a failure!"))
      misn.finish(false)

   elseif mem.stage == 5 and spob.cur() == spob.get("Sroolu") then
      misn.markerRm(mem.mark)

      vn.clear()
      vn.scene()
      local joanne = vn.newCharacter( achack.vn_joanne() )
      vn.transition()
      vn.na(_([[After you both land your ships, you meet Joanne in the spaceport bar.]]))
      joanne(fmt.f(_([["Whew! That was definitely the most exciting round I've done to date! Thank you {player}, I probably owe you my life. You more than deserved your payment, I've already arranged for the transfer." Joanne hesitates, but then apparently makes up her mind. "In fact, would you sit down for a while? I think you deserve to know what this whole business with Harja is all about. And to be honest, I kind of want to talk to someone about this, and seeing how you're involved already anyway..."]]),
         {player=player.name()}))
      vn.na(_([[You decide to sit down and listen to Joanne's story, not in the last place because you're rather curious yourself.]]))
      joanne(fmt.f(_([["Thank you, {player}," Joanne says, "I appreciate it. Well, I guess I should start at the beginning."]]),
         {player=player.name()}))
      joanne(_([[She continues. "Several cycles ago, Harja and I were both students at the High Academy on Sinass. It's a very prestigious place among us Sirii, as you may or may not know. It's only one jump away from Mutris itself and... Well, anyway, it's one of the best academies in all of Sirius space, and only the most capable students are allowed to attend. Now, I don't mean to brag, you understand, but even in that environment I was among the top rated students. And, believe it or not, so was Harja. We were in the same study unit, actually.]]))
      joanne(_([["Another thing you should know is that the High Academy offers the very best among its students the chance to advance to the Serra echelon. You're not Sirian so you might not understand, but it's an exceptional honor for those born into the Shaira or Fyrra echelons to rise to a higher echelon. It's extremely valuable to us. So you see, the prospect of being rewarded like that is a very strong motivation for most of the students. It was no different for Harja and myself, since we were both Fyrra echelon."]]))
      joanne(_([["With our abilities, each of us had a good chance of earning the promotion. However, since we were in the same study unit, only one of us could be promoted; only one promotion is awarded per study unit each curriculum. That meant that Harja and I were rivals, but we were good-natured rivals. We each had every intention of winning the promotion through fair competition... Or so I thought."]]))
      joanne(_([["After the final exams had been taken and we were only days away from receiving the results, there was an incident. There had been a security breach in the academy's main computer. Someone had hacked the system and altered the data for the final exams, mine to be exact. My grades had been altered to be straight one hundred percent, in every subject. Can you believe that? Someone had actually tried to make it look like I was cheating. What were they thinking?"]]))
      joanne(_([[The academy staff wasn't fooled for even a moment. Nobody would be reckless enough to alter their own scores that way, so the only reason my scores would have been altered is if someone else did it, no doubt in order to discredit me. And you guessed it, the prime suspect was Harja. After all, if I was disqualified, he would certainly have gotten the promotion. Instead, he got what he deserved, and was expelled for his unethical attempt to secure his own success."]]))
      joanne(fmt.f(_([["That's basically the history between me and Harja. Up until you came to me, I just thought of him as an untrustworthy man whose own underhanded plan backfired on him. But here we are, cycles later, and now he's trying to kill me. Why, I wonder? Could he really be so bitter over what happened that he wants me dead? Even though he has nobody to blame but himself? I just don't understand it, {player}, I really don't."]]),
         {player=player.name()}))
      joanne(_([[Joanne remains silent for a moment, then takes a deep breath. "Whew, I feel quite a bit better now for having told this to you. Thanks for listening, it means a lot to me. I shouldn't keep you here any longer though, I'm sure you have a lot of other problems to look after."]]))
      vn.na(_([[Joanne leaves the spaceport bar. You can't help but reflect that even in the highest levels of society, you can find envy and vice.]]))

      vn.sfxVictory()
      vn.func( function ()
         player.pay(reward)
      end )
      vn.na(fmt.reward(reward))
      vn.run()

      var.pop("achack02repeat")
      srs.addAcHackLog( _([[Joanne, the Serra military officer who Harja tried to hire you to assassinate, enlisted you to aid her against would-be assassins. Along the way, she explained that Harja was a classmate of hers in the High Academy. According to her, Harja had hacked into the academy's main computer to change all of her grades to perfect scores in an attempt to sabotage her by making her look like a cheater.]]) )
      misn.finish(true)
   end
end

-- Enter hook.
function enter()
   if not (system.cur() == mem.nextsys and mem.joannejumped) then
      vntk.msg(_("You didn't follow Joanne!"), _("You jumped to a system before Joanne did, or you jumped to a different system than Joanne. Your mission is a failure!"))
      misn.finish(false)
   end
   if not player_has_fast_ship() then
      vntk.msg(_("Your ship is to slow!"), _("You need a faster ship to be able to protect Joanne. Your mission is a failure!"))
      misn.finish(false)
   end

   -- Warn the player if fuel is not sufficient
   if mem.warnFuel then
      if player.pilot():stats().fuel < 7*player.pilot():stats().fuel_consumption then
         vntk.msg(_("Fuel Warning"),_("You don't have enough fuel for making 7 jumps. You'll have to buy some from civilian ships on the way."))
      end
      mem.warnFuel = false
   end

   joanneship = pilot.add("Sirius Fidelity", achack.fct_sirius(), mem.origin, _("Joanne"))
   joanneship:control(true)
   joanneship:setVisplayer(true)
   joanneship:setHilight(true)
   joanneship:setInvincPlayer(true)
   local stats = joanneship:stats()
   local joanneshipspeed = stats.speed_max
   if mem.playershipspeed < joanneshipspeed then
     joanneship:setSpeedLimit(mem.playershipspeed)
   end

   -- Clear the natural pirate pilots to make it a "safe" trip
   pir.clearPirates(true)

   mem.joannejumped = false
   mem.origin = system.cur()

   if system.cur() == mem.destsys then
      mem.destplanet:landAllow(true)
      joanneship:land(mem.destplanet)
      hook.pilot(joanneship, "land", "joanneLand")
   else
      mem.nextsys = lmisn.getNextSystem(system.cur(), mem.destsys) -- This variable holds the system the player is supposed to jump to NEXT.
      joanneship:hyperspace(mem.nextsys)
      hook.pilot(joanneship, "jump", "joanneJump")
   end
   hook.pilot(joanneship, "death", "joanneDead")

   if system.cur() == system.get("Druss") and mem.stage == 3 then
      ambushSet({"Hyena", "Hyena"}, vec2.new(-12000, 9000))
   elseif system.cur() == system.get("Humdrum") and mem.stage == 4 then
      ambushSet({"Vendetta", "Vendetta"}, vec2.new(2230, -15000))
   end
end

-- Sets up the ambush ships and trigger area.
function ambushSet(ships, location)
   -- TODO should probably redo how this is handled. It's not very robust if
   -- the player or whatever attacks them early
   ambush = fleet.add(1, ships, achack.fct_thugs(), location)
   for _, j in ipairs(ambush) do
      j:control()
   end
   hook.timer(0.5, "proximity", {anchor = ambush[1], radius = 3000, funcname = "ambushActivate", focus = joanneship})
end

-- Commences combat once Joanne is close to the ambushers.
function ambushActivate()
   mem.deadmans = 0
   for _, j in ipairs(ambush) do
      if j:exists() then
         j:control(false)
         j:setHostile()
         j:setVisplayer()
         hook.pilot(j, "death", "ambusherDead")
      else
         mem.deadmans = mem.deadmans+1
      end
   end
   ambush[1]:broadcast(_("That's our target! Get her, boys!"))
   joanneship:control(false)
   misn.osdActive(2)
end

-- Death hook for ambushers.
function ambusherDead()
   mem.deadmans = mem.deadmans + 1
   if mem.deadmans == #ambush then
      joanneship:control()
      joanneship:hyperspace(mem.nextsys)
      misn.osdActive(1)
   end
end

-- Load hook. Makes sure the player can't start on military stations.
function on_load()
   if mem.stage > 1 and mem.stage < 5 then
      vntk.msg(_("Another stop successfully reached"), fmt.f(stoptext, {pnt=spob.cur()}))
      player.takeoff()
   elseif mem.stage == 5 then
      laststop_vn()
      player.takeoff()
   end
end

function joanneJump()
   mem.joannejumped = true
   player.msg(fmt.f(_("Joanne has jumped for the {sys} system. Follow her!"), {sys=mem.nextsys}))
end

function joanneLand()
   mem.joannelanded = true
   player.msg(fmt.f(_("Joanne has docked with {pnt}. Follow her!"), {pnt=mem.destplanet}))
end

function joanneDead()
   vntk.msg(_("Joanne's ship has been destroyed!"), _("Joanne's assailants have succeeded in destroying her ship. Your mission is a failure!"))
   misn.finish(false)
end
