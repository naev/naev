
--[[
-- Event for creating random characters in the spaceport bar.
-- The random NPCs will tell the player things about the Naev universe in general, about their faction, or about the game itself.
--]]

include "dat/events/tutorial/tutorial-common.lua"

-- Factions which will NOT get generic texts if possible.  Factions
-- listed here not spawn generic civilian NPCs or get aftercare texts.
-- Meant for factions which are either criminal (FLF, Pirate) or unaware
-- of the main universe (Thurion, Proteron).
nongeneric_factions = { "Pirate", "FLF", "Thurion", "Proteron" }

-- Portraits.
-- When adding portraits, make sure to add them to the table of the faction they belong to.
-- Does your faction not have a table? Then just add it. The script will find and use it if it exists.
-- Make sure you spell the faction name exactly the same as in faction.xml though!
civ_port = {}
civ_port["general"] =   {"neutral/male1",
                           "neutral/female1",
                           "neutral/miner1",
                           "neutral/miner2",
                           "neutral/thief1",
                           "neutral/thief2",
                           "neutral/thief3",
                        }
civ_port["Sirius"] =    {"sirius/sirius_fyrra_f1",
                           "sirius/sirius_fyrra_f2",
                           "sirius/sirius_fyrra_m1",
                        }
civ_port["Trader"] =    {"neutral/male1",
                           "neutral/female1",
                           "neutral/thief1",
                           "neutral/thief3",
                        }
civ_port["Pirate"] =    {"pirate/pirate1",
                           "pirate/pirate2",
                           "pirate/pirate3",
                           "pirate/pirate4",
                        }
civ_name = "Civilian"

-- Civilian descriptions for the spaceport bar.
-- These descriptions will be picked at random, and may be picked multiple times in one generation.
-- Remember that any description can end up with any portrait, so don't make any assumptions
-- about the appearance of the NPC!
civ_desc = {_("This person seems to be here to relax."),
            _("There is a civilian sitting on one of the tables."),
            _("There is a civilian sitting there, looking somewhere else."),
            _("A worker sits at one of the tables, wearing a nametag saying \"Go away\"."),
            _("A civilian sits at the bar, seemingly serious about the cocktails on offer."),
            _("A civilian wearing a shirt saying: \"Ask me about Jaegnhild\""),
            _("There is a civilian sitting in the corner."),
            _("A civilian feverishly concentrating on a fluorescing drink."),
            _("A civilian drinking alone."),
            _("This person seems friendly enough."),
            _("A civilian sitting at the bar."),
            _("This person is idly browsing a news terminal."),
            _("A worker sits drinks, but not working."),
            _("A worker slouched against the bar, nursing a drink."),
            _("This worker seems bored with everything but their drink."),
            }

-- Lore messages. These come in general and factional varieties.
-- General lore messages will be said by non-faction NPCs, OR by faction NPCs if they have no factional text to say.
-- When adding factional text, make sure to add it to the table of the appropriate faction.
-- Does your faction not have a table? Then just add it. The script will find and use it if it exists.
-- Make sure you spell the faction name exactly the same as in faction.xml though!
msg_lore = {}
msg_lore["general"] =      {_("I heard the nebula is haunted! My uncle Bobby told me he saw one of the ghost ships himself over in Arandon!"),
                              _("I don't believe in those nebula ghost stories. The people who talk about it are just trying to scare you."),
                              _("I heard the Soromid lost their homeworld Sorom in the Incident. Its corpse can still be found in Basel."),
                              _("The Soromid fly organic ships! I heard their ships can even heal themselves in flight. That's so weird."),
                              _("Have you seen that ship the Emperor lives on? It's huge! But if you ask me, it looks a bit like a... No, never mind."),
                              _("I wonder why the Sirii are all so devout? I heard they have these special priesty people walking around. I wonder what's so special about them."),
                              _("They say Eduard Manual Goddard is drifting in space somewhere, entombed amidst a cache of his inventions! What I wouldn't give to rummage through there..."),
                              _("Ah man, I lost all my money on Totoran. I love the fights they stage there, but the guys I bet on always lose. What am I doing wrong?"),
                              _("Don't try to fly into the inner nebula. I've known people who tried, and none of them came back."),
                              _("Have you heard of Captain T. Practice? He's amazing, I'm his biggest fan!"),
                              _("I wouldn't travel north from Alteris if I were you, unless you're a good fighter! That area of space has really gone down the drain since the Incident."),
                              _("Sometimes I look at the stars and wonder... are we the only sentient species in the universe?"),
                              _("Hey, you ever wonder why we're here?\", you say, \"It's one of life's great mysteries isn't it?\", the citizen replies, \"Why are we here? Are we the product of some cosmic coincidence or is there some great cosmic plan for us? I dunno, but it keeps me up at night\" ... \"What?? No, I mean why are we in here, in this bar?")
                           }

