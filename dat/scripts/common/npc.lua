--[[
Helper library containing some common npc stuff to be shared
--]]
local fmt = require "format"
local tut = require "common.tutorial"

local npc = {}

function npc.combine_cond( msglist )
   local msg_combined = {}
   for k,msg in ipairs( msglist ) do
      if msg[1]() then
         table.insert( msg_combined, msg[2] )
      end
   end
   return msg_combined
end

function npc.combine_desc( desclist, tags )
   local descriptions = tcopy( desclist["generic"] )
   for t,v in pairs(tags) do
      local dl = desclist[t]
      if dl then
         for k,d in ipairs(dl) do
            table.insert( descriptions, d )
         end
      end
   end
   return descriptions
end

function npc.test_misnHint( misnname )
   return function ()
      return not (player.misnDone(misnname) or player.misnActive(misnname))
   end
end

function npc.test_evtHint( evtname )
   return function ()
      return not (player.evtDone(evtname) or player.evtActive(evtname))
   end
end

function npc.test_misnDone( misnname )
   return function ()
      return player.misnDone(misnname)
   end
end

function npc.test_evtDone( evtname )
   return function ()
      return player.evtDone(evtname)
   end
end

--[[
Lore messages. These come in general and factional varieties.
General lore messages will be said by non-faction NPCs, OR by faction NPCs if
they have no factional text to say. When adding factional text, make sure to
add it to the table of the appropriate faction. Does your faction not have a
table? Then just add it. The script will find and use it if it exists. Make
sure you spell the faction name exactly the same as in faction.xml though!
--]]
npc.msg_lore = {
   _([["I heard the nebula is haunted! My uncle Bobby told me he saw one of the ghost ships himself over in Arandon!"]]),
   _([["I don't believe in those nebula ghost stories. The people who talk about it are just trying to scare you."]]),
   _([["I heard the Soromid lost their homeworld Sorom in the Incident. Its corpse can still be found in Basel."]]),
   _([["The Soromid fly organic ships! I heard some of their ships can even repair themselves. That's so weird."]]),
   _([["Have you seen that ship the Emperor lives on? It's huge! But if you ask me, it looks a bit like a… No, never mind."]]),
   _([["I wonder why the Sirii are all so devout? I heard they have these special priesty people walking around. I wonder what's so special about them."]]),
   _([["They say Eduard Manual Goddard is drifting in space somewhere, entombed amidst a cache of his inventions! What I wouldn't give to rummage through there…"]]),
   _([["Ah man, I lost all my money on Totoran. I love the fights they stage there, but the guys I bet on always lose. What am I doing wrong?"]]),
   _([["Don't try to fly into the inner nebula. I've known people who tried, and none of them came back."]]),
   _([["Have you heard of Captain T. Practice? He's amazing, I'm his biggest fan!"]]),
   _([["I wouldn't travel north from Alteris if I were you, unless you're a good fighter! That area of space has really gone down the drain since the Incident."]]),
   _([["Sometimes I look at the stars and wonder… are we the only sentient species in the universe?"]]),
   _([["Hey, you ever wonder why we're here?" You respond that it's one of the great mysteries of the universe. Why are we here? Are we the product of some cosmic coincidence or is there some great cosmic plan for us? You don't know, but it sometimes keeps you up at night. As you say this, the citizen stares at you incredulously. "What?? No, I mean why are we in here, in this bar?"]]),
   _([["Life is so boring here. I would love to go gamble with all the famous people at Minerva Station."]]),
}

