--[[
-- This is the mission part of the shipwrecked Space Family mission, started from a random event.
-- See dat/events/neutral/shipwreck.lua
--]]

shipname = _("August") --The ship will have a unique name

title = {}
text = {}
directions = {}
title[1] = _("Shipwrecked space family")
text[1] = _([[The airlock opens, and you are greeted by a nervous-looking man, ditto woman and three neurotic children.
    "Thank god you are here," the man says. "I don't know how much longer we could've held out. They left us for dead, you know. No fuel, no food and only auxiliary power to sustain us." He then begins to incoherently tell you how much his group has suffered in the past few hours, but you cut him short, not willing to put up with his endless babbling.
    With a few to-the-point questions you learn that the man's name is Harrus, and that he and his wife and children live, or at least used to live, aboard their trading vessel. "It was a good life, you know," Harrus tells you. "You get to see the galaxy, meet people and see planets, and all that while working from home because, haha, you take your home with you!"
    You can't help but glance at Harrus' kids, who have begun enthusiastically stampeding through your ship, pressing any buttons low enough for them to reach, despite their mother's hopeless attempts to keep them under control.]])
text[2] = _([[Harrus is about to launch into another anecdote about his existence as a trader, but you manage to forestall him. You soon learn that his family's lifestyle has come to an abrupt change at the hands of a minor gang of pirates. Though the %s had some weaponry and shielding systems, the attackers were too much for a single cargo ship.
    "I never thought it would end like this," Harrus sighs. "I mean, I knew space was dangerous, but I stayed clear of the unsafe areas. Stuck to the patrolled lanes. Didn't take any risks. I've got a family, you know."
    Then Harrus brightens up, apparently putting his recent misfortune behind him in the blink of an eye. "Everything's going to be fine now," he says cheerfully. "We've been rescued, and all we need now is for you to take us to a suitable world where we can build a new life."
    Without further ado, and without so much as formally asking for the favour, Harrus and his family proceed onto your ship and install themselves into your living quarters. They do not seem about to leave.]])
title[2] = _("Next stop")
directions[1] = _([["I know just the place," Harrus tells you. "Take us to planet %s in the %s system. I'm sure a man of my calibre can find everything he needs there. Captain, please notify me when we arrive." With that, Harrus turns and rejoins his family. The kids seem in the process of redecorating (if not wrecking) your quarters, and despite the apologetic glance the woman gives you you can't help but wonder if you did the right thing responding to that SOS.]])
title[3] = _("Distractions on the bridge")
harrass_msg = _([[You are going over a routine navigation check when Harrus enters your cabin unannounced. He seems to have recovered from his distressed state, and now radiates confidence.
    "Captain," he says to you. "I hope I don't have to remind you that we must get to our destination as soon as possible. I have a wife and children to think of and frankly I find your, ah, facilities a bit lacking."
    You consider ordering Harrus off your bridge, but he doesn't seem the kind of man to back off, so the only thing you would accomplish is to sour the mood on your ship. You inform Harrus that you're making every effort to get his family to a safe haven, which seems to satisfy him. Finally alone again, you take a moment to subside before completing that check.]])
directions[2] = _([[Harrus steps out of your ship and takes a look around the spaceport you docked at. "No, no. This won't do at all," he says disapprovingly. "This place is a mess! Look at the dust and grime!" He rounds on you. "How are we supposed to make a decent living in a dump like this? You've brought us to the wrong place altogether. I must say I'm disappointed. I demand you take us away from this abysmal hole this minute! Let's see... Yes, %s in %s will do. At least they're civilized there!"
    You attempt to remind Harrus that it was in fact he who asked you to take him to this system in the first place, and that the spaceport is hardly a representation of the entire world, but the man doesn't want to hear it. He stalks back into your ship without another word, leaving you annoyed and frustrated. Harrus' wife worriedly peeks around the corner of the hatch, silently eyeing you her sympathy.
    You heave a sigh, and proceed to the registration desk to get the docking formalities out of the way.]])
directions[3] = _([["The sky! Have you LOOKED at it?"
    Harrus rounds on you with a furious expression. Your keen understanding of the human body language tells you he isn't happy. You thought he might be satisfied with the state of the spacedock, since it's kept in prime condition, and indeed he was. That changed as soon as he looked up.
    "It's com-plete-ly the wrong colour," Harrus fumes. "It's a mockery of our standards of living, and it's right there overhead! Do you want my children to grow up believing the sky is supposed to look like, like... like THAT?" Harrus again looks up at the heavens that offend him so. "No, captain, my patience is at an end. I expect you to take me and my family to %s in the %s system. We've got relatives there who will take us in. I will waste my time with this pointless endeavour no longer!" 
    Before you get a chance at making a snappy retort, Harrus storms back to his (your) quarters, leaving you to either vent your anger on his wife, who is hovering nearby, or keep it to yourself. Since the poor woman has done nothing wrong, you grimly return to the bridge.]])
title[4] = _("Rid of them at last")
text[3] = _([[You land at your final stop in your quest to take the space family home, and not a moment too soon for both you and Harrus. Harrus stomps off your ship without so much as a greeting, his wife and children in tow, and you are just as happy to see them gone.
    Surveying your now deserted quarters, you are appalled at how much damage the temporary inhabitants have managed to do along the way. You console yourself with the thought that at least you'll have something to do during the dull periods in hyperspace and turn to tend to your ships needs, when your eye falls on a small box that you don't remember seeing here before.
    Inside the box, you find a sum of credits and a note written in neat, feminine handwriting that says, "Sorry for the trouble."]])

carg_type = "Civilians" 

-- Mission details
misn_title = _("The Space Family")
misn_reward = _("A clear conscience.")
misn_desc = {}
misn_desc[1] = _("A shipwrecked space family has enlisted your aid. Can you take them to safety?")
misn_desc[2] = _("Take the space family to %s (%s system).")

-- Aborted mission
msg_abortTitle = _("A parting of ways")
msg_abort_space = _([[You unceremoniously shove your passengers out of the airlock and into the coldness of space. You're done playing taxi; it's time to get back to important things!]])
msg_abort_landed = _([[You unceremoniously shove your passengers out of the airlock, leaving them to their fate on this planet. You're done playing taxi; it's time to get back to important things!]])

-- OSD stuff
osd_title = {}
osd_msg   = {}
osd_title[1] = _("The space family")
osd_msg[1]   = {
   _("A shipwrecked space family has enlisted your aid. Can you take them to safety?")
}


include("dat/scripts/jumpdist.lua")


function create ()
   -- Note: this mission does not make any system claims. 
   misn.accept() -- You boarded their ship, now you're stuck with them.
   misn.setTitle( misn_title )
   misn.setReward( misn_reward )
   misn.setDesc( misn_desc[1] )

   inspace = true -- For lack of a test, we'll just have to keep track ourselves.
   harrassmsg = true

   -- Intro text, player meets family
   tk.msg(title[1], text[1])
   tk.msg(title[1], string.format(text[2], shipname))

   carg_id = misn.cargoAdd( carg_type, 0 )

   -- First stop; subsequent stops will be handled in the land function
   nextstop = 1
   targsys = getsysatdistance(nil, 3) -- Populate the array
   targsys = getlandablesystems( targsys )
   if #targsys == 0 then targsys = {system.get("Apez")} end -- In case no systems were found.
   destsys = targsys[rnd.rnd(1, #targsys)] 
   destsysname = system.name(destsys)
   destplanet = getlandable(destsys) -- pick a landable planet in the destination system
   destplanetname = destplanet:name()
   tk.msg(title[2], string.format(directions[nextstop], destplanetname, destsysname)) -- NPC telling you where to go
   misn.setDesc(string.format(misn_desc[2], destplanetname, destsysname))
   misn.osdCreate(misn_title, {misn_desc[2]:format(destplanetname, destsysname)})
   misn_marker = misn.markerAdd( destsys, "low" )

   -- Force unboard
   player.unboard()

   hook.land("land")
   hook.takeoff("takeoff")
   hook.enter("enter")
end

-- Checks if the parameter system has planets you can land on. Return true if so, false otherwise.
function haslandable(sys)
   for a, b in pairs(sys:planets()) do
      if b:services()["inhabited"] then return true
      end
   end
   return false
end

-- Given a system, return the first landable planet found, or nil if none are landable (shouldn't happen in this script)
function getlandable(sys)
   for a, b in pairs(sys:planets()) do
      if b:services()["inhabited"] and b:canLand() then
         return b
      end
   end
   return nil
end

function land()
   if planet.cur() == destplanet then -- We've arrived!
      if nextstop >= 3 then -- This is the last stop
         tk.msg(title[4], string.format(text[3], destsysname)) -- Final message
         player.pay(500000)
         misn.cargoJet(carg_id)
         misn.finish(true)
      else
         nextstop = nextstop + 1
         targsys = getsysatdistance(nil, nextstop+1) -- Populate the array
         targsys = getlandablesystems( targsys )
         if #targsys == 0 then targsys = {system.get("Apez")} end -- In case no systems were found.
         destsys = targsys[rnd.rnd(1, #targsys)] 
         destsysname = system.name(destsys)
         destplanet = getlandable(destsys) -- pick a landable planet in the destination system
         destplanetname = destplanet:name()
         tk.msg(title[2], string.format(directions[nextstop], destplanetname, destsysname)) -- NPC telling you where to go
         misn.setDesc(string.format(misn_desc[2], destplanetname, destsysname))
         misn.osdCreate(misn_title, {misn_desc[2]:format(destplanetname, destsysname)})
         misn.markerMove( misn_marker, destsys )
      end
   end
   inspace = false
end

-- Only gets landable systems
function getlandablesystems( systems )
   t = {}
   for k,v in ipairs(systems) do
      for k,p in ipairs(v:planets()) do
         if p:services()["inhabited"] and p:canLand() then
            t[#t+1] = v
            break
         end
      end
   end
   return t
end

function takeoff()
   inspace = true
end

function enter()
   if harrassmsg then
      hook.timer(3000, "harrassme")
      harrassmsg = false
   else
   end
end

function harrassme()
   tk.msg(title[3], harrass_msg)
end

function abort ()
   if inspace then
      tk.msg(msg_abortTitle, msg_abort_space)
   else
      tk.msg(msg_abortTitle, msg_abort_landed)
   end
   misn.cargoJet(carg_id)
   misn.finish(true)
end