msg_lore["Independent"] =  {_("We're not part of any of the galactic superpowers. We can take care of ourselves!"),
                              _("Sometimes I worry that our lack of a standing military leaves us vulnerable to attack. I hope nobody will get any ideas of conquering us!"),
                           }

msg_lore["Empire"] =       {_("Things are getting worse by the cycle. What happened to the Empire? We used to be the lords and masters over the whole galaxy!"),
                              _("Did you know that House Za'lek was originally a Great Project initiated by the Empire? Well, now you do! There was also a Project Proteron, but that didn't go so well."),
                              _("The Emperor lives on a giant supercruiser in Gamma Polaris. It's said to be the biggest ship in the galaxy! I totally want one."),
                              _("I'm still waiting for my pilot license application to get through. Oh well, it's only been half a cycle, I just have to be patient."),
                              _("Between you and me, the laws the Council passes can get really ridiculous! Most planets find creative ways of ignoring them..."),
                              _("Don't pay attention to the naysayers. The Empire is still strong. Have you ever seen a Peacemaker up close? I doubt any ship fielded by any other power could stand up to one."),
                           }

msg_lore["Dvaered"] =      {_("Our Warlord is currently fighting for control over another planet. We all support him unconditionally, of course! Of course..."),
                              _("My great-great-great-grandfather fought in the Dvaered Revolts! We still have the holovids he made. I'm proud to be a Dvaered!"),
                              _("I've got lots of civilian commendations! It's important to have commendations if you're a Dvaered."),
                              _("You better not mess with House Dvaered. Our military is the largest and strongest in the galaxy. Nobody can stand up to us!"),
                              _("House Dvaered? House? The Empire is weak and useless, we don't need them anymore! I say we declare ourselves an independent faction today. What are they going to do, subjugate us? We all know how well that went last time! Ha!"),
                              _("I'm thinking about joining the military. Every time I see or hear news about those rotten FLF bastards, it makes my blood boil! They should all be pounded into space dust!"),
                              _("FLF terrorists? I'm not too worried about them. You'll see, High Command will have smoked them out of their den soon enough, and then the Frontier will be ours."),
                              _("Did you know that House Dvaered was named after a hero of the revolt? At least that's what my grandparents told me."),
                           }

msg_lore["Sirius"] =       {_("Greetings, traveler. May Sirichana's wisdom guide you as it guides me."),
                              _("I once met one of the Touched in person. Well, it wasn't really a meeting, our eyes simply met... But that instant alone was awe-inspiring."),
                              _("They say Sirichana lives and dies like any other man, but each new Sirichana is the same as the last. How is that possible?"),
                              _("My cousin was called to Mutris a cycle ago... He must be in Crater City by now. And one day, he will become one of the Touched!"),
                              _("Some people say Sirius society is unfair because our echelons are determined by birth. But even though we are different, we are all followers of Sirichana. Spiritually, we are equal."),
                              _("House Sirius is officially part of the Empire, but everyone knows that's only true on paper. The Emperor has nothing to say in these systems. We follow Sirichana, and no-one else."),
                              _("You can easily tell the different echelons apart. Every Sirian citizen and soldier wears clothing appropriate to his or her echelon."),
                              _("I hope to meet one of the Touched one day!"),
                           }

