--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Prince">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <done>Baron</done>
   <chance>2</chance>
   <location>Bar</location>
   <faction>Empire</faction>
   <faction>Dvaered</faction>
   <faction>Goddard</faction>
   <faction>Sirius</faction>
  </avail>
 </mission>
 --]]
--[[
-- This is the second mission in the baron string.
--]]

require "fleethelper.lua"
require "numstring.lua"
require "portrait.lua"
require "dat/missions/baron/common.lua"


title = {}
text = {}
osd_msg = {}
misn_desc = {}
   
title[1] = _("His Baronship remembers you")
text[1] = _([[As you approach the stranger, he extends his hand in greeting. He introduces himself as an associate of Baron Sauterfeldt, the man you helped to "acquire" a holopainting not too long ago.
   "The Baron was quite pleased with your performance in that matter," he confides. "He has asked me to try to find you again for another job not unlike the last one. The Baron is a collector, you see, and his hunger for new possessions is a hard one to satiate." He makes a face. "Of course, his methods aren't always completely respectable, as you've experienced for yourself. But I assure you that the Baron is not a bad man, he is simply very enthusiastic."
   You decide to keep your opinion of the aristocrat to yourself. Instead you inquire as to what the man wants from you this time. "To tell the truth, I don't actually know," the man says. "The Baron wants you to meet him so he can brief you in person. You will find his ship in the %s system. Shall I inform his lordship that you will be paying him a visit?"]])

refusetitle = _("Sorry, not today")
refusetext = _([["Okay, fair enough," the man says with a disappointed look on his face. "I'll try to find someone else. But maybe we'll run into each other again, so if you change your mind..."]])

title[2] = _("At your beck and call")
text[2] = _([["Splendid. Please go see his lordship at the earliest opportunity. He doesn't like to be kept waiting. I will send word that you will be coming, so contact the Pinnacle when you arrive at %s, and they will allow you to board."]])

title[3] = _("Green light for docking")
text[3] = _([[Your comm is answered by a communications officer on the bridge of the Pinnacle. You tell her you've got a delivery for the baron. She runs a few checks on a console off the screen, then tells you you've been cleared for docking and that the Pinnacle will be brought to a halt.]]) 
   
title[4] = _("An audience with the Baron")
text[4] = _([[You find yourself once again aboard the Pinnacle, Baron Sauterfeldt's flag ship. After a short time, an attendant ushers you into the Baron's personal quarters, which are as extravagant as you remember them. You notice the holopainting is now firmly fixed on one of the walls.
   Baron Dovai Sauterfeldt greets you with a pompous wave of his hand. "Ahh yes, there you are at last. %s, was it? Do have a seat." He then offers you a drink, but you decline on the basis that you still have to drive. "Now then, %s, I assume you're wondering why I've called you here. As you've no doubt heard, I have an interest in the unique, the exquisite." The Baron gestures around the room. "I have built up quite an impressive collection, as you can see, but it is still lacking something. Fortunately, news has reached me about a priceless artifact from Earth itself, dating back to before the Faction Wars. I must have it. It belongs in the hands of a connoisseur like myself."]])
   
text[5] = _([["Unfortunately, news of this artifact has reached more ears than just mine. All over the galaxy there are people who will try to sell you 'ancient artifacts', which always turn out to be imitations at best and worthless scrap they picked up from the streets at worst." The Baron snorts derisively. "Even the contacts who usually fenc- ah, I mean, supply me with new items for my collection are in on the frenzy.
   "I've narrowed down my search to three of these people. I'm confident that one of them is selling the genuine article, while the other two are shams. And this is where you come in, %s. I want you to visit these vendors, buy their wares off them and bring me the authentic artifact. You will have the help of a man named Flintley, who is a history buff or some such rot. You will find him on %s in the %s system. Simply tell him you're working for me and show him any artifacts in your possession. He will tell you which are authentic and which are fake.
   "I should warn you, %s. Some of my, ah, colleagues have also set their sights on this item, and so you can expect their henchmen to try to take it off you. I trust you are equipped to defend yourself against their despicable sort."]])
   
title[5] = _("Off to the shops")
   
