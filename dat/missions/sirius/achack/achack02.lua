--[[
-- This is the second mission in the Academy Hack minor campaign.
--]]

include "dat/scripts/nextjump.lua"
include "fleethelper.lua"
include "selectiveclear.lua"
include "proximity.lua"

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

    title1 = "An unexpected reunion"
    text1 = [[When you approach her, the officer greets you with a smile. "What a surprise that we should run into each other again," she says. "I'm afraid to say I don't remember your name. What was it again? Ah yes, %s. I don't think I introduced myself last time, my name is Joanne. Well met. As you can see I'm still doing quite well, no poison in my wine or snakes in my bed or anything." Then her expression turns more serious. "Actually, about that. I think our friend Harja still has it in for me. You had the common sense to use your head, but I'm afraid not everyone is like that. I'm convinced Harja will try to hire more assassins to do what you didn't, so I'm in considerable danger."
    You sympathize with Joanne, but you wonder aloud why she hasn't asked the local authorities for protection. Joanne gives you a somewhat uncomfortable look.
    "Pride, I suppose. I'm a military officer. It wouldn't look too handsome if I asked for personal protection when the navy is already stretched thin out there, trying to protect our civilians from pirates and other criminals every day. Besides, my conflict with Harja is a personal matter. I feel I should resolve this with my own resources." She gives you a wan smile. "That being said, I wouldn't say no to a helping hand."]]
    
    text1r = [["Hello again, %s," Joanne greets you. "I'm afraid I still find myself under threat from mercenary assassins. Have you reconsidered my offer? Let me tell you again what I need."]]
    
    text2 = [["See, this is the situation," she continues. "My duties as a Sirian administration officer require me to pay visits to several military outposts throughout Sirius space. It's a long trek every time, but that just comes with the job. Now, the trouble is that Harja's assassins might have many opportunities to ambush me along the way. I've had combat training, of course, just like every other Serra soldier, but I'm afraid I have little actual fighting experience, and I'm not sure I could survive an attack unassisted. You, however, seem to have seen quite some action. If you were to escort me while I make my rounds, I would feel a lot more secure. I can reward you, of course. Let's see... Another 150,000 credits seems a fair sum. What do you think, are you willing to do it?"]]
    
    title2 = "Joanne's escort"
    text3 = [["That's wonderful! Thank you so much. As I said, my job requires that I travel between Sirian military bases. You're going to need fuel to make the necessary jumps, of course. Now that you have agreed to be my personal escort I can give you clearance to dock with those bases if you don't have it already so you can automatically refuel, but either way I won't be on station long, so you won't have time to disembark and do other things while you're there.
    "Also, I think this goes without saying, but I need you to stick close to me so your ship must be fast enough. Also don't jump to any systems before I do, don't jump to the wrong systems, and don't land on any planets I don't land on. If we meet any unpleasantries along the way I will rely on you to help me fight them off. That's about it. I'll finish up a few things here, and then I'll head to my ship. I'll be in the air when you are."
    Joanne leaves the bar. You will meet up with her in orbit.]]
    
    title3 = "Mission accomplished"
    text4 = [[You go through the now familiar routine of waiting for Joanne. She soon hails you on the comms.
    "That's it, %s! This was the final stop. You've been a great help. This isn't a good place to wrap things up though. Tell you what, let's relocate to Sroolu and meet up in the spaceport bar there. I need to give you your payment of course, but I also want to talk to you for a bit. See you planetside!"
    The comm switches off. You prepare to take off and set a course for Sroolu.]]
    
    title4 = "One damsel, safe and sound"
    text5 = [[After you both land your ships, you meet Joanne in the spaceport bar.
    "Whew! That was definitely the most exciting round I've done to date! Thank you %s, I probably owe you my life. You more than deserved your payment, I've already arranged for the transfer." Joanne hesitates, but then apparently makes up her mind. "In fact, would you sit down for a while? I think you deserve to know what this whole business with Harja is all about. And to be honest, I kind of want to talk to someone about this, and seeing how you're involved already anyway..."
    You decide to sit down and listen to Joanne's story, not in the last place because you're rather curious yourself.
    "Thank you, %s," Joanne says, "I appreciate it. Well, I guess I should start at the beginning."]]
    
    title5 = "Joanne and Harja"
    text6 = [["Several SCU ago, me and Harja were both students at the High Academy on Sinass. It's a very prestigious place among us Sirii, as you may or may not know. It's only one jump away from Mutris itself and... Well, anyway, it's one of the best academies in all of Sirius space, and only the most capable students are even allowed to attend. Now, I don't mean to brag, you understand, but even in that environment I was among the top rated students. And, believe it or not, so was Harja. We were in the same study unit, actually.
    "Another thing you should know is that the High Academy offers the very best among its students the chance to advance to the Serra echelon. You're not Sirian so you might not understand, but it's an exceptional honor for those born into the Shaira or Fyrra echelons to rise to a higher echelon. It's extremely valuable to us. So you see, the prospect of being rewarded like that is a very strong motivation for most of the students. It was no different for Harja and myself, since we were both Fyrra echelon. With our abilities, each of us had a good chance of earning the promotion. However, since we were in the same study unit, only one of us could be promoted, since only one promotion is awarded per study unit each curriculum. That meant that Harja and I were rivals, but we were rivals in good sport. We each had every intention of winning the promotion through fair competition... Or so I thought."]]
    
    text7 = [["After the final exams had been taken and we were only days away from receiving the results, there was an incident. There had been a security breach in the academy's main computer. Someone had hacked the system and altered the data for the final exams, mine to be exact. My grades had been altered to be straight one hundred per cents, in every subject. Can you believe that? Someone had actually tried to make it look like I was cheating. What were they thinking? The academy staff wasn't fooled for even a moment. Nobody would be stupid enough to alter their own scores that way, so the only reason my scores would have been altered is if someone else did it, no doubt in order to discredit me. And you guessed it, the prime suspect was Harja. After all, if I was disqualified, he would certainly have gotten the promotion. Instead, he got what he deserved, and was expelled for his low attempt to secure his own success."
    "That's basically the history between me and Harja. Up until you came to me, I just thought of him as an untrustworthy man whose own underhanded plan backfired on him. But here we are, cycles later, and now he's trying to kill me. Why, I wonder? Could he really be so bitter over what happened that he wants me dead? Even though he has nobody to blame but himself? I just don't understand it, %s, I really don't."
    Joanne remains silent for a moment, then takes a deep breath. "Whew, I feel quite a bit better now for having told this to you. Thanks for listening, it means a lot to me. I shouldn't keep you here any longer though, I'm sure you have a lot of other problems to look after."
    Joanne leaves the spaceport bar. You can't help but reflect that even in the highest levels of society, you can find envy and vice.]]

    stoptitle = "Another stop successfully reached"
    stoptext = "You dock with %s, and the spacedock personnel immediately begins to refuel your ship. You spend a few minutes going through checklists and routine maintenance operations. Then you get a ping on your comms from Joanne. She tells you that she has finished her business on this station, and that she's taking off again. You follow suit."

    destfailtitle = "You didn't follow Joanne!"
    sysfailtext = "You jumped to a system before Joanne did, or you jumped to a different system than Joanne. Your mission is a failure!"
    planetfailtext = "You landed on a planet Joanne didn't land on. Your mission is a failure!"
    
    deathfailtitle = "Joanne's ship has been destroyed!"
    deathfailtext = "Joanne's assailants have succeeded in destroying her ship. Your mission is a failure!"

    toslowshiptitle = 'Your ship is to slow!'
    toslowshipmsg = "You need a faster ship to be able to protect Joanne. Your mission is a failure!"

    jumpmsg = "Joanne has jumped for the %s system. Follow her!"
    landmsg = "Joanne has docked with %s. Follow her!"
    
    ambushmsg = "That's our target! Get her, boys!"

    -- Mission info stuff
    joannename1 = "The Serra military officer"
    joannedesc1 = "You know this woman. She's the military officer from before, the one you were hired to assassinate."
    joannename2 = "Joanne"
    joannedesc2 = "This is Joanne, the woman you were hired to assassinate."

    osd_msg   = {}
    osd_title = "Harja's Vengeance"
    osd_msg[1] = "Follow Joanne's ship"
    osd_msg[2] = "Defeat Joanne's attackers"
    osd_msg["__save"] = true
    osd_final = {"Land on Sroolu to get your reward"}
    osd_final["__save"] = true

    misn_desc = "Joanne needs you to escort her ship and fight off mercenaries sent to kill her."
    misn_reward = "Joanne will pay you another 150,000 credits."
end

function create()
   if not misn.claim ( {system.get("Humdrum"), system.get("Lapis")} ) then
      abort()
   end
   
   if var.peek("achack02repeat") then
      misn.setNPC(joannename1, "sirius/unique/joanne")
      misn.setDesc(joannedesc1)
   else
      misn.setNPC(joannename2, "sirius/unique/joanne")
      misn.setDesc(joannedesc2)
   end
end

function player_has_fast_ship()
  local stats = player.pilot():stats()
  playershipspeed = stats.speed_max
  local has_fast_ship = false
     if playershipspeed > 200 then
        has_fast_ship = true
     end
     return has_fast_ship
end

function accept()
   if var.peek("achack02repeat") then
      tk.msg(title1, text1r:format(player.name()))
   else
      tk.msg(title1, text1:format(player.name()))
   end
   var.push("achack02repeat", true)
   if not tk.yesno(title1, text2) then
      misn.finish()
   end
   tk.msg(title2, text3)
   
   -- This is the route Joanne will take.
   route = {"Violin Station", "Fyruse Station", "Inios Station", "Tankard Station", "Sroolu"}
   route["__save"] = true
   stage = 1

   destplanet, destsys = planet.get(route[stage])
   nextsys = system.cur() -- This variable holds the system the player is supposed to jump to NEXT. This is the current system when taking off.
   origin = planet.cur() -- Determines where Joanne spawns from. Can be a planet or system.
   joannejumped = true -- Determines if Joanne has jumped. Needs to be set when landed.
   
   misn.accept()
   misn.setDesc(misn_desc)
   misn.setReward(misn_reward)
   misn.osdCreate(osd_title, osd_msg)
   
   hook.land("land")
   hook.enter("enter")
   hook.load("load")
end

-- Land hook.
function land()
   if planet.cur() == destplanet and joannelanded and stage < 4 then
      stage = stage + 1
      destplanet, destsys = planet.get(route[stage])
      origin = planet.cur()
      player.refuel(200)
      tk.msg(stoptitle, stoptext:format(planet.cur():name()))
      joannejumped = true -- She "jumped" into the current system by taking off.
      player.takeoff()
   elseif planet.cur() == destplanet and joannelanded and stage == 4 then
      tk.msg(title3, text4:format(player.name()))
      stage = stage + 1
      misn.osdCreate(osd_title, osd_final)
      origin = planet.cur()
      destplanet, destsys = planet.get(route[stage])
      joannejumped = true -- She "jumped" into the current system by taking off.
      misn.osdCreate(osd_title, osd_final)
      player.takeoff()
   elseif stage < 4 then
      tk.msg(destfailtitle, planetfailtext)
      abort()
   elseif stage == 5 and planet.cur() == planet.get("Sroolu") then
      tk.msg(title4, text5:format(player.name(), player.name()))
      tk.msg(title5, text6)
      tk.msg(title5, text7:format(player.name()))
      player.pay(150000) -- 150K
      var.pop("achack02repeat")
      misn.finish(true)
   end
end

-- Enter hook.
function enter()
   if not (system.cur() == nextsys and joannejumped) then
      tk.msg(destfailtitle, sysfailtext)
      abort()
   end
   if not player_has_fast_ship() then
      tk.msg(toslowshiptitle, toslowshipmsg)
      abort()
   end
   
   joanne = addRawShips("Sirius Fidelity", "sirius_norun", origin, "Achack_sirius")[1]
   joanne:control()
   joanne:rename("Joanne")
   joanne:rmOutfit("all")
   joanne:addOutfit("Razor MK2", 3)
   joanne:addOutfit("Reactor Class I", 1)
   joanne:addOutfit("Shield Capacitor", 1)
   joanne:setVisplayer()
   joanne:setHilight()
   joanne:setInvincPlayer()
   local stats = joanne:stats()
   local joanneshipspeed = stats.speed_max
   if playershipspeed < joanneshipspeed then
     joanne:setSpeedLimit(playershipspeed)
   end
 
   
   pilot.toggleSpawn("Pirate")
   pilot.clearSelect("Pirate") -- Not sure if we need a claim for this.

   joannejumped = false
   origin = system.cur()

   if system.cur() == destsys then
      destplanet:landOverride(true)
      joanne:land(destplanet)
      hook.pilot(joanne, "land", "joanneLand")
   else
      nextsys = getNextSystem(system.cur(), destsys) -- This variable holds the system the player is supposed to jump to NEXT.
      joanne:hyperspace(nextsys)
      hook.pilot(joanne, "jump", "joanneJump")
   end
   hook.pilot(joanne, "death", "joanneDead")
   
   if system.cur() == system.get("Valur Gem") and stage == 3 then
      ambushSet({"Hyena", "Hyena"}, vec2.new(880, -140))
   elseif system.cur() == system.get("Humdrum") then
      ambushSet({"Vendetta", "Vendetta"}, vec2.new(2230, -15000))
   end
end

-- Sets up the ambush ships and trigger area.
function ambushSet(ships, location)
   ambush = addRawShips(ships, "baddie_norun", location, "Achack_thugs")
   for _, j in ipairs(ambush) do
      j:control()
   end
   hook.timer(500, "proximity", {anchor = ambush[1], radius = 1500, funcname = "ambushActivate", focus = joanne}) 
end

-- Commences combat once Joanne is close to the ambushers.
function ambushActivate()
   for _, j in ipairs(ambush) do
      j:control(false)
      j:setHostile()
      j:setVisplayer()
      hook.pilot(j, "death", "ambusherDead")
      deadmans = 0
   end
   ambush[1]:broadcast(ambushmsg)
   joanne:control(false)
   misn.osdActive(2)
end

-- Death hook for ambushers.
function ambusherDead()
   deadmans = deadmans + 1
   if deadmans == #ambush then
      joanne:control()
      joanne:hyperspace(nextsys)
      misn.osdActive(1)
   end
end

-- Load hook. Makes sure the player can't start on military stations.
function load()
   if stage > 1 and stage < 5 then
      tk.msg(stoptitle, stoptext:format(planet.cur():name()))
      player.takeoff()
   elseif stage == 5 then
      tk.msg(title3, text4:format(player.name()))
      player.takeoff()
   end
end

function joanneJump()
   joannejumped = true
   player.msg(jumpmsg:format(nextsys:name()))
end

function joanneLand()
   joannelanded = true
   player.msg(landmsg:format(destplanet:name()))
end

function joanneDead()
   tk.msg(deathfailtitle, deathfailtext)
   abort()
end

function abort()
   misn.finish(false)
end