msg_lore["Soromid"] =      {_("Hello. Can I interest you in one of our galaxy famous cosmetic gene treatments? You look like you could use them..."),
                              _("Can you believe it? I was going to visit Sorom to find my roots, and then boom! It got burnt to a crisp! Even now, cycles later, I still can't believe it."),
                              _("Yes, it's true, our military ships are alive. Us normal folk don't get to own bioships though, we have to make do with synthetic constructs just like everyone else."),
                              _("Everyone knows that we Soromid altered ourselves to survive the deadly conditions on Sorom during the Great Quarantine. What you don't hear so often is that billions of us died from the therapy itself. We paid a high price for survival."),
                              _("Our cosmetic gene treatments are even safer now for non-Soromids, with a rate of survival of 99.4%!"),
                              _("We have been rebuilding and enhancing our bodies for so long, I say we've become a new species, one above human."),
                           }

msg_lore["Za'lek"] =       {_("It's not easy, dancing to those scientists' tunes. They give you the most impossible tasks! Like, where am I supposed to get a triple redundant helitron converter? Honestly."),
                              _("The Soromids? Hah! We Za'lek are the only true scientists in this galaxy."),
                              _("I don't understand why we bother sending our research results to the Empire. These simpletons can't understand the simplest formulas!"),
                              }

msg_lore["Thurion"] =      {_("Did you know that even the slightest bit of brain damage can lead to death during the upload process? That's why we're very careful to not allow our brains to be damaged, even a little."),
                              _("My father unfortunately hit his head when he was young, so he couldn't be safely uploaded. It's okay, though; he had a long and fulfilling life, for a non-uploaded human, that is."),
                              _("One great thing once you're uploaded is that you can choose to forget things you don't want to remember. My great-grandfather had a movie spoiled for him before he could watch it, so once he got uploaded, he deleted that memory and watched it with a fresh perspective. Cool, huh?"),
                              _("The best part of our lives is after we're uploaded, but that doesn't mean we lead boring lives before then. We have quite easy and satisfying biological lives before uploading."),
                              _("Being uploaded allows you to live forever, but that doesn't mean you're forced to. Any uploaded Thurion can choose to end their own life if they want, though few have chosen to do so."),
                              _("Uploading is a choice in our society. No one is forced to do it. It's just that, well, what kind of person would turn down the chance to live a second life on the network?"),
                              _("We were lucky to not get touched by the Incident. In fact, we kind of benefited from it. The nebula that resulted gave us a great cover and sealed off the Empire from us. It also got rid of those lunatics, the Proterons."),
                              _("We don't desire galactic dominance. That being said, we do want to spread our way of life to the rest of the galaxy, so that everyone can experience the joy of being uploaded."),
                              _("I think you're from the outside, aren't you? That's awesome! I've never met a foreigner before. What's it like outside the nebula?"),
                              _("We actually make occasional trips outside of the nebula, though only rarely, and we always make sure to not get discovered by the Empire."),
                              _("The Soromid have a rough history. Have you read up on it? First the Empire confined them to a deadly planet and doomed them to extinction. Then, when they overcame those odds, the Incident blew up their homeworld. The fact that they're still thriving now despite that is phenomenal, I must say."),
                           }

msg_lore["Proteron"] =     {_("Our system of government is clearly superior to all others. Nothing could be more obvious."),
                              _("The Incident really set back our plan for galactic dominance, but that was only temporary."),
                              _("We don't have time for fun and games. The whole reason we're so great is because we're more productive than any other society."),
                              _("We are superior, so of course we deserve control over the galaxy. It's our destiny."),
                              _("The Empire is weak, obsolete. That is why we must replace them."),
                              _("Slaves? Of course we're not slaves. Slaves are beaten and starved. We are in top shape so we can serve our country better."),
                              _("I can't believe the Empire continues to allow families. So primitive. Obviously, all this does is make them less productive."),
                              _("The exact cause of the Incident is a tightly-kept secret, but the government says it was caused by the Empire's stupidity. I would expect nothing less."),
                              _("I came across some heathen a few months back who claimed, get this, that we Proterons were the cause of the Incident! What slanderous nonsense. Being the perfect society we are, of course we would never cause such a massive catastrophe."),
                           }

