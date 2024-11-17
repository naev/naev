--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Space Family">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>None</location>
 <notes>
  <done_evt name="Shipwreck">If you make the mistake to help Harrus</done_evt>
 </notes>
</mission>
--]]
--[[
   This is the mission part of the shipwrecked Space Family mission, started from a random event.
   See dat/events/neutral/shipwreck
--]]
local fmt = require "format"
local lmisn = require "lmisn"
local der = require "common.derelict"
local vn = require "vn"
local vntk = require "vntk"

local reward = 500e3
local getlandable, getlandablesystems -- Forward-declared functions

local directions = {}
directions[1] = _([["I know just the place," Harrus tells you. "Take us to planet {pnt} in the {sys} system. I'm sure a man of my calibre can find everything he needs there. Captain, please notify me when we arrive." With that, Harrus turns and rejoins his family. The kids seem in the process of redecorating (if not wrecking) your quarters, and despite the apologetic glance the woman gives you you can't help but wonder if you did the right thing responding to that SOS.]])
directions[2] = _([[Harrus steps out of your ship and takes a look around the spaceport you docked at. "No, no. This won't do at all," he says disapprovingly. "This place is a mess! Look at the dust and grime!" He rounds on you. "How are we supposed to make a decent living in a dump like this? You've brought us to the wrong place altogether. I must say I'm disappointed. I demand you take us away from this abysmal hole this minute! Let's see... Yes, {pnt} in {sys} will do. At least they're civilized there!"
    You attempt to remind Harrus that it was in fact he who asked you to take him to this system in the first place, and that the spaceport is hardly a representation of the entire world, but the man doesn't want to hear it. He stalks back into your ship without another word, leaving you annoyed and frustrated. Harrus's wife worriedly peeks around the corner of the hatch, a look of sympathy etched on her face.
    You heave a sigh, and proceed to the registration desk to get the docking formalities out of the way.]])
directions[3] = _([["The sky! Have you LOOKED at it?"
    Harrus rounds on you with a furious expression. Your keen understanding of human body language tells you he isn't happy. You thought he might be satisfied with the state of the spacedock, since it's kept in prime condition, and indeed he was. That changed as soon as he looked up.
    "It's com-plete-ly the wrong colour!" Harrus fumes. "It's a mockery of our standards of living, and it's right there overhead! Do you want my children to grow up believing the sky is supposed to look like, like... like THAT?" Harrus again looks up at the heavens that offend him so. "No, Captain, my patience is at an end. I expect you to take me and my family to {pnt} in the {sys} system. We've got relatives there who will take us in. I will waste my time with this pointless endeavour no longer!"
    Before you get a chance at making a snappy retort, Harrus storms back to his (your) quarters, leaving you to either vent your anger on his wife, who is hovering nearby, or keep it to yourself. Since the poor woman has done nothing wrong, you grimly return to the bridge.]])

function create ()
   -- Note: this mission does not make any system claims.
   misn.accept() -- You boarded their ship, now you're stuck with them.
   misn.setTitle( _("The Space Family") )
   misn.setReward( _("A clear conscience.") )
   misn.setDesc( _("A shipwrecked space family has enlisted your aid. Can you take them to safety?") )

   mem.harrassmsg = true

   -- First stop; subsequent stops will be handled in the land function
   mem.nextstop = 1
   mem.targsys = lmisn.getSysAtDistance(nil, 3) -- Populate the array
   mem.targsys = getlandablesystems( mem.targsys )
   if #mem.targsys == 0 then mem.targsys = {system.get("Apez")} end -- In case no systems were found.
   mem.destsys = mem.targsys[rnd.rnd(1, #mem.targsys)]
   mem.destplanet = getlandable(mem.destsys) -- pick a landable planet in the destination system

   -- Intro text, player meets family
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_([[The airlock opens, and you are greeted by a nervous-looking man, a shy woman, and three neurotic children.
"Thank god you are here," the man says. "I don't know how much longer we could've held out. They left us for dead, you know. No fuel, no food, and only auxiliary power to sustain us." He then begins to incoherently tell you how much his family has suffered in the past few periods, but you cut him short, not willing to put up with his endless babbling."]]))
   vn.na(_([[With a few to-the-point questions, you learn that the man's name is Harrus, and that he and his wife and children live, or at least used to live, aboard their trading vessel. "It was a good life, you know," Harrus tells you. "You get to see the galaxy, meet people, and see planets, and all that while working from home because, haha, you take your home with you!"
You can't help but glance at Harrus's kids, who have begun enthusiastically stampeding through your ship, pressing any buttons low enough for them to reach, despite their mother's hopeless attempts to keep them under control.]]))
   vn.na(fmt.f(_([[Harrus is about to launch into another anecdote about his life as a trader, but you manage to forestall him. You soon learn that his family's lifestyle has come to an abrupt change at the hands of a minor gang of pirates. Though the {plt} had some weaponry and shielding systems, the attackers were too much for a single cargo ship.
"I never thought it would end like this," Harrus sighs. "I mean, I knew space was dangerous, but I stayed clear of the unsafe areas. Stuck to the patrolled lanes. Didn't take any risks. I've got a family, you know."]]),
      {plt=_("August")}))
   vn.na(_([[Then Harrus brightens up, apparently putting his recent misfortune behind him in the blink of an eye. "Everything's going to be fine now," he says cheerfully. "We've been rescued, and all we need now is for you to take us to a suitable world where we can build a new life."
Without further ado, and without so much as formally asking for the favour, Harrus and his family proceed onto your ship and install themselves into your living quarters. They do not seem about to leave.]]))
   vn.na(fmt.f(directions[mem.nextstop], {pnt=mem.destplanet, sys=mem.destsys})) -- NPC telling you where to go
   vn.run()

   local c = commodity.new( N_("Space Family"), N_("A family who you rescued in space.") )
   mem.carg_id = misn.cargoAdd( c, 0 )

   misn.osdCreate(_("The Space Family"), {
      fmt.f(_("Take the space family to {pnt} in the {sys} system"), {pnt=mem.destplanet, sys=mem.destsys}),
   })
   mem.misn_marker = misn.markerAdd( mem.destplanet, "low" )

   -- Force unboard
   player.unboard()

   hook.land("land")
   hook.enter("enter")
end

-- Given a system, return the first landable planet found, or nil if none are landable (shouldn't happen in this script)
function getlandable(sys)
   for a, b in pairs(sys:spobs()) do
      if b:services()["inhabited"] and b:canLand() then
         return b
      end
   end
   return nil
end

function land()
   if spob.cur() == mem.destplanet then -- We've arrived!
      if mem.nextstop >= 3 then -- This is the last stop
         vn.clear()
         vn.scene()
         vn.transition()
         vn.na(_([[You land at your final stop in your quest to take the space family home, and not a moment too soon, for both you and Harrus. Harrus stomps off your ship without so much as a farewell, his wife and children in tow, and you are just as happy to see them gone.
    Surveying your now deserted quarters, you are appalled at how much damage the temporary inhabitants have managed to do along the way. You console yourself with the thought that at least you'll have something to do during the dull periods in hyperspace and turn to tend to your ships needs, when your eye falls on a small box that you don't remember seeing here before.]]))
         vn.sfxVictory()
         vn.func( function ()
            player.pay( reward )
         end )
         vn.na(_([[Inside the box, you find a sum of credits and a note written in neat, feminine handwriting that says, "Sorry for the trouble."]]).."\n\n"..fmt.reward(reward) ) -- Final message

         vn.run()

         der.addMiscLog(fmt.f(_([[You rescued a bad-tempered man and his family who were stranded aboard their ship. After a lot of annoying complaints, the man and his family finally left your ship on {pnt} ({sys}), the man's wife leaving a generous payment for the trouble.]]), {pnt=spob.cur(), sys=system.cur()}))

         misn.finish(true)
      else
         mem.nextstop = mem.nextstop + 1
         mem.targsys = lmisn.getSysAtDistance(nil, mem.nextstop+1) -- Populate the array
         mem.targsys = getlandablesystems( mem.targsys )
         if #mem.targsys == 0 then mem.targsys = {system.get("Apez")} end -- In case no systems were found.
         mem.destsys = mem.targsys[rnd.rnd(1, #mem.targsys)]
         mem.destplanet = getlandable(mem.destsys) -- pick a landable planet in the destination system
         vntk.msg(_("Next stop"), fmt.f(directions[mem.nextstop], {pnt=mem.destplanet, sys=mem.destsys})) -- NPC telling you where to go
         misn.osdCreate(_("The Space Family"), {
            fmt.f(_("Take the space family to {pnt} in the {sys} system"), {pnt=mem.destplanet, sys=mem.destsys})})
         misn.markerMove( mem.misn_marker, mem.destplanet )
      end
   end
end

-- Only gets landable systems
function getlandablesystems( systems )
   local t = {}
   for _k1,v in ipairs(systems) do
      for _k2,p in ipairs(v:spobs()) do
         if p:services()["inhabited"] and p:canLand() then
            t[#t+1] = v
            break
         end
      end
   end
   return t
end

function enter()
   if mem.harrassmsg then
      hook.timer(3.0, "harrassme")
      mem.harrassmsg = false
   end
end

function harrassme()
   vn.clear()
   vn.scene()
   vn.transition()
   vn.na(_([[You are going over a routine navigation check when Harrus enters your cabin unannounced. He seems to have recovered from his distressed state, and now radiates confidence.
"Captain," he says to you. "I hope I don't have to remind you that we must get to our destination as soon as possible. I have a wife and children to think of and frankly I find your, ah, facilities a bit lacking."]]))
   vn.na(_([[You consider ordering Harrus off your bridge, but he doesn't seem the kind of man to back off, so the only thing you would accomplish is to sour the mood on your ship. You inform Harrus that you're making every effort to get his family to a safe haven, which seems to satisfy him. Finally alone again, you take a moment to subside before completing that check.]]))
   vn.run()
end

function abort ()
   if not player.isLanded() then
      vntk.msg(_("A parting of ways"), _([[You unceremoniously shove your passengers out of the airlock and into the coldness of space. You're done playing taxi; it's time to get back to important things!]]))

      der.addMiscLog(fmt.f(_([[You rescued a bad-tempered man and his family who were stranded aboard their ship. After a lot of annoying complaints, you dumped the family out of the airlock in {sys}!]]), {sys=system.cur()}))
   else
      vntk.msg(_("A parting of ways"), _([[You unceremoniously shove your passengers out of the airlock, leaving them to their fate on this planet. You're done playing taxi; it's time to get back to important things!]]))

      der.addMiscLog(fmt.f(_([[You rescued a bad-tempered man and his family who were stranded aboard their ship. After a lot of annoying complaints, you turfed the family out on {pnt} ({sys})!]]), {pnt=spob.cur(), sys=system.cur()}))
   end
   misn.cargoJet(mem.carg_id)
   misn.finish(true)
end
