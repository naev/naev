--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Baroncomm_baron">
 <unique />
 <location>enter</location>
 <chance>4</chance>
 <cond>
   if var.peek("baron_hated") or
      player.misnDone("Baron") or
      player.misnActive("Baron") then
      return false
   end
   local sf = system.cur():faction()
   if not inlist( {
      faction.get("Empire"),
      faction.get("Dvaered"),
      faction.get("Sirius"),
   }, sf ) then
      return false
   end
   if player.wealth() &lt; 1e6 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <notes>
  <campaign>Baron Sauterfeldt</campaign>
  <tier>2</tier>
 </notes>
</event>
--]]
--[[
   Comm Event for the Baron mission string
--]]
local vn = require "vn"
local ccomm = require "common.comm"
local fmt = require "format"
local neu = require "common.neutral"

local hyena
local pnt, sys1 = spob.getS("Varia")
local sys2 = system.get("Ingot")

function create ()
   -- Inclusive claims, so not an issue they overlap with the mission itself
   if not evt.claim( {sys1, sys2}, true ) then
      evt.finish()
   end

   hyena = pilot.add( "Hyena", "Independent" )

   hook.pilot(hyena, "jump", "finish")
   hook.pilot(hyena, "death", "finish")
   hook.land("finish")
   hook.jumpout("finish")

   hook.timer( 3.0, "hailme" )
end

-- Make the ship hail the player
function hailme()
   hyena:hailPlayer()
   hook.pilot(hyena, "hail", "hail")
end

-- Triggered when the player hails the ship
function hail()
   local accepted, refused

   vn.clear()
   vn.scene()
   local plt = ccomm.newCharacter( vn, hyena )
   vn.transition()

   vn.na(_([[Your viewscreen flashes to life. You're greeted by a nondescript pilot who doesn't seem to be affiliated with anyone you know.]]))
   plt(_([["Hello there! I represent a man by the name of Baron Sauterfeldt. You may have heard of him in your travels? No? Well, I suppose you can't have it all. My employer is a moderately influential man, you see, and... But no, I'll not bore you with the details. The bottom line is, Lord Sauterfeldt is looking for hired help, and you seem like the sort he needs, judging by your ship."]]))
   plt(_([[You inquire what it is exactly this Mr. Sauterfeldt needs from you. "Oh, nothing too terribly invasive, I assure you. His Lordship currently needs a courier, nothing more. Erm, well, a courier who can't be traced back to him, if you understand what I mean. So what do you think? Sound like a suitable job for you? The pay is good, I can assure you that!"]]))
   vn.na(_([[You pause for a moment before responding to this sudden offer. It's not everyday that people come to you with work instead of you looking for it, but then again this job sounds like it could get you in trouble with the authorities. What will you do?]]))
   vn.menu{
      {_("Accept the job"), "accept"},
      {_("Politely decline"), "decline"},
      {_("Angrily refuse"), "refuse"},
   }

   vn.label("decline")
   plt(_([["Oh. Oh well, too bad. I'll just try to find someone who will take the job, then. Sorry for taking up your time. See you around!"]]))
   vn.done()

   vn.label("refuse")
   plt(_([[The pilot frowns. "I see I misjudged you. I thought for sure you would be more open-minded. Get out of my sight and never show your face to me again! You are clearly useless to my employer."]]))
   vn.func( function ()
      refused = true
      var.push("baron_hated", true)
      neu.addMiscLog( _([[You were offered a sketchy-looking job by a nondescript pilot, but you angrily refused to accept the job. It seems whoever the pilot worked for won't be contacting you again.]]) )
   end )
   vn.done()

   vn.label("accept")
   vn.func( function () accepted=true end )
   plt(fmt.f(_([["Oh, that's great! Okay, here's what Baron Sauterfeldt needs you to do. You should fly to the Dvaered world {pnt}. There's an art museum dedicated to one of the greatest Warlords in recent Dvaered history. I forget his name. Drovan or something? Durvan? Uh, anyway. This museum has a holopainting of the Warlord and his military entourage. His Lordship really wants this piece of art, but the museum has refused to sell it to him. So, we've sent agents to... appropriate... the holopainting."]]),
      {pnt=pnt}))
   vn.na(_([[You raise an eyebrow, but the pilot on the other end seems to be oblivious to the gesture.]]))
   plt(fmt.f(_([["So, right, you're going to {pnt} to meet with our agents. You should find them in the spaceport bar. They'll get the item onto your ship, and you'll transport it out of Dvaered space. All quiet-like of course. No need for the authorities to know until you're long gone. Don't worry, our people are pros. It'll go off without a hitch, trust me."]]),
      {pnt=pnt}))
   vn.na(_([[You smirk at that. You know from experience that things seldom 'go off without a hitch', and this particular plan doesn't seem to be all that well thought out. Still, it doesn't seem like you'll be in a lot of danger. If things go south, they'll go south well before you are even in the picture. And even if the authorities somehow get on your case, you'll only have to deal with the planetary police, not the entirety of House Dvaered.]]))
   vn.na(_([[You ask the Baron's messenger where this holopainting needs to be delivered.]]))
   plt(fmt.f(_([["His Lordship will be taking your delivery in the {sys} system, aboard his ship, the Pinnacle," he replies. "Once you arrive with the holopainting onboard your ship, hail the Pinnacle and ask for docking permission. They'll know who you are, so you should be allowed to dock. You'll be paid on delivery. Any questions?"]]),
      {sys=sys2}))
   vn.na(fmt.f(_([[You indicate that you know what to do, then cut the connection. Next stop: planet {pnt}.]]),
      {pnt=pnt}))
   vn.run()

   player.commClose()
   if accepted then
      naev.missionStart("Baron")
      evt.finish(false) -- Can't set to true in case the mission gets failed or whatever, so it can be started again
   elseif refused then
      evt.finish(true)
   end
   evt.finish(false)
end

function finish ()
   evt.finish(false)
end
