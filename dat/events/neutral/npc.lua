
--[[
-- Event for creating random characters in the spaceport bar.
-- The random NPCs will tell the player things about the Naev universe in general, about their faction, or about the game itself.
--]]

include "dat/events/tutorial/tutorial-common.lua"

lang = naev.lang()
if lang == 'es' then --not translated atm
else --default english
   -- Portraits.
   -- When adding portraits, make sure to add them to the table of the faction they belong to.
   -- Does your faction not have a table? Then just add it. The script will find and use it if it exists.
   -- Make sure you spell the faction name exactly the same as in faction.xml though!
   civ_port = {}
   civ_port["general"] =   {"neutral/male1",
                            "neutral/thief1",
                            "neutral/thief2",
                            "neutral/thief3"
                           }
   civ_port["Sirius"] =    {"sirius/sirius_fyrra_f1",
                            "sirius/sirius_fyrra_f2",
                            "sirius/sirius_fyrra_m1"
                           }
   civ_name = "Civilian"

   -- Civilian descriptions for the spaceport bar.
   -- These descriptions will be picked at random, and may be picked multiple times in one generation.
   -- Remember that any description can end up with any portrait, so don't make any assumptions
   -- about the appearance of the NPC!
   civ_desc = {"This person seems to be here to relax.",
               "There is a civilian sitting on one of the tables.",
               "There is a civilian sitting there, looking somewhere else.",
               "A worker sits at one of the tables, wearing a nametag saying \"Go away\".",
               "A civilian sits at the bar, seemingly serious about the cocktails on offer.",
               "You see a civilian wearing a shirt saying: \"Ask me about Jaegnhild\"",
               "There is a civilian sitting in the corner.",
               "A civilian feverishly concentrating on a fluorescing drink.",
               "A civilian drinking alone.",
               "This person seems friendly enough.",
               "A civilian sitting at the bar.",
               "This person is idly browsing a news terminal."
               }

   -- Lore messages. These come in general and factional varieties.
   -- General lore messages will be said by non-faction NPCs, OR by faction NPCs if they have no factional text to say.
   -- When adding factional text, make sure to add it to the table of the appropriate faction.
   -- Does your faction not have a table? Then just add it. The script will find and use it if it exists.
   -- Make sure you spell the faction name exactly the same as in faction.xml though!
   msg_lore = {}
   msg_lore["general"] =      {"I heard the nebula is haunted! My uncle Bobby told me he saw one of the ghost ships himself over in Arandon!",
                               "I don't believe in those nebula ghost stories. The people who talk about it are just trying to scare you.",
                               "I heard the Soromid lost their homeworld Sorom in the Incident. Its corpse can still be found in Basel.",
                               "The Soromid fly organic ships! I heard their ships can even heal themselves in flight. That's so weird.",
                               "Have you seen that ship the Emperor lives on? It's huge! But if you ask me, it looks a bit like a... No, never mind.",
                               "I wonder why the Sirii are all so devout? I heard they have these special priesty people walking around. I wonder what's so special about them.",
                               "They say Eduard Manual Goddard is drifting in space somewhere, entombed amidst a cache of his inventions! What I wouldn't give to rummage through there...",
                               "Ah man, I lost all my money on Totoran. I love the fights they stage there, but the guys I bet on always lose. What am I doing wrong?",
                               "Don't try to fly into the inner nebula. I've known people who tried, and none of them came back.",
                               "Have you heard of Captain T. Practice? He's amazing, I'm his biggest fan!",
                               "I wouldn't travel north from Alteris if I were you, unless you're a good fighter! That area of space has really gone down the drain since the Incident.",
                               "Sometimes I look at the stars and wonder... are we the only sentient species in the universe?",
                              }

   msg_lore["Independent"] =  {"We're not part of any of the galactic superpowers. We can take care of ourselves!",
                               "Sometimes I worry that our lack of a standing military leaves us vulnerable to attack. I hope nobody will get any ideas of conquering us!"
                              }

   msg_lore["Empire"] =       {"Things are getting worse by the cycle. What happened to the Empire? We used to be the lords and masters over the whole galaxy!",
                               "Did you know that House Za'lek was originally a Great Project initiated by the Empire? Well, now you do! There was also a Project Proteron, but that didn't go so well.",
                               "The Emperor lives on a giant supercruiser in Gamma Polaris. It's said to be the biggest ship in the galaxy! I totally want one.",
                               "I'm still waiting for my pilot license application to get through. Oh well, it's only been half a cycle, I just have to be patient.",
                               "Between you and me, the laws the Council passes can get really ridiculous! Most planets find creative ways of ignoring them...",
                               "Don't pay attention to the naysayers. The Empire is still strong. Have you ever seen a Peacemaker up close? I doubt any ship fielded by any other power could stand up to one."
                              }

   msg_lore["Dvaered"] =      {"Our Warlord is currently fighting for control over another planet. We all support him unconditionally, of course! Of course...",
                               "My great-great-great-grandfather fought in the Dvaered Revolts! We still have the holovids he made. I'm proud to be a Dvaered!",
                               "I've got lots of civilian commendations! It's important to have commendations if you're a Dvaered.",
                               "You better not mess with House Dvaered. Our military is the largest and strongest in the galaxy. Nobody can stand up to us!",
                               "House Dvaered? House? The Empire is weak and useless, we don't need them anymore! I say we declare ourselves an independent faction today. What are they going to do, subjugate us? We all know how well that went last time! Ha!",
                               "I'm thinking about joining the military. Every time I see or hear news about those rotten FLF bastards, it makes my blood boil! They should all be pounded into space dust!",
                               "FLF terrorists? I'm not too worried about them. You'll see, High Command will have smoked them out of their den soon enough, and then the Frontier will be ours.",
                               "Did you know that House Dvaered was named after a hero of the revolt? At least that's what my grandparents told me.",
                              }

   msg_lore["Sirius"] =       {"Greetings, traveler. May Sirichana's wisdom guide you as it guides me.",
                               "I once met one of the Touched in person. Well, it wasn't really a meeting, our eyes simply met... But that instant alone was awe-inspiring.",
                               "They say Sirichana lives and dies like any other man, but each new Shirichana is the same as the last. How is that possible?",
                               "My cousin was called to Mutris a cycle ago... He must be in Crater City by now. And one day, he will become one of the Touched!",
                               "Some people say Sirius society is unfair because our echelons are determined by birth. But even though we are different, we are all followers of Sirichana. Spiritually, we are equal.",
                               "House Sirius is officially part of the Empire, but everyone knows that's only true on paper. The Emperor has nothing to say in these systems. We follow Sirichana, and no-one else.",
                               "You can easily tell the different echelons apart. Every Sirian citizen and soldier wears clothing appropriate to his or her echelon.",
                               "I hope to meet one of the Touched one day!",
                              }

   msg_lore["Soromid"] =      {"Hello. Can I interest you in one of our galaxy famous cosmetic gene treatments? You look like you could use them...",
                               "Can you believe it? I was going to visit Sorom to find my roots, and then boom! It got burnt to a crisp! Even now, cycles later, I still can't believe it.",
                               "Yes, it's true, our military ships are alive. Us normal folk don't get to own bioships though, we have to make do with synthetic constructs just like everyone else.",
                               "Everyone knows that we Soromid altered ourselves to survive the deadly conditions on Sorom during the Great Quarantine. What you don't hear so often is that billions of us died from the therapy itself. We paid a high price for survival.",
                               "Our cosmetic gene treatments are even safer now for non-soromids, with a rate of survival of 99.4%!",
                               "We have been rebuilding and enhancing our bodies for so long. I say we became a new species, one above human",
                              }

   msg_lore["Za'lek"] =       {"It's not easy, dancing to those scientists' tunes. They give you the most impossible tasks! Like, where am I supposed to get a triple redundant helitron converter? Honestly.",
                               "The Soromids? Hah! We Za'lek are the only true scientists in this galaxy.",
                               "I don't understand why we bother sending our research results to the Empire. These simpletons can't understand the simplest formulas!",
                               }

   msg_lore["Proteron"] =     {"Hello, traveler. Welcome to Proteron space. We are an evil, power hungry dystopia on a quest for dominance over the galaxy. Would you like a brochure?"
                              }

   msg_lore["Frontier"] =     {"We value our autonomy. We don't want to be ruled by those megalomanic Dvaered Warlords! Can't they just shoot at each other instead of threatening us? If it wasn't for the Liberation Front...",
                               "Have you studied your galactic history? The Frontier worlds were the first to be colonized by humans. That makes our worlds the oldest human settlements in the galaxy, now that Earth is gone.",
                               "We have the Dvaered encroaching on our territory on one side, and the Sirius zealots on the other. Sometimes I worry that in a few decacycles, the Frontier will no longer exist.",
                               "Have you visited the Frontier Museum? They've got a scale model of a First Growth colony ship on display in one of the big rooms. Even scaled down like that, it's massive! Imagine how overwhelming the real ones must have been.",
                               "There are eleven true Frontier worlds, because eleven colony ships successfully completed their journey in the First Growth. But did you know that there were twenty colony ships to begin with? Nine of them never made it. Some are said to have mysteriously disappeared. I wonder what happened to them?",
                               "We don't have much here in the Frontier, other than our long history leading directly back to Earth. But I don't mind. I'm happy living here, and I wouldn't want to move anywhere else.",
                               "You know the Frontier Liberation Front? They're the guerilla movement that fights for the Frontier. Not to be confused with the Liberation Front of the Frontier, the Frontier Front for Liberation, or the Liberal Frontier's Front!"
                              }

   msg_lore["Pirate"] =       {"Hi mate. Money or your life! Heh heh, just messing with you.",
                               "Hey, look at these new scars I got!",
                              }

   -- Gameplay tip messages.
   -- ALL NPCs have a chance to say one of these lines instead of a lore message.
   -- So, make sure the tips are always faction neutral.
   msg_tip =                  {"I heard you can set your weapons to only fire when your target is in range, or just let them fire when you pull the trigger. Sounds handy!",
                               "Did you know that if a planet doesn't like you, you can often bribe the spaceport operators and land anyway? Just hail the planet with " .. tutGetKey("hail") .. ", and click the bribe button! Careful though, it doesn't always work.",
                               "Many factions offer rehabilitation programs to criminals through the mission computer, giving them a chance to get back into their good graces. It can get really expensive for serious offenders though!",
                               "These new-fangled missile systems! You can't even fire them unless you get a target lock first! But the same thing goes for your opponents. You can actually make it harder for them to lock on to your ship by equipping scramblers or jammers. Scout class ships are also harder to target.",
                               "Your equipment travels with you from planet to planet, but your ships don't! Nobody knows why, it's just life, I guess.",
                               "You know how you can't change your ship or your equipment on some planets? Well, it seems you need an outfitter to change equipment, and a shipyard to change ships! Bet you didn't know that.",
                               "Are you buying missiles? You can hold down \027bctrl\0270 to buy 5 of them at a time, and \027bshift\0270 to buy 10. And if you press them both at once, you can buy 50 at a time! It actually works for everything, but why would you want to buy 50 laser cannons?",
                               "If you're on a mission you just can't beat, you can open the information panel and abort the mission. There's no penalty for doing it, so don't hesitate to try the mission again later.",
                               "Don't forget that you can revisit the tutorial modules at any time from the main menu. I know I do.",
                               "Some weapons have a different effect on shields than they do on armour. Keep that in mind when equipping your ship.",
                               "Afterburners only work when you activate them. While they're inactive all they do is slow you down!",
                               "There are passive outfits and active outfits. The passive ones modify your ship continuously, but the active ones only work if you turn them on. You usually can't keep an active outfit on all the time, so you need to be careful only to use it when you need it.",
                               "If you're new to the galaxy, I recommend you buy a map or two. It can make exploration a bit easier.",
                               "Missile jammers slow down missiles close to your ship. If your enemies are using missiles, it can be very helpful to have one on board."
                              }

   -- Jump point messages.
   -- For giving the location of a jump point in the current system to the player for free.
   -- All messages must contain exactly one %s, this is the name of the target system.
   -- ALL NPCs have a chance to say one of these lines instead of a lore message.
   -- So, make sure the tips are always faction neutral.
   msg_jmp =                  {"Hi there, traveler. Is your system map up to date? Just in case you didn't know already, let me give you the location of the jump from here to %s. I hope that helps.",
                               "Quite a lot of people who come in here complain that they don't know how to get to %s. I travel there often, so I know exactly where the jump point is. Here, let me show you.",
                               "So you're still getting to know about this area, huh? Tell you what, I'll give you the coordinates of the jump to %s. Check your map next time you take off!",
                               "True fact, there's a direct jump from here to %s. Want to know where it is? It'll cost you! Ha ha, just kidding. Here you go, I've added it to your map.",
                               "There's a system just one jump away by the name of %s. I can tell you where the jump point is. There, I've updated your map. Don't mention it."
                              }

   -- Mission hint messages. Each element should be a table containing the mission name and the corresponding hint.
   -- ALL NPCs have a chance to say one of these lines instead of a lore message.
   -- So, make sure the hints are always faction neutral.
   msg_mhint =                {{"Shadowrun", "Apparently there's a woman who regularly turns up on planets in and around the Klantar system. I wonder what she's looking for?"},
                               {"Collective Espionage 1", "The Empire is trying to really do something about the Collective, I hear. Who knows, maybe you can even help them out if you make it to Omega station."},
                               {"Hitman", "There are often shady characters hanging out in the Alteris system. I'd stay away from there if I were you, someone might offer you a dirty kind of job!"}
                              }

   -- Event hint messages. Each element should be a table containing the event name and the corresponding hint.
   -- Make sure the hints are always faction neutral.
   msg_ehint =                {{"FLF/DV Derelicts", "The FLF and the Dvaered sometimes clash in Surano. If you go there, you might find something of interest... Or not."}
                              }

   -- Mission after-care messages. Each element should be a table containing the mission name and a line of text.
   -- This text will be said by NPCs once the player has completed the mission in question.
   -- Make sure the messages are always faction neutral.
   msg_mdone =                {{"Nebula Satellite", "Heard some crazy scientists got someone to put a satellite inside the nebula for them. I thought everyone with half a brain knew to stay out of there, but oh well."},
                               {"Shadow Vigil", "Did you hear? There was some big incident during a diplomatic meeting between the Empire and the Dvaered. Nobody knows what exactly happened, but both diplomats died. Now both sides are accusing the other of foul play. Could get ugly."},
                               {"Operation Cold Metal", "Hey, remember the Collective? They got wiped out! I feel so much better now that there aren't a bunch of robot ships out there to get me anymore."},
                               {"Baron", "Some thieves broke into a museum on Varia and stole a holopainting! Most of the thieves were caught, but the one who carried the holopainting offworld is still at large. No leads. Damn criminals..."},
                               {"Destroy the FLF base!", "The Dvaered scored a major victory against the FLF recently. They went into Sigur and blew the hidden base there to bits! I bet that was a serious setback for the FLF."}
                              }

   -- Event after-care messages. Each element should be a table containing the event name and a line of text.
   -- This text will be said by NPCs once the player has completed the event in question.
   -- Make sure the messages are always faction neutral.
   msg_edone =                {{"Animal trouble", "What? You had rodents sabotage your ship? Man, you're lucky to be alive. If it had hit the wrong power line..."},
                               {"Naev Needs You!", "What do you mean, the world ended and then the creator of the universe came and fixed it? What kind of illegal substance are you on? Get away from me, you lunatic."}
                              }