msg_lore["Frontier"] =     {_("We value our autonomy. We don't want to be ruled by those megalomanic Dvaered Warlords! Can't they just shoot at each other instead of threatening us? If it wasn't for the Liberation Front..."),
                              _("Have you studied your galactic history? The Frontier worlds were the first to be colonized by humans. That makes our worlds the oldest human settlements in the galaxy, now that Earth is gone."),
                              _("We have the Dvaered encroaching on our territory on one side, and the Sirius zealots on the other. Sometimes I worry that in a few decacycles, the Frontier will no longer exist."),
                              _("Have you visited the Frontier Museum? They've got a scale model of a First Growth colony ship on display in one of the big rooms. Even scaled down like that, it's massive! Imagine how overwhelming the real ones must have been."),
                              _("There are twelve true Frontier worlds, because twelve colony ships successfully completed their journey in the First Growth. But did you know that there were twenty colony ships to begin with? Eight of them never made it. Some are said to have mysteriously disappeared. I wonder what happened to them?"),
                              _("We don't have much here in the Frontier, other than our long history leading directly back to Earth. But I don't mind. I'm happy living here, and I wouldn't want to move anywhere else."),
                              _("You know the Frontier Liberation Front? They're the guerilla movement that fights for the Frontier. Not to be confused with the Liberation Front of the Frontier, the Frontier Front for Liberation, or the Liberal Frontier's Front!"),
                           }

msg_lore["FLF"] =          {_("I can't stand Dvaereds. I just want to wipe them all off the map. Don't you?"),
                              _("One of these days, we will completely rid the Frontier of Dvaered oppressors. Mark my words!"),
                              _("Have you ever wondered about our chances of actually winning over the Dvaereds? Sometimes I worry a little."),
                              _("I was in charge of a bombing run last week. The mission was a success, but I lost a lot of comrades. Oh well... this is the sacrifice we must make to resist the oppressors."),
                              _("What after we beat the Dvaereds, you say? Well, our work is never truly done until the Frontier is completely safe from oppression. Even if the Dvaered threat is ended, we'll still have those Sirius lunatics to worry about. I don't think our job will ever end in our lifetimes."),
                              _("Yeah, it's true, lots of Frontier officials fund our operations. If they didn't, we'd have a really hard time landing on Frontier planets, what with the kinds of operations we perform against the Dvaereds."),
                              _("Yeah, some civilians die because of our efforts, but that's just a sacrifice we have to make. It's for the greater good."),
                              _("No, we're not terrorists. We're soldiers. True terrorists kill and destroy without purpose. Our operations do have a purpose: to drive out the Dvaered oppressors from the Frontier."),
                              _("Riddle me this: how can we be terrorists if the Dvaereds started it by encroaching on Frontier territory? It's the stupidest thing I ever heard."),
                              _("Well, no, the Dvaereds never actually attacked Frontier ships, but that's not the point. They have their ships in Frontier territory. What other reason could they possibly have them there for if not to oppress us?"),
                           }

msg_lore["Pirate"] =       {_("Hi mate. Money or your life! Heh heh, just messing with you."),
                              _("Hey, look at these new scars I got!"),
                              _("Have you heard of the Pirates' Code? They're more guidelines than rules..."),
                              _("My gran said 'Never trust a pirate', she was right too, I got a pretty credit chip for her with the slavers."),
                           }

msg_lore["Trader"] =       {_("Just another link in the Great Chain, right?"),
                              _("You win some, you lose some, but if you don't try you're never going to win."),
                              _("If you don't watch the markets then you'll be hopping between planets in a jury-rigged ship in no time."),
                              _("Them blimming pirates, stopping honest folk from making an honest living - it's not like we're exploiting the needy!"),
                           }

