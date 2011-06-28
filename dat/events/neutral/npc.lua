
--[[
-- Event for creating random characters in the spaceport bar.
--]]

include "dat/events/tutorial/tutorial-common.lua"

lang = naev.lang()
if lang == 'es' then --not translated atm
else --default english
   -- Portraits. Store generic and by faction.
   civ_port = {}
   civ_port["general"] =   {"male1",
                            "thief1",
                            "thief2",
                            "thief3"
                           }
   civ_port["Sirius"] =    {"sirius_fyrra_f1"
                           }
   civ_name = "Civilian"

   -- Civilian descriptions for the spaceport bar.
   civ_desc = {"A worker on his day off.",
               "There is a civilian sitting on one of the tables. He ignores you.",
               "There is a civilian sitting there, looking somewhere else.",
               "A worker sits at one of the tables, he's wearing a nametag saying \"Go away\".",
               "A civilian sits at the bar, bragging about how many Alterian burning cocktails he can drink.",
               "You see a drunk civilian, babbling to himself.",
               "You see a civilian wearing a shirt saying: \"Ask me about Jaegnhild\"",
               "There is a civilian sitting in the corner.",
               "A civilian on his day off.",
               "The man seems to be new to drinking and is disgusted by the other people in the bar.",
               "A man feverishly concentrating on the fluorescing drink in his hand."
               }

   -- Lore messages. These come in general and factional varieties.
   -- Important: the "general" message table MUST NOT be empty!
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
                               "Have you heard of Captain T. Practice? He's amazing, I'm his biggest fan!"
                              }

   msg_lore["Independent"] =  {"We're not part of any of the galactic superpowers. We can take care of ourselves!",
                               "Sometimes I worry that our lack of a standing military leaves us vulnerable to attack. I hope nobody will get any ideas of conquering us!"
                              }

   msg_lore["Empire"] =       {"Things are getting worse by the cycle. What happened to the Empire? We used to be the lords and masters over the whole galaxy!",
                               "Did you know that House Za'lek was originally a Great Project initiated by the Empire? Well, now you do! There was also a Project Proteron, but that didn't go so well."
                              }

   msg_lore["Dvaered"] =      {"Our Warlord is currently fighting for control over another planet. We all support him unconditionally, of course! Of course...",
                               "My great-great-great-grandfather fought in the Dvaered Revolts! We still have the holovids he made. I'm proud to be a Dvaered!"
                              }

   msg_lore["Sirius"] =       {"Greetings, traveler. May Sirichana's wisdom guide you as it guides me.",
                               "I once met one of the Touched in person. Well, it wasn't really a meeting, our eyes simply met... But that instant alone was awe-inspiring.",
                               "They say Sirichana lives and dies like any other man, but each new Shirichana is the same as the last. How is that possible?",
                               "My cousin was called to Mutris a cycle ago... He must be in Crater City by now. And one day, he will become one of the Touched!",
                               "Some people say Sirius society is unfair because our echelons are determined by birth. But even though we are different, we are all followers of Sirichana. Spiritually, we are equal."
                              }

   msg_lore["Soromid"] =      {"Hello. Can I interest you in one of our galaxy famous gene treatments? You look like you could use them..."
                              }

   msg_lore["Za'lek"] =       {"It's not easy, dancing to those scientists' tunes. They give you the most impossible tasks! Like, where am I supposed to get a triple redundant helitron converter? Honestly."
                              }

   msg_lore["Proteron"] =     {"Hello, traveler. Welcome to Proteron space. We are an evil, power hungry dystopia on a quest for dominance over the galaxy. Would you like a brochure?"
                              }

   msg_lore["Frontier"] =     {"We value our autonomy. We don't want to be ruled by those megalomanic Dvaered Warlords! Can't they just shoot at each other instead of threatening us? If it wasn't for the Liberation Front..."
                              }

   msg_lore["Pirate"] =       {"Hi mate. Money or your life! Heh heh, just messing with you."
                              }

   msg_lore["Thurion"] =      {}
   
   -- Gameplay tip messages.
   msg_tip =                  {"I heard you can set your weapons to only fire when your target is in range, or just let them fire when you pull the trigger. Sounds handy!",
                               "Did you know that if a planet doesn't like you, you can often bribe the spaceport operators and land anyway? Just hail the planet with " .. tutGetKey("hail") .. ", and click the bribe button! Careful though, it doesn't always work.",
                               "Many factions offer rehabilitation programs to criminals through the mission computer, giving them a chance to get back into their good graces. It can get really expensive for serious offenders though!",
                               "These new-fangled missile systems! You can't even fire them unless you get a target lock first! But the same thing goes for your opponents. You can actually make it harder for them to lock on to your ship by equipping scramblers or jammers. Scout class ships are also harder to target.",
                               "Your equipment travels with you from planet to planet, but your ships don't! Nobody knows why, it's just life, I guess.",
                               "You know how you can't change your ship or your equipment on some planets? Well, it seems you need an outfitter to change equipment, and a shipyard to change ships! Bet you didn't know that.",
                               "Are you buying missiles? You can hold down \027bctrl\0270 to buy 5 of them at a time, and \027bshift\0270 to buy 10. And if you press them both at once, you can buy 50 at a time! It actually works for everything, but why would you want to buy 50 laser cannons?",
                               "If you're on a mission you just can't beat, you can open the information panel and abort the mission. There's no penalty for doing it, so don't hesitate to try the mission again later.",
                               "Don't forget that you can revisit the tutorial modules at any time from the main menu. I know I do."
                              }

   -- Mission hint messages. Each element should be a table containing the mission name and the corresponding hint.
   msg_mhint =                {{"Shadowrun", "Apparently there's a woman who regularly turns up on planets in and around the Klantar system. I wonder what she's looking for?"},
                               {"Collective Espionage 1", "The Empire is trying to really do something about the Collective, I hear. Who knows, maybe you can even help them out if you make it to Omega station."},
                               {"Hitman", "There are often shady characters hanging out in the Alteris system. I'd stay away from there if I were you, someone might offer you a dirty kind of job!"}
                              }

   -- Event hint messages. Each element should be a table containing the event name and the corresponding hint.
   msg_ehint =                {{"FLF/DV Derelicts", "The FLF and the Dvaered sometimes clash in Surano. If you go there, you might find something of interest... Or not."}
                              }
