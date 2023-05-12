--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Baron">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>None</location>
 <notes>
  <done_evt name="Baroncomm_baron">Triggers</done_evt>
  <campaign>Baron Sauterfeldt</campaign>
 </notes>
</mission>
--]]
--[[
-- This is the first mission in the baron string.
--]]

local fmt = require "format"
local portrait = require "portrait"
local baron = require "common.baron"
local neu = require "common.neutral"

local sys1 = system.get("Darkstone")
local sys2 = system.get("Ingot")
local pnt = spob.get("Varia")

local credits = baron.rewards.baron
local pinnacle, vendetta1, vendetta2 -- Non-persistent state


function create ()
   local missys = {system.get("Darkstone")}
   if not misn.claim(missys) then
      abort()
   end

   tk.msg(_("You've been scouted!"), _([[Your viewscreen flashes to life. You're greeted by a nondescript pilot who doesn't seem to be affiliated with anyone you know.]]))
   tk.msg(_("You've been scouted!"), _([["Hello there! I represent a man by the name of Baron Sauterfeldt. You may have heard of him in your travels? No? Well, I suppose you can't have it all. My employer is a moderately influential man, you see, and... But no, I'll not bore you with the details. The bottom line is, Lord Sauterfeldt is looking for hired help, and you seem like the sort he needs, judging by your ship."]]))
   tk.msg(_("You've been scouted!"), _([[You inquire what it is exactly this Mr. Sauterfeldt needs from you. "Oh, nothing too terribly invasive, I assure you. His Lordship currently needs a courier, nothing more. Erm, well, a courier who can't be traced back to him, if you understand what I mean. So what do you think? Sound like a suitable job for you? The pay is good, I can assure you that!"]]))
   local c = tk.choice(_("You've been scouted!"), _([[You pause for a moment before responding to this sudden offer. It's not everyday that people come to you with work instead of you looking for it, but then again this job sounds like it could get you in trouble with the authorities. What will you do?]]), _("Accept the job"), _("Politely decline"), _("Angrily refuse"))
   if c == 1 then
      accept()
   elseif c == 2 then
      tk.msg(_("Never the wiser"), _([["Oh. Oh well, too bad. I'll just try to find someone who will take the job, then. Sorry for taking up your time. See you around!"]]))
      abort()
   else
      tk.msg(_("Well, then..."), _([[The pilot frowns. "I see I misjudged you. I thought for sure you would be more open-minded. Get out of my sight and never show your face to me again! You are clearly useless to my employer."]]))
      var.push("baron_hated", true)
      neu.addMiscLog( _([[You were offered a sketchy-looking job by a nondescript pilot, but you angrily refused to accept the job. It seems whoever the pilot worked for won't be contacting you again.]]) )
      abort()
   end
end

function accept()
   tk.msg(_("A risky retrieval"), fmt.f(_([["Oh, that's great! Okay, here's what Baron Sauterfeldt needs you to do. You should fly to the Dvaered world {pnt}. There's an art museum dedicated to one of the greatest Warlords in recent Dvaered history. I forget his name. Drovan or something? Durvan? Uh, anyway. This museum has a holopainting of the Warlord and his military entourage. His Lordship really wants this piece of art, but the museum has refused to sell it to him. So, we've sent agents to... appropriate... the holopainting."]]), {pnt=pnt}))
   tk.msg(_("A risky retrieval"), fmt.f(_([[You raise an eyebrow, but the pilot on the other end seems to be oblivious to the gesture. "So, right, you're going to {pnt} to meet with our agents. You should find them in the spaceport bar. They'll get the item onto your ship, and you'll transport it out of Dvaered space. All quiet-like of course. No need for the authorities to know until you're long gone. Don't worry, our people are pros. It'll go off without a hitch, trust me."]]), {pnt=pnt}))
   tk.msg(_("A risky retrieval"), _([[You smirk at that. You know from experience that things seldom 'go off without a hitch', and this particular plan doesn't seem to be all that well thought out. Still, it doesn't seem like you'll be in a lot of danger. If things go south, they'll go south well before you are even in the picture. And even if the authorities somehow get on your case, you'll only have to deal with the planetary police, not the entirety of House Dvaered.]]))
   tk.msg(_("A risky retrieval"), fmt.f(_([[You ask the Baron's messenger where this holopainting needs to be delivered. "His Lordship will be taking your delivery in the {sys} system, aboard his ship, the Pinnacle," he replies. "Once you arrive with the holopainting onboard your ship, hail the Pinnacle and ask for docking permission. They'll know who you are, so you should be allowed to dock. You'll be paid on delivery. Any questions?" You indicate that you know what to do, then cut the connection. Next stop: planet {pnt}.]]), {sys=sys2, pnt=pnt}))

   misn.accept()

   misn.setTitle(_("Baron"))
   misn.setReward(_("A tidy sum of money"))
   misn.setDesc(_("You've been hired as a courier for one Baron Sauterfeldt. Your job is to transport a holopainting from a Dvaered world to the Baron's ship."))

   misn.osdCreate(_("Baron"), {
      fmt.f(_("Fly to the {sys} system and land on planet {pnt}"), {sys=sys1, pnt=pnt}),
      fmt.f(_("Fly to the {sys} system and dock with (board) Gauss Pinnacle"), {sys=sys2}),
   })

   mem.misn_marker = misn.markerAdd( pnt, "low" )

   mem.talked = false
   mem.stopping = false

   hook.land("land")
   hook.enter("jumpin")
   hook.takeoff("takeoff")
end

function land()
   if spob.cur() == pnt and not mem.talked then
      local npc_desc = _("These must be the 'agents' hired by this Baron Sauterfeldt. They look shifty. Why must people involved in underhanded business always look shifty?")
      mem.thief1 = misn.npcAdd("talkthieves", _("Sauterfeldt's agents"), portrait.get("Pirate"), npc_desc)
      mem.thief2 = misn.npcAdd("talkthieves", _("Sauterfeldt's agents"), portrait.get("Pirate"), npc_desc)
      mem.thief3 = misn.npcAdd("talkthieves", _("Sauterfeldt's agents"), portrait.get("Pirate"), npc_desc)
   end
end

function jumpin()
   if mem.talked and system.cur() == sys2 then
      local homepos = spob.get("Ulios"):pos()
      pinnacle = pilot.add( "Proteron Gauss", "Proteron", homepos + vec2.new(-400,-400), _("Pinnacle"), {ai="trader"} )
      pinnacle:setFaction("Independent")
      pinnacle:setInvincible(true)
      pinnacle:control()
      pinnacle:setHilight(true)
      pinnacle:moveto(homepos + vec2.new( 400, -400), false)
      mem.idlehook = hook.pilot(pinnacle, "idle", "idle")
      hook.pilot(pinnacle, "hail", "hail")
   end
end

function idle()
   local homepos = spob.get("Ulios"):pos()
   pinnacle:moveto(homepos + vec2.new( 400,  400), false)
   pinnacle:moveto(homepos + vec2.new(-400,  400), false)
   pinnacle:moveto(homepos + vec2.new(-400, -400), false)
   pinnacle:moveto(homepos + vec2.new( 400, -400), false)
end

function hail()
   if mem.talked then
      tk.msg(_("Green light for docking"), _([[Your comm is answered by a communications officer on the bridge of the Pinnacle. You tell her you've got a delivery for the baron. She runs a few checks on a console off the screen, then tells you you've been cleared for docking and that the Pinnacle will be brought to a halt.]]))
      pinnacle:taskClear()
      pinnacle:brake()
      pinnacle:setActiveBoard(true)
      hook.pilot(pinnacle, "board", "board")
      hook.rm(mem.idlehook)
      player.commClose()
   end
end

function board()
   tk.msg(_("No bad deed goes unrewarded"), _([[When you arrive at your ship's airlock, the chest containing the Dvaered holopainting is already being carted onto the Pinnacle by a pair of crewmen. "You'll be wanting your reward, eh? Come along", one of them yells at you. They both chuckle and head off down the corridor.]]))
   tk.msg(_("No bad deed goes unrewarded"), _([[You follow the crewmen as they push the cart through the main corridor of the ship. Soon you arrive at a door leading to a large, luxurious compartment. You can tell at a glance that these are Baron Sauterfeldt's personal quarters. The Baron himself is present. He is a large man, wearing a tailored suit that manages to make him look stately rather than pompous, a monocle, and several rings on each finger. In a word, the Baron has a taste for the extravagant.]]))
   tk.msg(_("No bad deed goes unrewarded"), _([["Ah, my holopainting," he coos as the chest is being carried into his quarters. "At last, I've been waiting forever." The Baron does not seem to be aware of your presence at all. He continues to fuss over the holopainting even as his crewman strip away the chest and lift the frame up to the wall.]]))
   tk.msg(_("The Baron's Quarters"), _([[You look around his quarters. All sorts of exotic blades and other "art" works adorn his room, along with tapestries and various other holopaintings. You notice a bowl atop a velvet rug with "Fluffles" on it. Hanging above it seems to be a precariously balanced ancient blade.]]))
   tk.msg(_("The Baron's Quarters"), _([[The crewmen finally unpack the holopainting. You glance at the three-dimensional depiction of a Dvaered warlord, who seems to be discussing strategy with his staff. Unfortunately you don't seem to be able to appreciate Dvaered art, and you lose interest almost right away.]]))
   tk.msg(_("The Baron's Quarters"), _([[You cough to get the Baron's attention. He looks up, clearly displeased at the disturbance, then notices you for the first time. "Ah, of course," he grunts. "I suppose you must be paid for your service. Here, have some credits. Now leave me alone. I have art to admire." The Baron tosses you a couple of credit chips, and then you are once again air to him. You are left with little choice but to return to your ship, undock, and be on your way.]]))
   player.pay( credits )
   player.refuel()
   player.unboard()
   pinnacle:control(false)
   pinnacle:changeAI("flee")
   pinnacle:setHilight(false)
   pinnacle:setActiveBoard(false)
   baron.addLog( _([[You helped some selfish baron steal a Dvaered holopainting and were paid a measly sum of credits.]]) )
   misn.finish(true)
end

function talkthieves()
   tk.msg(_("Cloak and dagger"), _([[The three shifty-looking patrons regard you with apprehension as you approach their table. Clearly they don't know who their contact is supposed to be. You decide to be discreet, asking them if they've ever heard of a certain Sauterfeldt. Upon hearing this, the trio visibly relaxes. They tell you that indeed they know the man you speak of, and that they have something of his in their possession. Things proceed smoothly from that point, and several hectoseconds later you are back at your ship, preparing it for takeoff while you wait for the agents to bring you your cargo.]]))
   tk.msg(_("Cloak and dagger"), _([[You're halfway through your pre-flight security checks when the three appear in your docking hangar. They have a cart with them on which sits a rectangular chest as tall as a man and as long as two. Clearly this holopainting is fairly sizeable. As you watch them from your bridge's viewport, you can't help but wonder how they managed to get something that big out of a Dvaered museum unnoticed.]]))
   tk.msg(_("Cloak and dagger"), _([[As it turns out, they didn't. They have only just reached the docking bridge leading into your ship when several armed Dvaered security forces come bursting into the docking hangar. They spot the three agents and immediately open fire. One of them goes down, the others hurriedly push the crate over the bridge towards your ship. Despite the drastic change in the situation, you have time to note that the Dvaered seem more interested in punishing the criminals than retrieving their possession intact.]]))
   tk.msg(_("Cloak and dagger"), _([[The second agent is caught by a Dvaered bullet, and topples off the docking bridge and into the abyss below. The third manages to get the cart with the chest into your airlock before catching a round with his chest as well. As the Dvaered near your ship, you seal the airlock, fire up your engines and punch it out of the docking hangar.]]))

   misn.npcRm(mem.thief1)
   misn.npcRm(mem.thief2)
   misn.npcRm(mem.thief3)

   mem.talked = true
   local c = commodity.new( N_("The Baron's holopainting"), N_("A rectangular chest containing a holopainting.") )
   mem.carg_id = misn.cargoAdd(c, 0)
   c:illegalto( "Dvaered" )

   misn.osdActive(2)
   misn.markerMove( mem.misn_marker, sys2 )

   player.takeoff()
end

function takeoff()
   if mem.talked and system.cur() == sys1 then
      hook.timer( 3, "takeoff_delay" )
   end
end

function takeoff_delay ()
   vendetta1 = pilot.add( "Dvaered Vendetta", "Dvaered", pnt, _("Dvaered Police Vendetta"), {ai="dvaered_norun"} )
   vendetta2 = pilot.add( "Dvaered Vendetta", "Dvaered", pnt, _("Dvaered Police Vendetta"), {ai="dvaered_norun"} )
   for k,v in ipairs{vendetta1, vendetta2} do
      v:setHostile(true)
   end
   vendetta1:broadcast(fmt.f(_("All troops, engage {ship_type} {ship_name}! It has broken {pnt} law!"), {ship_type=_(player.pilot():ship():baseType()), ship_name=player.ship(), pnt=pnt}), true)
end

function abort()
   misn.finish(false)
end