end


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

   local fac = "general"
   local select = rnd.rnd()
   if select >= (0.5) and planet.cur():faction() ~= nil then
      fac = planet.cur():faction():name()
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
      msg, func = getJmpMessage()
   elseif select <= 0.8 then
      -- Gameplay tip message.
      msg = getTipMessage()
   else
      -- Mission hint message.
      msg = getMissionLikeMessage()
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
function getJmpMessage()
   -- Collect a table of jump points in the system the player does NOT know.
   local mytargets = {}
   for _,j in ipairs(system.cur():jumps(true)) do
      if not j:known() and not j:hidden() then
         table.insert(mytargets, j)
      end
   end

   if #mytargets == 0 then -- The player already knows all jumps in this system.
      return getLoreMessage(), nil
   end

   -- All jump messages are valid always.
   if #msg_jmp == 0 then
      return getLoreMessage(), nil
   end
   local retmsg =  msg_jmp[rnd.rnd(1, #msg_jmp)]
   local sel = rnd.rnd(1, #mytargets)
   local myfunc = function()
                     mytargets[sel]:setKnown(true)
                     mytargets[sel]:system():setKnown(true, false)
                  end

   -- Don't need to remove messages from tables here.
   return retmsg:format(mytargets[sel]:dest():name()), myfunc
end

-- Returns a tip message.
function getTipMessage()
   -- All tip messages are valid always.
   if #msg_tip == 0 then
      return getLoreMessage()
   end
   local sel = rnd.rnd(1, #msg_tip)
   local pick = msg_tip[sel]
   table.remove(msg_tip, sel)
   return pick
end

-- Returns a mission hint message, a mission after-care message, OR a lore message if no missionlikes are left.
function getMissionLikeMessage()
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
      return getLoreMessage()
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