--[[
Gameplay tip messages.
ALL NPCs have a chance to say one of these lines instead of a lore message.
So, make sure the tips are always faction neutral.
--]]
npc.msg_tip = {
   _([["I heard you can set your weapons to only fire when your target is in range, or just let them fire when you pull the trigger. Sounds handy!"]]),
   function () return fmt.f( _([["Did you know that if a planet doesn't like you, you can often bribe the spaceport operators and land anyway? Just hail the planet with {hailkey}, and click the bribe button! Careful though, it doesn't always work."]]), {hailkey=tut.getKey("hail")} ) end,
   _([["Many factions offer rehabilitation programs to criminals through the mission computer, giving them a chance to get back into their good graces. It can get really expensive for serious offenders though!"]]),
   _([["These new-fangled missile systems! You can't even fire them unless you get a target lock first! But the same thing goes for your opponents. You can actually make it harder for them to lock on to your ship by equipping scramblers or jammers. Scout class ships are also harder to target."]]),
   _([["You know how you can't change your ship or your equipment on some planets? Well, it seems you need an outfitter to change equipment, and a shipyard to change ships! Bet you didn't know that."]]),
   _([["Are you trading commodities? You can hold down #bctrl#0 to buy 50 of them at a time, and #bshift#0 to buy 100. And if you press them both at once, you can buy 500 at a time! You can actually do that with outfits too, but why would you want to buy 50 laser cannons?"]]),
   _([["If you're on a mission you just can't beat, you can open the information panel and abort the mission. There's no penalty for doing it, so don't hesitate to try the mission again later."]]),
   _([["Some weapons have a different effect on shields than they do on armour. Keep that in mind when equipping your ship."]]),
   _([["Afterburners can speed you up a lot, but when they get hot they don't work as well anymore. Don't use them carelessly!"]]),
   _([["There are passive outfits and active outfits. The passive ones modify your ship continuously, but the active ones only work if you turn them on. You usually can't keep an active outfit on all the time, so you need to be careful only to use it when you need it."]]),
   _([["If you're new to the galaxy, I recommend you buy a map or two. It can make exploration a bit easier."]]),
   _([["Scramblers and jammers make it harder for missiles to track you. They can be very handy if your enemies use missiles."]]),
   function () return fmt.f( _([["If you're having trouble with overheating weapons or outfits, you can either press {cooldownkey} or double-tap {reversekey} to put your ship into Active Cooldown; that'll dissipate all heat from your ship and also refill your rocket ammunition. Careful though, your energy and shields won't recharge while you do it!"]]), {cooldownkey=tut.getKey("cooldown"), reversekey=tut.getKey("reverse")} ) end,
   _([["If you're having trouble shooting other ships face on, try outfitting with turrets or use an afterburner to avoid them entirely!"]]),
   _([["You know how time speeds up when Autonav is on, but then goes back to normal when enemies are around? Turns out you can't disable the return to normal speed entirely, but you can control what amount of danger triggers it. Really handy if you want to ignore enemies that aren't actually hitting you."]]),
   _([["Flying bigger ships is awesome, but it's a bit tougher than flying smaller ships. There's so much more you have to do for the same actions, time just seems to fly by faster. I guess the upside of that is that you don't notice how slow your ship is as much."]]),
   _([["I know it can be tempting to fly the big and powerful ships, but don't underestimate smaller ones! Given their simpler designs and lesser crew size, you have a lot more time to react with a smaller vessel. Some are even so simple to pilot that time seems to slow down all around you!"]]),
   _([["Rich folk will pay extra to go on an offworld sightseeing tour in a luxury yacht. I don't get it personally; it's all the same no matter what ship you're in."]]),
   _([["Different ships should be built and piloted differently. One of the hardest lessons I learned as a pilot was to stop worrying so much about the damage my ship was taking in battle while piloting a large ship. These ships are too slow for dodging, not to mention so complicated that they reduce your reaction time, so you need to learn to just take the hits and focus your attention on firing back at your enemies."]]),
   _([["Remember that when you pilot a big ship, you perceive time passing a lot faster than you do when you pilot a small ship. It can be easy to forget just how slow these larger ships are when you're frantically trying to depressurize the exhaust valve while also configuring the capacitance array. In a way the slow speed of the ship becomes a pretty huge relief!"]]),
   _([["There's always an exception to the rule, but I wouldn't recommend using forward-facing weapons on larger ships. Large ships' slower turn rates aren't able to keep up with the dashing and dodging of smaller ships, and aiming is harder anyway what with how complex these ships are. Turrets are much better; they aim automatically and usually do a very good job!"]]),
   _([["Did you know that turrets' automatic tracking of targets is slowed down by cloaking? Well, now you do! Small ships majorly benefit from a scrambler or two; it makes it much easier to dodge those turrets on the larger ships."]]),
   _([["Don't forget to have your target selected. Even if you have forward-facing weapons, the weapons will swivel a bit to track your target. But it's absolutely essential for turreted weapons."]]),
   _([["Did you know that you can automatically follow pilot with Autonav? It's true! Just #bleft-click#0 the pilot to target them and then #bright-click#0 your target to follow! I like to use this feature for escort missions. It makes them a lot less tedious."]]),
   _([["The new aiming helper feature is awesome! Simply turn it on in your ship's weapons configuration and you get little guides telling you where you should aim to hit your target! I use it a lot."]]),
   _([["The '¤' symbol is the official galactic symbol for credits. Supposedly it comes from the currency symbol of an ancient Earth civilization. It's sometimes expressed with SI prefixes: 'k¤' for thousands of credits, 'M¤' for millions of credits, and so on."]]),
   _([["If you're piloting a medium ship, I'd recommend you invest in at least one turreted missile launcher. I had a close call a few decaperiods ago where a bomber nearly blew me to bits outside the range of my Laser Turrets. Luckily I just barely managed to escape to a nearby planet so I could escape the pilot. I've not had that problem ever since I equipped a turreted missile launcher."]]),
   _([["I've heard rumours that a pirate's reputations depends on flying pirate ships, but I think they only loathe peaceful honest work."]]),
   function () return fmt.f(_([["These computer symbols can be confusing sometimes! I've figured it out, though: '{F}' means friendly, '{N}' means neutral, '{H}' means hostile, '{R}' means restricted, and '{U}' means uninhabited but landable. I wish someone had told me that!"]]), {F="#F+#0", N="#N~#0", H="#H!!#0", R="#R*#0", U="#I=#0"} ) end,
   _([["Trade Lanes are the safest bet to travel around the universe. They have many patrols to keep you safe from pirates."]]),
}