end   


function create()
   -- Logic to decide what to spawn, if anything.
   -- TODO: Do not spawn any NPCs on restricted assets.
   
   local num_npc = rnd.rnd(1, 5)
   num_npc = 8 -- deleteme
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
   if select <= 0.4 then
      -- Lore message.
      msg = getLoreMessage(fac)
   elseif select <= 0.7 then
      -- Gameplay tip message.
      msg = getTipMessage()
   else
      -- Mission hint message.
      msg = getHintMessage()
   end
   
   local npcdata = {name = npcname, msg = msg}
   
   id = evt.npcAdd("talkNPC", npcname, portrait, desc, 10)
   npcs[id] = npcdata
end

-- Returns a lore message for the given faction.
function getLoreMessage(fac)
   -- Select the faction messages for this NPC's faction, if it exists.
   local facmsg = msg_lore[fac]
   if facmsg == nil or #facmsg == 0 then
      facmsg = msg_lore["general"]
   end

   -- Select a string, then remove it from the list of valid strings. This ensures all NPCs have something different to say.
   local select = rnd.rnd(1, #facmsg)
   local pick = facmsg[select]
   table.remove(facmsg, select)
   return pick
end

-- Returns a tip message.
function getTipMessage()
   -- All tip messages are valid always.
   if #msg_tip == 0 then
      return
   end
   return msg_tip[rnd.rnd(1, #msg_tip)]
end

-- Returns a mission hint message, OR a lore message if no hints are left.
function getHintMessage()
   -- Hint messages are only valid if the relevant mission has not been completed and is not currently active.
   for i, j in pairs(msg_mhint) do
      if player.misnDone(j[1]) or player.misnActive(j[1]) then
         table.remove(msg_mhint, i)
      end
   end
   for i, j in pairs(msg_ehint) do
      if player.evtDone(j[1]) or player.evtActive(j[1]) then
         table.remove(msg_ehint, i)
      end
   end
   
   if #msg_mhint + #msg_ehint == 0 then
      return getLoreMessage()
   else
      -- Select a string, then remove it from the list of valid strings. This ensures all NPCs have something different to say.
      local select = rnd.rnd(1, #msg_mhint + #msg_ehint)
      local pick
      if select > #msg_mhint then
         pick = msg_ehint[select - #msg_mhint][2]
         table.remove(msg_ehint, select - #msg_mhint)
      else
         pick = msg_mhint[select][2]
         table.remove(msg_mhint, select)
      end
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
   
   tk.msg(npcdata["name"], npcdata["msg"])
end

--[[
--    Event is over when player takes off.
--]]
function leave ()
   evt.finish()
end
