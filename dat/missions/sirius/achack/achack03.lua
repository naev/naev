--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Joanne's Doubt">
 <unique />
 <priority>3</priority>
 <done>Harja's Vengeance</done>
 <chance>20</chance>
 <location>Bar</location>
 <faction>Sirius</faction>
 <notes>
  <campaign>Academy Hack</campaign>
 </notes>
</mission>
 --]]
--[[
   This is the third mission in the Academy Hack minor campaign.
--]]
require "proximity"
local srs = require "common.sirius"
local fmt = require "format"
local achack = require "common.achack"
local vn = require "vn"
local vntk = require "vntk"
local love_shaders = require "love_shaders"

local harja -- Non-persistent state

local reward = 1e6

function create()
   -- Note: this mission does not make any system claims.
   misn.setNPC( achack.joanne.name, achack.joanne.portrait, _("Joanne, the Serra military officer, is here enjoying a drink by herself."))
end

function accept()
   local accepted = false

   vn.clear()
   vn.scene()
   local joanne = vn.newCharacter( achack.vn_joanne() )
   vn.transition()

   if var.peek("achack03repeat") then
      joanne(fmt.f(_([["Hi {player}," Joanne says. "Good to see you again. Say, I still haven't resolved that issue with Harja and the attempts on my life. Any chance you've changed your mind about helping out a damsel in distress? Let me tell you again what I want you to do."]]),
         {player=player.name()}))
   else
      joanne(fmt.f(_([["Hello there {player}," Joanne greets you. "We do seem to keep running into each other, don't we? Like they say, it's a small galaxy after all."]]),
         {player=player.name()}))
      vn.na(_([[You and Joanne spend some time chatting. You learn that Joanne often comes to public spaceport bars to relax. She doesn't seem to enjoy the military cantinas much for some reason. After some small talk, the conversation inevitably gets on the topic of your previous encounter.]]))
      joanne(_([["Actually, that's still bothering me," Joanne confides. "Like I told you, the whole incident with Harja at the academy was a closed chapter for me until the attempts at my life started. It's been giving me sleepless nights. I don't mind admitting that I'm a little scared that someone out there is trying to kill me, but that's not everything."]]))
      joanne(_([["I know Harja, or at least I knew him for cycles. These assassinations, the whole fraud, it doesn't add up with the image I had of him. I've been thinking what to do about it, but I'm not sure. I considered reporting the matter to my superiors and let the armed forces handle it. That would probably be the end of it, but I can't shake the feeling that I would never be free from these doubts I'm having.]]))
      joanne(fmt.f(_([["Actually, {player}, now that you're here anyway, maybe you can help me figure out what's really happening. I need to know. It's really preying on my mind. Since you've been so helpful in the past, maybe you're willing to make another effort?"]]),
         {player=player.name()}))
   end
   vn.func( function ()
      var.push("achack03repeat", true)
   end )

   joanne(_([["I want you to get out there and find Harja. I doubt he's on the Wringer anymore. That place is bad for your health, especially for an academic type like Harja. He's probably on the move, doing whatever it is he does to be able to hire those assassins. So, I need you to intercept him and get him to tell you what he thinks he's doing. I don't know where he is now, you'll have to look for him yourself. I don't think he'll venture outside Sirius controlled space though."]]))
   joanne(_([[He won't be happy to see you, of course, so you may have to be a little... persuasive, shall we say? Just make sure to find out what his motives are and what it'll take for him to leave the past alone. But!" and here Joanne's face turns stern, "I don't want any serious harm to come to him. You may be thinking that removing Harja will solve the problem, but even if he's a threat to my life, murder is still murder. This is a private investigation, not a private war. Are we clear?"]]))
   vn.na(_([[You acknowledge that you won't kill Harja if you can possibly help it. This seems to satisfy Joanne.]]))
   joanne(fmt.f(_([[ "Good. I know I can trust you, {player}. I can offer you {credits} if you complete this job. Go find Harja. Make him talk. Then come back and tell me what he said. Maybe, hopefully, it'll put my mind at ease."]]),
      {player=player.name(), credits=fmt.credits(reward)}))
   vn.menu{
      {_([[Accept the job.]]),"accept"},
      {_([[Decline.]]),"decline"},
   }

   vn.label("decline")
   vn.na(_([[You decide you can't be bothered to find Harja now. Maybe later.]]))
   vn.done()

   vn.label("accept")
   vn.func( function () accepted=true end )
   joanne(_([["Oh, I'm glad to hear that. Here, I'll upload the details of Harja's private ship into your computer. I did some digging in the military database to find them. You don't graduate from the Sinass High Academy with honors without picking up a few tricks! I know, I know, it's classified data, but it's for a good cause, wouldn't you say?"]]))
   joanne(_([["You should be able to identify Harja when you pick him up on your sensors now. If you have trouble locating him, consider installing better sensors on your ship so you can pick him up from farther away. But don't spend too much effort looking for him, just keep a look out as you go about your normal business. If you just stay in Sirius space, I'm sure you'll run into him sooner or later."]]))
   joanne(fmt.f(_([[Joanne gets up to leave, but before she goes she adds, "I'll be on {pnt} for a while longer, so come back here when you've got something. Good luck!"]]),
      {pnt=spob.cur()}))
   vn.run()

   if not accepted then return end

   mem.destplanet, mem.destsys = spob.cur() -- Keeps track of where the mission was accepted.
   mem.origin = spob.cur() -- Keeps track of where the player enters the system from.

   misn.accept()
   misn.setDesc(_("Joanne wants you to find Harja and interrogate him about his motives."))
   misn.setReward(reward)
   misn.osdCreate(_("Joanne's Doubt"), {
      _("Find Harja in Sirius space"),
      _("Talk to Harja"),
      fmt.f(_("Return to Joanne on {pnt} ({sys} system)"), {pnt=mem.destplanet, sys=mem.destsys}),
   })

   mem.enterhook = hook.enter("enter")
   mem.enterhook = hook.jumpout("jumpout")
   hook.land("land")
   hook.date(time.new(0, 2, 0), "date")
end

-- Jump-out hook.
function jumpout()
   mem.origin = system.cur()
end

-- Enter hook.
function enter()
   if not mem.harjatalked then
      misn.osdActive(1)
   end
end

-- Date hook.
function date()
   if (harja == nil or not harja:exists()) and system.cur():presences()["Sirius"] then
      -- Determine spawn point. The reason why we don't use the normal random is that we don't want Harja spawning from the same place as the player.
      local spawnpoints = tmergei(system.cur():adjacentSystems(), system.cur():spobs())
      for i, j in ipairs(spawnpoints) do
         if j == mem.origin then
            table.remove(spawnpoints, i) -- The place the player entered from is not a valid spawn point.
         end
      end
      mem.spawnpoint = spawnpoints[rnd.rnd(#spawnpoints)]

      harja = pilot.add("Shark", achack.fct_sirius(), mem.spawnpoint, _("Harja's Shark"), {ai="trader"})
      harja:memory().aggressive = true
      harja:control()
      harja:follow(player.pilot())
      hook.timer(0.5, "proximityScan", {focus = harja, funcname = "detectHarja"})
   end
end

-- Triggers when the player's sensors pick up Harja's ship.
function detectHarja()
   player.autonavReset(5)
   vntk.msg(_("Harja spotted!"), _("Your sensors flash you an alert. They have picked up Harja's ship! This is a good opportunity to talk to him, and hailing his ship seems the most straightforward way to go about it."))
   harja:setHilight()
   harja:control(false)
   hook.pilot(harja, "hail", "hail")
   hook.pilot(harja, "disable", "disable")
   hook.pilot(harja, "board", "board")
   hook.pilot(harja, "death", "death")
   hook.pilot(harja, "land", "leave")
   hook.pilot(harja, "jump", "leave")
   misn.osdActive(2)
end

-- Triggers when Harja leaves the system.
function leave()
   player.msg(_("Harja is no longer in the system."))
   if not mem.harjatalked then
      misn.osdActive(1)
   end
end

-- Harja's hail hook.
function hail()
   player.commClose()
   if not mem.hailed then

      vn.clear()
      vn.scene()
      local hrj = achack.vn_harja{shader=love_shaders.hologram()}
      vn.transition("electric")
      vn.na(_([[You hail Harja's ship. A few moments later his face appears on your console. When he recognizes you, his face sets in an angry scowl.]]))
      hrj(_([["You! How dare you show your face to me again after double-crossing me like that! I had a perfect shot at taking out that damn woman, but you had to go and tip her off instead. Worse, you even helped her fight off the other guys I hired to blast her into space dust! You ruined everything! The past cycles of my life, completely wasted! Damn you!"]]))
      hrj(_([[You try to convince Harja that you want to have a word with him about this matter, but he is furious. "I've got nothing to say to a traitorous scumbag like you! Now leave me alone, I'm having a bad day as it is without you spoiling it further!" With that, Harja breaks the connection. Oh well, looks like you're going to have to do this the hard way.]]))
      vn.done("electric")
      vn.run()

      mem.hailed = true
   else
      player.msg(_("Harja doesn't respond. It doesn't look like he wants to talk to you."))
   end
end

-- Harja's disable hook.
function disable()
   if harja:health() > 0 then
      harja:setInvincible()
      harja:disable() -- This is to make sure he doesn't re-enable himself.
      vntk.msg(_("Harja's ship is disabled"), _("With a few well placed shots you overload Harja's systems and set his ship adrift. Now you can have a little talk with him, face to face."))
   end
end

-- Harja's board hook.
function board()
   vn.clear()
   vn.scene()
   local hrj = achack.vn_harja{shader=love_shaders.hologram()}
   vn.transition()
   vn.na(_([[You step through the airlock onto Harja's ship. Harja comes storming out of the cockpit, his face red with rage, but he pipes down considerably when you draw your sidearm and point it at his chest. It takes little further persuasion to get him to take his captain's seat again and hear you out.]]))
   hrj(_([["What do you mean, my side of the story?" Harja grumbles after you've explained your actions. "I'm taking revenge, that's all there is to it. Did she tell you about the academy on Sinass? Okay. Did she also tell you how she framed me for the computer hack? No, I bet she didn't! But that's the truth of it, mark my words! She knew I was at least as good as she was, so she needed to get rid of me to be sure she'd get the promotion."]]))
   hrj(_([["Oh yeah, she's a clever one, that Joanne. She knew nobody would believe she'd try to forge her own scores, and everyone knew I was her only competition. So she set me up! Just to get me expelled! I spent almost my whole life up to that point getting to where I was, and she destroyed it in a single night. Yeah, I want revenge. It's all I've got left."]]))
   vn.na(_([[You point out that Harja had more than enough motive to place that hack himself, and that, unlike Harja's, you have no reason to doubt Joanne's integrity. Harja deflates a little.]]))
   hrj(_([["Okay, look, I know what it looks like. I know I was hanging out on the Wringer, that I hired you as a hit man, and that I lied about her being a criminal. That doesn't help my case. But I didn't do it! I was confident I could win the promotion fair and square. Still think I would have gotten it if I hadn't been thrown out. I can't prove it, but on Sirichana I swear it wasn't me. And now you either believe me or you believe her. I've told you everything."]]))
   vn.na(_([[It's clear to you that you're not going to get anything else out of him. You don't know what to make of this. Both Harja and Joanne seem convinced that the other hacked the academy's computer, and neither of them has any evidence to prove their claim. This whole matter doesn't seem any closer to a conclusion than before, but you have little choice other than to report your findings to Joanne at this point.]]))
   vn.run()

   player.unboard()
   harja:setHealth(100, 100)
   harja:control()
   harja:hyperspace()
   hook.rm(mem.enterhook)
   hook.rm(mem.datehook)
   misn.osdActive(3)
   misn.markerAdd(mem.destplanet, "low")
   mem.harjatalked = true
end

-- Harja's death hook.
function death()
   vntk.msg(_("Harja's ship is destroyed!"), _("You have destroyed Harja's ship! You were supposed to talk to him, not destroy him. Your mission is a failure!"))
   misn.finish(false)
end

-- Land hook.
function land()
   mem.origin = spob.cur()
   if spob.cur() == mem.destplanet and mem.harjatalked then
      player.landWindow("bar")

      vn.clear()
      vn.scene()
      local joanne = vn.newCharacter( achack.vn_joanne() )
      vn.transition()

      vn.na(_([[You meet Joanne in the spaceport bar. She listens to your account of your conversation with Harja. When you're finished, she frowns.]]))
      joanne(_([["I've got to admit, I find his story a little disturbing. He genuinely seems to believe I framed him for that hack, just as I believe he tried to frame me. From what you told me, it doesn't seem like he was just putting up a self-righteous story to justify his actions, and I wouldn't expect that from him anyway. But that's impossible. Besides the two of us, there was nobody who had the slightest interest in removing either one of us from the academy."]]))
      joanne(_([["I know I didn't do it, so that means Harja must have done it. Only..." Joanne pauses. "You said he swore an oath. Whatever else I might think of him, I can't quite believe he would abuse his Sirian beliefs in such a way. We Sirii take our faith very seriously."]]))
      joanne(fmt.f(_([["I don't know what to say, {player}. The more I find out about all this, the less sense it makes. I need to think about this, see if there's something I can figure out on my side. I suspect this isn't the end of it yet, though, so I may need your help again soon. But for now, I should give you what I promised you so you can take care of your other matters."]]),
         {player=player.name()}))
      vn.na(_([[Joanne pays you the agreed upon sum. Then she walks out of the spaceport bar, a thoughtful expression on her face. It seems your role in the conflict between Joanne and Harja is growing. Who knows where it'll end.]]))

      vn.sfxVictory()
      vn.func( function ()
         player.pay(reward)
      end )
      vn.na(fmt.reward(reward))
      vn.run()

      var.pop("achack03repeat")
      srs.addAcHackLog( _([[Joanne hired you to interrogate Harja about his motives for trying to assassinate her. He was unwilling to talk to you, but when you backed him into a corner, Harja claimed that it was Joanne who hacked the High Academy's main computer to change her scores in an attempt to frame him. He swore "on Sirichana" that he wasn't responsible for the hack. Joanne took this oath seriously, saying that he wouldn't "abuse his Sirian beliefs". She said that she may need your help again soon.]]) )
      misn.finish(true)
   end
end