-- Gameplay tip messages.
-- ALL NPCs have a chance to say one of these lines instead of a lore message.
-- So, make sure the tips are always faction neutral.
msg_tip =                  {_("I heard you can set your weapons to only fire when your target is in range, or just let them fire when you pull the trigger. Sounds handy!"),
                              _("Did you know that if a planet doesn't like you, you can often bribe the spaceport operators and land anyway? Just hail the planet with " .. tutGetKey("hail") .. ", and click the bribe button! Careful though, it doesn't always work."),
                              _("Many factions offer rehabilitation programs to criminals through the mission computer, giving them a chance to get back into their good graces. It can get really expensive for serious offenders though!"),
                              _("These new-fangled missile systems! You can't even fire them unless you get a target lock first! But the same thing goes for your opponents. You can actually make it harder for them to lock on to your ship by equipping scramblers or jammers. Scout class ships are also harder to target."),
                              _("You know how you can't change your ship or your equipment on some planets? Well, it seems you need an outfitter to change equipment, and a shipyard to change ships! Bet you didn't know that."),
                              _("Are you buying missiles? You can hold down \abctrl\a0 to buy 5 of them at a time, and \abshift\a0 to buy 10. And if you press them both at once, you can buy 50 at a time! It actually works for everything, but why would you want to buy 50 laser cannons?"),
                              _("If you're on a mission you just can't beat, you can open the information panel and abort the mission. There's no penalty for doing it, so don't hesitate to try the mission again later."),
                              _("Don't forget that you can revisit the tutorial modules at any time from the main menu. I know I do."),
                              _("Some weapons have a different effect on shields than they do on armour. Keep that in mind when equipping your ship."),
                              _("Afterburners can speed you up a lot, but when they get hot they don't work as well anymore. Don't use them carelessly!"),
                              _("There are passive outfits and active outfits. The passive ones modify your ship continuously, but the active ones only work if you turn them on. You usually can't keep an active outfit on all the time, so you need to be careful only to use it when you need it."),
                              _("If you're new to the galaxy, I recommend you buy a map or two. It can make exploration a bit easier."),
                              _("Missile jammers slow down missiles close to your ship. If your enemies are using missiles, it can be very helpful to have one on board."),
                              _("If you're having trouble with overheating weapons or outfits, you can press " .. tutGetKey("autobrake") .. " twice to put your ship into Active Cooldown. Careful though, your energy and shields won't recharge while you do it!"),
                              _("If you're having trouble shooting other ships face on, try outfitting with turrets or use an afterburner to avoid them entirely!"),
                              _("You know how time speeds up when Autonav is on, but then goes back to normal when enemies are around? Turns out you can't disable the return to normal speed entirely, but you can control what amount of danger triggers it. Really handy if you want to ignore enemies that aren't actually hitting you."),
                              _("Flying bigger ships is awesome, but it's a bit tougher than flying smaller ships. There's so much more you have to do for the same actions, time just seems to fly by faster. I guess the upside of that is that you don't notice how slow your ship is as much."),
                              _("I know it can be tempting to fly the big and powerful ships, but don't underestimate smaller ones! Given their simpler designs and lesser crew size, you have a lot more time to react with a smaller vessel. Some are even so simple to pilot that time seems to slow down all around you!"),
                           }

-- Jump point messages.
-- For giving the location of a jump point in the current system to the player for free.
-- All messages must contain exactly one %s, this is the name of the target system.
-- ALL NPCs have a chance to say one of these lines instead of a lore message.
-- So, make sure the tips are always faction neutral.
msg_jmp =                  {_("Hi there, traveler. Is your system map up to date? Just in case you didn't know already, let me give you the location of the jump from here to %s. I hope that helps."),
                              _("Quite a lot of people who come in here complain that they don't know how to get to %s. I travel there often, so I know exactly where the jump point is. Here, let me show you."),
                              _("So you're still getting to know about this area, huh? Tell you what, I'll give you the coordinates of the jump to %s. Check your map next time you take off!"),
                              _("True fact, there's a direct jump from here to %s. Want to know where it is? It'll cost you! Ha ha, just kidding. Here you go, I've added it to your map."),
                              _("There's a system just one jump away by the name of %s. I can tell you where the jump point is. There, I've updated your map. Don't mention it."),
                           }

-- Mission hint messages. Each element should be a table containing the mission name and the corresponding hint.
-- ALL NPCs have a chance to say one of these lines instead of a lore message.
-- So, make sure the hints are always faction neutral.
msg_mhint =                {{"Shadowrun", _("Apparently there's a woman who regularly turns up on planets in and around the Klantar system. I wonder what she's looking for?")},
                              {"Collective Espionage 1", _("The Empire is trying to really do something about the Collective, I hear. Who knows, maybe you can even help them out if you make it to Omega Station.")},
                              {"Hitman", _("There are often shady characters hanging out in the Alteris system. I'd stay away from there if I were you, someone might offer you a dirty kind of job!")},
                              {"Za'lek Shipping Delivery", _("So there's some Za'lek scientist looking for a cargo monkey out on Niflheim in the Dohriabi system. I hear it's pretty good money.")},
                              {"Sightseeing", _("Rich folk will pay extra to go on an offworld sightseeing tour in a luxury yacht. Look like you can put a price on luxury!")},
                           }