text[6] = _([[You are swiftly escorted back to your ship. You didn't really get the chance to ask the Baron any questions, such as who these potential attackers are, how you're supposed to pay for the artifacts once you locate the sellers, or what you will get out of all this. You do, however, find an update to your galaxy map that shows the location of the sellers, as well as a list of names and portraits. It would seem that the only way to find out what you're dealing with is the hard way.]])
   
title[6] = _("What are you doing here?")

text[7] = _([[You have not yet collected and identified the genuine artifact. Buy the artifacts from the sellers and visit Flintley on %s (%s) to identify the real one.]])

title[7] = _("Flintley, at your service")

text[8] = _([[You approach the nervous-looking man and inquire if he is Flintley, the historian in Baron Sauterfeldt's employ.
   "Oh, yes. Yes! That is me! I'm Flintley," the man responds. "And you must be %s. I know what's going on, the people from the Pinnacle have informed me. Oh, but where are my manners. Let me properly introduce myself. My name is Flintley, and I'm an archaeologist and historian. The best in the galaxy, some might say, ha-ha!" He gives you a look. "Well, maybe not. But I'm quite knowledgeable about the history of the galaxy. Too bad not too many people seem interested in that these days. The only work I can really get is the occasional appraisal, like I'm doing now for his lordship. I wish I didn't have to take jobs like this, but there you have it."
   Flintley sighs. "Well, that's that. Come to me with any artifacts you manage to procure, and I'll analyze them to the best of my ability."]])
   
title[8] = _("A suspicious salesman")

text[9] = _([["Hello there," the guy says to you when you approach. "Can I interest you in this bona fide relic from an ancient past? Unlike all those scammers out there, I offer you the real deal, no fakes here!"]])

text[10] = _([[The man grins at you. "Ah, I can tell you have the eye of a connoisseur! I deal only in the finest, counterfeit-free antiques. If you're smart, and I can see that you are, you won't trust all those opportunists who will try to sell you fakes! How about it?"]])

text[11] = _([[The woman beckons you over to the bar. "Listen, friend. I have here a unique, extremely rare remnant of prehistoric times. This is the genuine article, trust me on that. One hundred per cent legit! And you wouldn't want to spend good credits on a fake, right?"]])

title[9] = _("This is not the artifact you're looking for")

text[12] = _([["Let's see what we have here," Flintley says as you hand him the artifact you bought on %s. "Ah, I know what this is without even looking anything up. It's a piece of an old-fashioned airlock mechanism, as used on most ships during the Faction Wars. That makes it rather old, but that also makes it worthless, I'm afraid. This is just old scrap." He gives you an apologetic look. "Don't let it get you down. Not many people would know this on first sight. Those scammers can be pretty clever."
   You feel disappointed and frustrated, but you have no choice but to deposit the "artifact" into the nearest disintegrator inlet.]])

text[13] = _([[You hand Flintley the artifact you procured on %s. He examines it for a few moments, then enters a few queries in the info terminal in his table. Once he has found what he was looking for, he heaves a sigh. "I'm sorry, %s. It seems you've been had. What you've got here is little more than a trinket. It's a piece of 'art' created by a third-rank sculptress named Biena Gharibri who lives on Lapra. She's not very talented, I'm afraid. Her creations have been called 'worse than Dvaered opera' by a leading art critic. I really don't think you want to present his lordship with this."
   You promptly decide to dispose of the thing, unwilling to carry it around with you a moment longer than necessary.]])

text[14] = _([[Flintley studies the object on the table for a while, checking the online database a number of times in the process. Then, finally, he turns to you. "I hate to say this, but it seems you've bought a counterfeit. It's a good one, though! That seller on %s must have known his stuff. You see, this is very similar to a number plate used by hovercars on Mars at the time of the Second Growth. However, it's missing a number of vital characteristics, and some details betray its recent manufacture. Close, %s, close. But no cigar."
   You dispose of the counterfeit artifact. Hopefully the next one will be what Sauterfeldt is looking for...]])
   
flintdeftitle = _("Just passing through")
   
flintdeftext = _([[Flintley greets you. "Do you have any objects for me to look at, %s? No? Well, alright. I'll be here if you need me. Good luck out there."]])
   
title[10] = _("From days long gone")
   
text[15] = _([[Flintley carefully studies the object in front of him, turning it around and consulting the online database via the bar table's terminal. After several hectoseconds he leans back and whistles. "Well I never. This has to be it, %s. I'd do a carbon dating if I could, but even without I'm positive. This object dates back to pre-Growth Earth. And it's in an amazingly good condition!"
   You take another look at the thing. It resembles a small flat surface, apart from the crook at one end. On one side, there are cylindrical, solid protrusions that don't seem to serve any useful purpose at all. You are at a loss as to the artifact's purpose.
   "It's called a skate-board," Flintley continues. "The records about it are a bit sketchy and a lot is nothing but conjecture, but it appears it was once used in primitive communal rituals. The exact nature of these rituals is unknown, but they may have been tribal initiations or even mating rituals. The patterns in the board itself are thought to have a spiritual or mystical meaning. Also, according to some theories, people used to stand on top of the skate-board, with the cylinder wheels facing the ground. This has led some historians to believe that the feet were once central to human psychology."
   Flintley seems to have a lot more to say on the subject, but you're not that interested, so you thank him and return to your ship with the ancient artifact. You can only hope that the Baron is as enthusiastic about this skate-board as his historian!]])
   
title[11] = _("The Baron has his prize")
   
text[16] = _([[Baron Dovai Sauterfeldt turns the skate-board over in his hands, inspecting every nick, every scratch on the surface. His eyes are gleaming with delight.
   "Oh, this is marvelous, marvelous indeed, %s! A piece of pre-Growth history, right here in my hands! I can almost hear the echoes of that ancient civilization when I put my ear close to it! This is going to be the centerpiece in my collection of relics and artifacts. Yes indeed!
   "I was right to send you, %s, you've beautifully lived up to my expectations. And I'm a man of my word, I will reward you as promised. What was it we agreed on again? What, I never promised you anything? Well, that won't do. I'll have my assistant place a suitable amount of money in your account. You will not find me ungrateful! Ah, but you must excuse me. I need time to revel in this fantastic piece of art! Goodbye, %s, I will call on you when I have need of you again."
   You are seen out of the Baron's quarters, so you head back through the airlock and back into your own ship. The first thing you do is check your balance, and to your relief, it has indeed been upgraded by a substantial amount. As you undock, you wonder what kind of wild goose chase the man will send you on next time.]])

-- Mission details
misn_title = _("Prince")
misn_reward = _("You weren't told!")
misn_desc[1] = _("Baron Sauterfeldt has summoned you to his ship, which is in the %s system.")
misn_desc[2] = _("Baron Sauterfeldt has tasked you with finding an ancient artifact, but he doesn't know exactly where to get it.")

-- NPC stuff
npc_desc = _("An unfamiliar man")
bar_desc = _("A man you've never seen before makes eye contact with you. It seems he knows who you are.")

flint_npc1 = _("A reedy-looking man")
flint_bar1 = _("You spot a thin, nervous looking individual. He does not seem to want to be here. This could be that Flintley fellow the Baron told you about.")

flint_npc2 = _("Flintley")
flint_bar2 = _("Flintley is here. He nervously sips from his drink, clearly uncomfortable in this environment.")

sellerdesc = _("You spot a dodgy individual who matches one of the portraits in your ship's database. This must be one of the artifact sellers.")

buy = _("Buy the artifact (%s credits)")
nobuy = _("Don't buy the artifact")

nomoneytitle = _("Not enough money!")
nomoneytext = _("You can't currently afford to buy this artifact. You need %s credits.")

-- OSD stuff
osd_msg[1] = _("Fly to the %s system and dock with (board) Kahan Pinnacle")
osd_msg[2] = _("Buy the artifact and take it to Flintley")
osd_msg[3] = _("Take the artifact to Baron Sauterfeldt")

log_text = _([[Baron Sauterfeldt sent you on a wild goose chase to find some ancient artifact known as a "skate-board", which you found for him.]])


function create ()
   -- Note: this mission makes no system claims.
   misn.setNPC(npc_desc, "neutral/unique/unfamiliarman")
   misn.setDesc(bar_desc)
end

function accept()
   baronsys = system.get("Ingot")

   artifactplanetA, artifactsysA = planet.get("Varaati")
   artifactplanetB, artifactsysB = planet.get("Sinclair")
   artifactplanetC, artifactsysC = planet.get("Hurada")
   flintplanet, flintsys = planet.get("Tau Station")

   stage = 1
   
   flintleyfirst = true
   artifactsfound = 0
   
   reward = 200000 -- The price of each artifact will always be 15% of this, so at most the player will be paid 85% and at least 55%.
   
   if tk.yesno(title[1], text[1]:format(baronsys:name())) then
      misn.accept()
      tk.msg(title[2], text[2]:format(baronsys:name()))

      misn.setTitle(misn_title)
      misn.setReward(misn_reward)
      misn.setDesc(misn_desc[1]:format(baronsys:name()))
      misn.osdCreate(misn_title, { osd_msg[1]:format(baronsys:name()),
                           })
      marker = misn.markerAdd(baronsys, "low")
      
      enterhook = hook.enter("enter")
   else
      tk.msg(refusetitle, refusetext)
      abort()
   end
end

function board()
   if stage == 1 then
      tk.msg(title[4], text[4]:format(mangle(player.name()), mangle(player.name())))
      tk.msg(title[4], text[5]:format(mangle(player.name()), flintplanet:name(), flintsys:name(), mangle(player.name())))
      tk.msg(title[5], text[6])
      misn.osdCreate(misn_title, { osd_msg[1]:format(baronsys:name()),
                            osd_msg[2]
                           })
      misn.setDesc(misn_desc[2])
      misn.osdActive(2)
      
      stage = 2
      
      misn.markerRm(marker)
      markerA = misn.markerAdd(artifactsysA, "low")
      markerB = misn.markerAdd(artifactsysB, "low")
      markerC = misn.markerAdd(artifactsysC, "low")
      flintmarker = misn.markerAdd(flintsys, "high")
      
      hook.land("land")
      player.unboard()
      pinnacle:setHealth(100,100)
      idle()
   elseif stage == 2 then
      tk.msg(title[6], text[7]:format(flintplanet, flintsys))
      player.unboard()
      pinnacle:setHealth(100,100)
      idle()
   elseif stage == 3 then
      tk.msg(title[11], text[16]:format(mangle(player.name()), mangle(player.name()), mangle(player.name())))
      player.unboard()
      pinnacle:setHealth(100,100)
      pinnacle:control(false)
      player.pay(reward)
      misn.finish(true)
   end
   pinnacle:setHilight(false)
end

function land()
   if planet.cur() == artifactplanetA and not artifactA then
      sellnpc = misn.npcAdd("seller", _("Artifact seller"), getPortrait("Pirate"), sellerdesc, 4)
   elseif planet.cur() == artifactplanetB and not artifactB then
      sellnpc = misn.npcAdd("seller", _("Artifact seller"), getPortrait("Pirate"), sellerdesc, 4)
   elseif planet.cur() == artifactplanetC and not artifactC then
      sellnpc = misn.npcAdd("seller", _("Artifact seller"), getPortrait("Pirate"), sellerdesc, 4)
   elseif planet.cur() == flintplanet then
      if flintleyfirst then
         flintnpc = misn.npcAdd("flintley", flint_npc1, "neutral/unique/flintley", flint_bar1, 4)
      else
         flintnpc = misn.npcAdd("flintley", flint_npc2, "neutral/unique/flintley", flint_bar2, 4)
      end
   end
end

function flintley()
   local bingo = false

   if flintleyfirst then
      flintleyfirst = false
      tk.msg(title[7], text[8]:format(player.name()))
   elseif artifactA == nil and artifactB == nil and artifactC == nil then
      tk.msg(flintdeftitle, flintdeftext:format(player.name()))
   end
   
   if artifactA ~= nil then
      if rnd.rnd(1, 3 - artifactsfound) == 1 then
         bingo = true
      else
         tk.msg(title[9], text[12]:format(artifactplanetA:name()))
         artifactsfound = artifactsfound + 1
      end
      misn.cargoRm(artifactA)
      artifactA = nil
   end
   if artifactB ~= nil then
      if rnd.rnd(1, 3 - artifactsfound) == 1 then
         bingo = true
      else
         tk.msg(title[9], text[13]:format(artifactplanetB:name(), player.name()))
         artifactsfound = artifactsfound + 1
      end
      misn.cargoRm(artifactB)
      artifactB = nil
   end
   if artifactC ~= nil then
      if rnd.rnd(1, 3 - artifactsfound) == 1 then
         bingo = true
      else
         tk.msg(title[9], text[14]:format(artifactplanetC:name(), player.name()))
         artifactsfound = artifactsfound + 1
      end
      misn.cargoRm(artifactC)
      artifactC = nil
   end

   if bingo then
      tk.msg(title[10], text[15]:format(player.name()))
      misn.osdCreate(misn_title, { osd_msg[1]:format(baronsys:name()),
                            osd_msg[2],
                            osd_msg[3]
                           })
      misn.osdActive(3)
      stage = 3
      
      artifactReal = misn.cargoAdd("Ancient Artifact", 0)
      
      misn.markerRm(markerA)
      misn.markerRm(markerB)
      misn.markerRm(markerC)
      marker = misn.markerAdd(baronsys, "low")
      misn.markerRm(flintmarker)
   end
end

function seller()
   if planet.cur() == artifactplanetA then
      if tk.choice(title[8], text[9], buy:format(numstring(reward * 0.15)), nobuy) == 1 then
         if player.credits() >= reward * 0.15 then
            misn.npcRm(sellnpc)
            player.pay(-15000)
            artifactA = misn.cargoAdd("Artifact? A", 0)
            misn.markerRm(markerA)
         else
            tk.msg(nomoneytitle, nomoneytext:format(numstring(reward * 0.15)))
         end
      end
   elseif planet.cur() == artifactplanetB then
      if tk.choice(title[8], text[10], buy:format(numstring(reward * 0.15)), nobuy) == 1 then
         if player.credits() >= reward * 0.15 then
            misn.npcRm(sellnpc)
            player.pay(-15000)
            artifactB = misn.cargoAdd("Artifact? B", 0)
            misn.markerRm(markerB)
         else
            tk.msg(nomoneytitle, nomoneytext:format(numstring(reward * 0.15)))
         end
      end
   elseif planet.cur() == artifactplanetC then
      if tk.choice(title[8], text[11], buy:format(numstring(reward * 0.15)), nobuy) == 1 then
         if player.credits() >= reward * 0.15 then
            misn.npcRm(sellnpc)
            player.pay(-15000)
            artifactC = misn.cargoAdd("Artifact? C", 0)
            misn.markerRm(markerC)
         else
            tk.msg(nomoneytitle, nomoneytext:format(reward * 0.15))
         end
      end
   end
end

function enter()
   if system.cur() == baronsys then
      pinnacle = pilot.addRaw("Proteron Kahan", "trader", planet.get("Ulios"):pos() + vec2.new(-400,-400), "Civilian" )
      pinnacle:rename(_("Pinnacle"))
      pinnacle:setInvincible(true)
      pinnacle:setFriendly()
      pinnacle:control()
      pinnacle:setHilight(true)
      pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 500, -500), false, false)
      idlehook = hook.pilot(pinnacle, "idle", "idle")
      hhail = hook.pilot(pinnacle, "hail", "hail")
   elseif artifactA ~= nil or artifactB ~= nil or artifactC ~= nil or artifactReal ~= nil then
      -- Spawn artifact hunters, maybe.
      local choice = rnd.rnd(1, 5)
      local fleep
      local count
      local pilots = { "Hyena", "Hyena" }

      if choice == 2 then -- 60% chance of artifact hunters.
         pilots[2] = "Vendetta"
      elseif choice == 3 then
         pilots = { "Llama" }
         count = 3
      end
      if choice <= 3 then
         fleep = addRawShips( pilots, "mercenary", nil, "Mercenary", count );
         for i, j in ipairs(fleep) do
            j:control()
            j:setHostile(true)
            j:rename(_("Artifact Hunter"))
            j:attack(player.pilot())
         end
      end
   end
end

function idle()
   pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 500,  500), false)
   pinnacle:goto(planet.get("Ulios"):pos() + vec2.new(-500,  500), false)
   pinnacle:goto(planet.get("Ulios"):pos() + vec2.new(-500, -500), false)
   pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 500, -500), false)
end

function hail()
   tk.msg(title[3], text[3])
   pinnacle:taskClear()
   pinnacle:brake()
   pinnacle:setActiveBoard(true)
   boardhook = hook.pilot(pinnacle, "board", "board")
   hook.rm(idlehook)
   hook.rm(hhail)
end

-- Function that tries to misspell whatever string is passed to it.
-- TODO support unicode to a certain point
function mangle(intext)
   local outtext = intext
   
   local vowels = {"a", "e", "i", "o", "u", "y"}
   local consonants = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "z"}
   
   local i = 1
   local found = false
   
   while i < #intext - 1 do
      if isIn(intext:sub(i, i):lower(), consonants) and isIn(intext:sub(i + 1, i + 1):lower(), vowels) and isIn(intext:sub(i + 2, i + 2):lower(), consonants) then
         found = true
         break
      end
      i = i + 1
   end
   
   if found then
      local first = consonants[rnd.rnd(1, #consonants)]
      local second = vowels[rnd.rnd(1, #vowels)]
      if intext:sub(i, i):upper() == intext:sub(i, i) then first = first:upper() end -- preserve case
      if intext:sub(i + 1, i + 1):upper() == intext:sub(i, i) then second = second:upper() end -- preserve case
      outtext = intext:sub(-#intext, -(#intext - i + 2)) .. first .. second .. intext:sub(i + 2)
   end
   
   return outtext
end

-- Helper function for mangle()
function isIn(char, table)
   for _, j in pairs(table) do
      if j == char then return true end
   end
   return false
end

function abort()
   misn.finish(false)
end