--[[
Conditional messages.
--]]
npc.msg_cond = {
   -- Mission Hints
   {npc.test_misnHint("Shadowrun"), _([["Apparently there's a woman who regularly turns up on planets in and around the Klantar system. I wonder what she's looking for?"]])},
   {npc.test_misnHint("Hitman"), _([["There are often shady characters hanging out in the Alteris system. I'd stay away from there if I were you, someone might offer you a dirty kind of job!"]])},
   -- Event hints
   {npc.test_evtHint("FLF/DV Derelicts"), _([["The FLF and the Dvaered sometimes clash in Surano. If you go there, you might find something of interest… Or not."]])},
   -- Mission Completion
   {npc.test_misnDone("Nebula Satellite"), _([["Heard some reckless scientists got someone to put a satellite inside the nebula for them. I thought everyone with half a brain knew to stay out of there, but oh well."]])},
   {npc.test_misnDone("Shadow Vigil"), _([["Did you hear? There was some big incident during a diplomatic meeting between the Empire and the Dvaered. Nobody knows what exactly happened, but both diplomats died. Now both sides are accusing the other of foul play. Could get ugly."]])},
   {npc.test_misnDone("Baron"), _([["Some thieves broke into a museum on Varia and stole a holopainting! Most of the thieves were caught, but the one who carried the holopainting offworld is still at large. No leads. Damn criminals…"]])},
   {npc.test_misnDone("Destroy the FLF base!"), _([["The Dvaered scored a major victory against the FLF recently. They went into Sigur and blew the hidden base there to bits! I bet that was a serious setback for the FLF."]])},
   -- Event Completion
   {npc.test_evtDone("Animal trouble"), _([["What? You had rodents sabotage your ship? Man, you're lucky to be alive. If it had hit the wrong power line…"]])},
   {npc.test_evtDone("Naev Needs You!"), _([["What do you mean, the world ended and then the creator of the universe came and fixed it? What kind of illegal substance are you on?"]])},
}

function npc.cache ()
   -- Create a cache, unique per player
   local c = naev.cache()
   local pn = player.name()
   c.npc_msg = c.npc_msg or {}
   c.npc_msg[ pn ] = c.npc_msg[ pn ] or {}
   return c.npc_msg[ pn ]
end

return npc