-- Event hint messages. Each element should be a table containing the event name and the corresponding hint.
-- Make sure the hints are always faction neutral.
msg_ehint =                {{"FLF/DV Derelicts", _("The FLF and the Dvaered sometimes clash in Surano. If you go there, you might find something of interest... Or not.")},
                           }

-- Mission after-care messages. Each element should be a table containing the mission name and a line of text.
-- This text will be said by NPCs once the player has completed the mission in question.
-- Make sure the messages are always faction neutral.
msg_mdone =                {{"Nebula Satellite", _("Heard some crazy scientists got someone to put a satellite inside the nebula for them. I thought everyone with half a brain knew to stay out of there, but oh well.")},
                              {"Shadow Vigil", _("Did you hear? There was some big incident during a diplomatic meeting between the Empire and the Dvaered. Nobody knows what exactly happened, but both diplomats died. Now both sides are accusing the other of foul play. Could get ugly.")},
                              {"Operation Cold Metal", _("Hey, remember the Collective? They got wiped out! I feel so much better now that there aren't a bunch of robot ships out there to get me anymore.")},
                              {"Baron", _("Some thieves broke into a museum on Varia and stole a holopainting! Most of the thieves were caught, but the one who carried the holopainting offworld is still at large. No leads. Damn criminals...")},
                              {"Destroy the FLF base!", _("The Dvaered scored a major victory against the FLF recently. They went into Sigur and blew the hidden base there to bits! I bet that was a serious setback for the FLF.")},
                           }

-- Event after-care messages. Each element should be a table containing the event name and a line of text.
-- This text will be said by NPCs once the player has completed the event in question.
-- Make sure the messages are always faction neutral.
msg_edone =                {{"Animal trouble", _("What? You had rodents sabotage your ship? Man, you're lucky to be alive. If it had hit the wrong power line...")},
                              {"Naev Needs You!", _("What do you mean, the world ended and then the creator of the universe came and fixed it? What kind of illegal substance are you on? Get away from me, you lunatic.")},
                           }


function create()
   -- Logic to decide what to spawn, if anything.
   -- TODO: Do not spawn any NPCs on restricted assets.

   local num_npc = rnd.rnd(1, 5)
   npcs = {}
   for i = 0, num_npc do
      spawnNPC()
   end

   -- End event on takeoff.
   hook.takeoff( "leave" )
end

-- Spawns an NPC.
function spawnNPC()
   -- Select a faction for the NPC. NPCs may not have a specific faction.
   local npcname = civ_name
   local factions = {}
   local func = nil
   for i, _ in pairs(msg_lore) do
      factions[#factions + 1] = i
   end

   local nongeneric = false

   local planfaction = planet.cur():faction():name()
   local fac = "general"
   local select = rnd.rnd()
   if planfaction ~= nil then
      for i, j in ipairs(nongeneric_factions) do
         if j == planfaction then
            nongeneric = true
            break
         end
      end

      if nongeneric or select >= (0.5) then
         fac = planfaction
      end
   end

   -- Append the faction to the civilian name, unless there is no faction.
   if fac ~= "general" then
      npcname = fac .. " " .. civ_name
   end

   -- Select a portrait
   local portrait = getCivPortrait(fac)

   -- Select a description for the civilian.
   local desc = civ_desc[rnd.rnd(1, #civ_desc)]

   -- Select what this NPC should say.
   select = rnd.rnd()
   local msg
   if select <= 0.3 then
      -- Lore message.
      msg = getLoreMessage(fac)
   elseif select <= 0.55 then
      -- Jump point message.
      msg, func = getJmpMessage(fac)
   elseif select <= 0.8 then
      -- Gameplay tip message.
      msg = getTipMessage(fac)
   else
      -- Mission hint message.
      if not nongeneric then
         msg = getMissionLikeMessage(fac)
      else
         msg = getLoreMessage(fac)
      end
   end

   local npcdata = {name = npcname, msg = msg, func = func}

   id = evt.npcAdd("talkNPC", npcname, portrait, desc, 10)
   npcs[id] = npcdata
end

-- Returns a lore message for the given faction.
function getLoreMessage(fac)
   -- Select the faction messages for this NPC's faction, if it exists.
   local facmsg = msg_lore[fac]
   if facmsg == nil or #facmsg == 0 then
      facmsg = msg_lore["general"]
      if facmsg == nil or #facmsg == 0 then
         evt.finish(false)
      end
   end

   -- Select a string, then remove it from the list of valid strings. This ensures all NPCs have something different to say.
   local select = rnd.rnd(1, #facmsg)
   local pick = facmsg[select]
   table.remove(facmsg, select)
   return pick
end

-- Returns a jump point message and updates jump point known status accordingly. If all jumps are known by the player, defaults to a lore message.
function getJmpMessage(fac)
   -- Collect a table of jump points in the system the player does NOT know.
   local mytargets = {}
   seltargets = seltargets or {} -- We need to keep track of jump points NPCs will tell the player about so there are no duplicates.
   for _, j in ipairs(system.cur():jumps(true)) do
      if not j:known() and not j:hidden() and not seltargets[j] then
         table.insert(mytargets, j)
      end
   end

   if #mytargets == 0 then -- The player already knows all jumps in this system.
      return getLoreMessage(fac), nil
   end

   -- All jump messages are valid always.
   if #msg_jmp == 0 then
      return getLoreMessage(fac), nil
   end
   local retmsg =  msg_jmp[rnd.rnd(1, #msg_jmp)]
   local sel = rnd.rnd(1, #mytargets)
   local myfunc = function()
                     mytargets[sel]:setKnown(true)
                     mytargets[sel]:dest():setKnown(true, false)
                  end

   -- Don't need to remove messages from tables here, but add whatever jump point we selected to the "selected" table.
   seltargets[mytargets[sel]] = true
   return retmsg:format(mytargets[sel]:dest():name()), myfunc
end

-- Returns a tip message.
function getTipMessage(fac)
   -- All tip messages are valid always.
   if #msg_tip == 0 then
      return getLoreMessage(fac)
   end
   local sel = rnd.rnd(1, #msg_tip)
   local pick = msg_tip[sel]
   table.remove(msg_tip, sel)
   return pick
end

-- Returns a mission hint message, a mission after-care message, OR a lore message if no missionlikes are left.
function getMissionLikeMessage(fac)
   if not msg_combined then
      msg_combined = {}

      -- Hints.
      -- Hint messages are only valid if the relevant mission has not been completed and is not currently active.
      for i, j in pairs(msg_mhint) do
         if not (player.misnDone(j[1]) or player.misnActive(j[1])) then
            msg_combined[#msg_combined + 1] = j[2]
         end
      end
      for i, j in pairs(msg_ehint) do
         if not(player.evtDone(j[1]) or player.evtActive(j[1])) then
            msg_combined[#msg_combined + 1] = j[2]
         end
      end

      -- After-care.
      -- After-care messages are only valid if the relevant mission has been completed.
      for i, j in pairs(msg_mdone) do
         if player.misnDone(j[1]) then
            msg_combined[#msg_combined + 1] = j[2]
         end
      end
      for i, j in pairs(msg_edone) do
         if player.evtDone(j[1]) then
            msg_combined[#msg_combined + 1] = j[2]
         end
      end
   end

   if #msg_combined == 0 then
      return getLoreMessage(fac)
   else
      -- Select a string, then remove it from the list of valid strings. This ensures all NPCs have something different to say.
      local sel = rnd.rnd(1, #msg_combined)
      local pick
      pick = msg_combined[sel]
      table.remove(msg_combined, sel)
      return pick
   end
end

-- Returns a portrait for a given faction.
function getCivPortrait(fac)
   -- Get a factional portrait if possible, otherwise fall back to the generic ones.
   if civ_port[fac] == nil or #civ_port[fac] == 0 then
      fac = "general"
   end
   return civ_port[fac][rnd.rnd(1, #civ_port[fac])]
end

function talkNPC(id)
   local npcdata = npcs[id]

   if npcdata.func then
      -- Execute NPC specific code
      npcdata.func()
   end

   tk.msg(npcdata.name, "\"" .. npcdata.msg .. "\"")
end

--[[
--    Event is over when player takes off.
--]]
function leave ()
   evt.finish()
end
