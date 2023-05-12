--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Prince">
 <unique />
 <priority>4</priority>
 <done>Baron</done>
 <chance>2</chance>
 <location>Bar</location>
 <faction>Empire</faction>
 <faction>Dvaered</faction>
 <faction>Goddard</faction>
 <faction>Sirius</faction>
 <notes>
  <campaign>Baron Sauterfeldt</campaign>
 </notes>
</mission>
--]]
--[[
-- This is the second mission in the baron string.
--]]

local fleet = require "fleet"
local fmt = require "format"
local portrait = require "portrait"
local baron = require "common.baron"

-- Mission constants
local baronsys = system.get("Ingot")
local artefactplanetA = spob.get("Varaati")
local artefactplanetB = spob.get("Sinclair")
local artefactplanetC = spob.get("Hurada")
local flintplanet, flintsys = spob.getS("Tau Station")
local reward = baron.rewards.prince

local isIn, mangle -- Forward-declared functions
local pinnacle -- Non-persistent state

function create ()
   -- Note: this mission makes no system claims.
   misn.setNPC(_("An unfamiliar man"), "neutral/unique/unfamiliarman.webp", _("A man you've never seen before makes eye contact with you. He seems to know who you are."))
end

function accept()
   mem.stage = 1

   mem.flintleyfirst = true
   mem.artifactsfound = 0


   if tk.yesno(_("His Baronship remembers you"), fmt.f(_([[As you approach the stranger, he extends his hand in greeting. He introduces himself as an associate of Baron Sauterfeldt, the man you helped to "acquire" a holopainting not too long ago.
    "The Baron was quite pleased with your performance in that matter," he confides. "He has asked me to try to find you again for another job not unlike the last one. The Baron is a collector, you see, and his hunger for new possessions is a hard one to satiate." He makes a face. "Of course, his methods aren't always completely respectable, as you've experienced for yourself. But I assure you that the Baron is not a bad man, he is simply very enthusiastic."
    You decide to keep your opinion of the aristocrat to yourself. Instead you inquire as to what the man wants from you this time. "To tell the truth, I don't actually know," the man says. "The Baron wants you to meet him so he can brief you in person. You will find his ship in the {sys} system. Shall I inform his lordship that you will be paying him a visit?"]]), {sys=baronsys})) then
      misn.accept()
      tk.msg(_("At your beck and call"), fmt.f(_([["Splendid. Please go see his lordship at the earliest opportunity. He doesn't like to be kept waiting. I will send word that you will be coming, so contact the Pinnacle when you arrive at {sys}, and they will allow you to board."]]), {sys=baronsys}))

      misn.setTitle(_("Prince"))
      misn.setReward(_("You weren't told!"))
      misn.setDesc(fmt.f(_("Baron Sauterfeldt has summoned you to his ship, which is in the {sys} system."), {sys=baronsys}))
      misn.osdCreate(_("Prince"), {
         fmt.f(_("Fly to the {sys} system and dock with (board) Gauss Pinnacle"), {sys=baronsys}),
      })
      mem.marker = misn.markerAdd(baronsys, "low")

      mem.enterhook = hook.enter("enter")
   else
      tk.msg(_("Sorry, not today"), _([["Okay, fair enough," the man says with a disappointed look on his face. "I'll try to find someone else. But maybe we'll run into each other again, so if you change your mind…"]]))
      return
   end
end

function board()
   if mem.stage == 1 then
      tk.msg(_("An audience with the Baron"), fmt.f(_([[You find yourself once again aboard the Pinnacle, Baron Sauterfeldt's flag ship. After a short time, an attendant ushers you into the Baron's personal quarters, which are as extravagant as you remember them. You notice the holopainting is now firmly fixed on one of the walls.
    Baron Dovai Sauterfeldt greets you with a pompous wave of his hand. "Ahh yes, there you are at last. {wrongname1}, was it? Do have a seat." He then offers you a drink, but you decline on the basis that you still have to drive. "Now then, {wrongname2}, I assume you're wondering why I've called you here. As you've no doubt heard, I have an interest in the unique, the exquisite." The Baron gestures around the room. "I have built up quite an impressive collection, as you can see, but it is still lacking something. Fortunately, news has reached me about a priceless artefact from Earth itself, dating back to before the Faction Wars. I must have it. It belongs in the hands of a connoisseur like myself."]]), {wrongname1=mangle(player.name()), wrongname2=mangle(player.name())}))
      tk.msg(_("An audience with the Baron"), fmt.f(_([["Unfortunately, news of this artefact has reached more ears than just mine. All over the galaxy there are people who will try to sell you 'ancient artefacts', which always turn out to be imitations at best and worthless scrap they picked up from the streets at worst." The Baron snorts derisively. "Even the contacts who usually fenc- ah, I mean, supply me with new items for my collection are in on the frenzy.
    "I've narrowed down my search to three of these people. I'm confident that one of them is selling the genuine article, while the other two are shams. And this is where you come in, {wrongname1}. I want you to visit these vendors, buy their wares off them and bring me the authentic artefact. You will have the help of a man named Flintley, who is a history buff or some such rot. You will find him on {pnt} in the {sys} system. Simply tell him you're working for me and show him any artefacts in your possession. He will tell you which are authentic and which are fake.
    "I should warn you, {wrongname2}. Some of my, ah, colleagues have also set their sights on this item, and so you can expect their henchmen to try to take it off you. I trust you are equipped to defend yourself against their despicable sort."]]), {wrongname1=mangle(player.name()), pnt=flintplanet, sys=flintsys, wrongname2=mangle(player.name())}))
      tk.msg(_("Off to the shops"), _([[You are swiftly escorted back to your ship. You didn't really get the chance to ask the Baron any questions, such as who these potential attackers are, how you're supposed to pay for the artefacts once you locate the sellers, or what you will get out of all this. You do, however, find an update to your galaxy map that shows the location of the sellers, as well as a list of names and portraits. It would seem that the only way to find out what you're dealing with is the hard way.]]))
      misn.osdCreate(_("Prince"), {
         fmt.f(_("Fly to the {sys} system and dock with (board) Gauss Pinnacle"), {sys=baronsys}),
         _("Buy the artefact and take it to Flintley"),
      })
      misn.setDesc(_("Baron Sauterfeldt has tasked you with finding an ancient artefact, but he doesn't know exactly where to get it."))
      misn.osdActive(2)

      mem.stage = 2

      misn.markerRm(mem.marker)
      mem.markerA = misn.markerAdd(artefactplanetA, "low")
      mem.markerB = misn.markerAdd(artefactplanetB, "low")
      mem.markerC = misn.markerAdd(artefactplanetC, "low")
      mem.flintmarker = misn.markerAdd(flintplanet, "high")

      hook.land("land")
      player.unboard()
      pinnacle:setHealth(100,100)
      idle()
   elseif mem.stage == 2 then
      tk.msg(_("What are you doing here?"), fmt.f(_([[You have not yet collected and identified the genuine artefact. Buy the artefacts from the sellers and visit Flintley on {pnt} ({sys}) to identify the real one.]]), {pnt=flintplanet, sys=flintsys}))
      player.unboard()
      pinnacle:setHealth(100,100)
      idle()
   elseif mem.stage == 3 then
      tk.msg(_("The Baron has his prize"), fmt.f(_([[Baron Dovai Sauterfeldt turns the skate-board over in his hands, inspecting every nick, every scratch on the surface. His eyes are gleaming with delight.
    "Oh, this is marvelous, marvelous indeed, {wrongname1}! A piece of pre-Growth history, right here in my hands! I can almost hear the echoes of that ancient civilization when I put my ear close to it! This is going to be the centrepiece in my collection of relics and artefacts. Yes indeed!
    "I was right to send you, {wrongname2}, you've beautifully lived up to my expectations. And I'm a man of my word, I will reward you as promised. What was it we agreed on again? What, I never promised you anything? Well, that won't do. I'll have my assistant place a suitable amount of money in your account. You will not find me ungrateful! Ah, but you must excuse me. I need time to revel in this fantastic piece of art! Goodbye, {wrongname3}, I will call on you when I have need of you again."
    You are seen out of the Baron's quarters, so you head back through the airlock and back into your own ship. The first thing you do is check your balance, and to your relief, it has indeed been upgraded by a substantial amount. You also seem to have received a statue of dubious taste. As you undock, you wonder what kind of wild goose chase the man will send you on next time.]]), {wrongname1=mangle(player.name()), wrongname2=mangle(player.name()), wrongname3=mangle(player.name())}))
      player.unboard()
      pinnacle:setHealth(100,100)
      pinnacle:control(false)
      player.pay(reward)
      player.outfitAdd("Ugly Statue")
      baron.addLog( _([[Baron Sauterfeldt sent you on a wild goose chase to find some ancient artefact known as a "skate-board", which you found for him.]]) )
      misn.finish(true)
   end
   pinnacle:setHilight(false)
end

function land()
   local sellerdesc = _("You spot a dodgy individual who matches one of the portraits in your ship's database. This must be one of the artefact sellers.")
   if spob.cur() == artefactplanetA and not mem.artifactA then
      mem.sellnpc = misn.npcAdd("seller", _("Artefact seller"), portrait.get("Pirate"), sellerdesc, 4)
   elseif spob.cur() == artefactplanetB and not mem.artifactB then
      mem.sellnpc = misn.npcAdd("seller", _("Artefact seller"), portrait.get("Pirate"), sellerdesc, 4)
   elseif spob.cur() == artefactplanetC and not mem.artifactC then
      mem.sellnpc = misn.npcAdd("seller", _("Artefact seller"), portrait.get("Pirate"), sellerdesc, 4)
   elseif spob.cur() == flintplanet then
      if mem.flintleyfirst then
         misn.npcAdd("flintley", _("A reedy-looking man"), "neutral/unique/flintley.webp", _("You spot a thin, nervous looking individual. He does not seem to want to be here. This could be that Flintley fellow the Baron told you about."), 4)
      else
         misn.npcAdd("flintley", _("Flintley"), "neutral/unique/flintley.webp", _("Flintley is here. He nervously sips from his drink, clearly uncomfortable in this environment."), 4)
      end
   end
end

function flintley()
   local bingo = false

   if mem.flintleyfirst then
      mem.flintleyfirst = false
      tk.msg(_("Flintley, at your service"), fmt.f(_([[You approach the nervous-looking man and inquire if he is Flintley, the historian in Baron Sauterfeldt's employ.
    "Oh, yes. Yes! That is me! I'm Flintley," the man responds. "And you must be {player}. I know what's going on, the people from the Pinnacle have informed me. Oh, but where are my manners. Let me properly introduce myself. My name is Flintley, and I'm an archaeologist and historian. The best in the galaxy, some might say, ha-ha!" He gives you a look. "Well, maybe not. But I'm quite knowledgeable about the history of the galaxy. Too bad not too many people seem interested in that these days. The only work I can really get is the occasional appraisal, like I'm doing now for his lordship. I wish I didn't have to take jobs like this, but there you have it."
    Flintley sighs. "Well, that's that. Come to me with any artefacts you manage to procure, and I'll analyse them to the best of my ability."]]), {player=player.name()}))
   elseif mem.artifactA == nil and mem.artifactB == nil and mem.artifactC == nil then
      tk.msg(_("Just passing through"), fmt.f(_([[Flintley greets you. "Do you have any objects for me to look at, {player}? No? Well, alright. I'll be here if you need me. Good luck out there."]]), {player=player.name()}))
   end

   if mem.artifactA ~= nil then
      if rnd.rnd(1, 3 - mem.artifactsfound) == 1 then
         bingo = true
      else
         tk.msg(_("This is not the artefact you're looking for"), fmt.f(_([["Let's see what we have here," Flintley says as you hand him the artefact you bought on {pnt}. "Ah, I know what this is without even looking anything up. It's a piece of an old-fashioned airlock mechanism, as used on most ships during the Faction Wars. That makes it rather old, but that also makes it worthless, I'm afraid. This is just old scrap." He gives you an apologetic look. "Don't let it get you down. Not many people would know this on first sight. Those scammers can be pretty clever."
    You feel disappointed and frustrated, but you have no choice but to deposit the "artefact" into the nearest disintegrator inlet.]]), {pnt=artefactplanetA}))
         mem.artifactsfound = mem.artifactsfound + 1
      end
      misn.cargoRm(mem.artifactA)
      mem.artifactA = nil
   end
   if mem.artifactB ~= nil then
      if rnd.rnd(1, 3 - mem.artifactsfound) == 1 then
         bingo = true
      else
         tk.msg(_("This is not the artefact you're looking for"), fmt.f(_([[You hand Flintley the artefact you procured on {pnt}. He examines it for a few moments, then enters a few queries in the info terminal in his table. Once he has found what he was looking for, he heaves a sigh. "I'm sorry, {player}. It seems you've been had. What you've got here is little more than a trinket. It's a piece of 'art' created by a third-rank sculptress named Biena Gharibri who lives on Lapra. She's not very talented, I'm afraid. Her creations have been called 'worse than Dvaered opera' by a leading art critic. I really don't think you want to present his lordship with this."
    You promptly decide to dispose of the thing, unwilling to carry it around with you a moment longer than necessary.]]), {pnt=artefactplanetB, player=player.name()}))
         mem.artifactsfound = mem.artifactsfound + 1
      end
      misn.cargoRm(mem.artifactB)
      mem.artifactB = nil
   end
   if mem.artifactC ~= nil then
      if rnd.rnd(1, 3 - mem.artifactsfound) == 1 then
         bingo = true
      else
         tk.msg(_("This is not the artefact you're looking for"), fmt.f(_([[Flintley studies the object on the table for a while, checking the online database a number of times in the process. Then, finally, he turns to you. "I hate to say this, but it seems you've bought a counterfeit. It's a good one, though! That seller on {pnt} must have known his stuff. You see, this is very similar to a number plate used by hovercars on Mars at the time of the Second Growth. However, it's missing a number of vital characteristics, and some details betray its recent manufacture. Close, {player}, close. But no cigar."
    You dispose of the counterfeit artefact. Hopefully the next one will be what Sauterfeldt is looking for…]]), {pnt=artefactplanetC, player=player.name()}))
         mem.artifactsfound = mem.artifactsfound + 1
      end
      misn.cargoRm(mem.artifactC)
      mem.artifactC = nil
   end

   if bingo then
      tk.msg(_("From days long gone"), fmt.f(_([[Flintley carefully studies the object in front of him, turning it around and consulting the online database via the bar table's terminal. After several hectoseconds he leans back and whistles. "Well I never. This has to be it, {player}. I'd do a carbon dating if I could, but even without I'm positive. This object dates back to pre-Growth Earth. And it's in an amazingly good condition!"
    You take another look at the thing. It resembles a small flat surface, apart from the crook at one end. On one side, there are cylindrical, solid protrusions that don't seem to serve any useful purpose at all. You are at a loss as to the artefact's purpose.
    "It's called a skate-board," Flintley continues. "The records about it are a bit sketchy and a lot is nothing but conjecture, but it appears it was once used in primitive communal rituals. The exact nature of these rituals is unknown, but they may have been tribal initiations or even mating rituals. The patterns in the board itself are thought to have a spiritual or mystical meaning. Also, according to some theories, people used to stand on top of the skate-board, with the cylinder wheels facing the ground. This has led some historians to believe that the feet were once central to human psychology."
    Flintley seems to have a lot more to say on the subject, but you're not that interested, so you thank him and return to your ship with the ancient artefact. You can only hope that the Baron is as enthusiastic about this skate-board as his historian!]]), {player=player.name()}))
      misn.osdCreate(_("Prince"), {
         fmt.f(_("Fly to the {sys} system and dock with (board) Gauss Pinnacle"), {sys=baronsys}),
         _("Buy the artefact and take it to Flintley"),
         _("Take the artefact to Baron Sauterfeldt"),
      })
      misn.osdActive(3)
      mem.stage = 3

      local c = commodity.new( N_("Ancient Artefact"), N_("A seemingly ancient artefact.") )
      mem.artifactReal = misn.cargoAdd(c, 0)

      misn.markerRm(mem.markerA)
      misn.markerRm(mem.markerB)
      misn.markerRm(mem.markerC)
      mem.marker = misn.markerAdd(baronsys, "low")
      misn.markerRm(mem.flintmarker)
   end
end

function seller()
   local option_yes = fmt.f(_("Buy the artefact ({credits})"), {credits=fmt.credits(reward * 0.15)})
   local option_no = _("Don't buy the artefact")
   if spob.cur() == artefactplanetA then
      if tk.choice(_("A suspicious salesman"), _([["Hello there," the guy says to you when you approach. "Can I interest you in this bona fide relic from an ancient past? Unlike all those scammers out there, I offer you the real deal, no fakes here!"]]), option_yes, option_no) == 1 then
         if player.credits() >= reward * 0.15 then
            misn.npcRm(mem.sellnpc)
            player.pay( -reward * 0.15 )
            local c = commodity.new( N_("Artefact? A"), N_("An ancient artefact?") )
            mem.artifactA = misn.cargoAdd(c, 0)
            misn.markerRm(mem.markerA)
         else
            tk.msg(_("Not enough money!"), fmt.f(_("You can't currently afford to buy this artefact. You need {credits}."), {credits=fmt.credits(reward * 0.15)}))
         end
      end
   elseif spob.cur() == artefactplanetB then
      if tk.choice(_("A suspicious salesman"), _([[The man grins at you. "Ah, I can tell you have the eye of a connoisseur! I deal only in the finest, counterfeit-free antiques. If you're smart, and I can see that you are, you won't trust all those opportunists who will try to sell you fakes! How about it?"]]), option_yes, option_no) == 1 then
         if player.credits() >= reward * 0.15 then
            misn.npcRm(mem.sellnpc)
            player.pay( -reward * 0.15 )
            local c = commodity.new( N_("Artefact? B"), N_("An ancient artefact?") )
            mem.artifactB = misn.cargoAdd(c, 0)
            misn.markerRm(mem.markerB)
         else
            tk.msg(_("Not enough money!"), fmt.f(_("You can't currently afford to buy this artefact. You need {credits}."), {credits=fmt.credits(reward * 0.15)}))
         end
      end
   elseif spob.cur() == artefactplanetC then
      if tk.choice(_("A suspicious salesman"), _([[The woman beckons you over to the bar. "Listen, friend. I have here a unique, extremely rare remnant of prehistoric times. This is the genuine article, trust me on that. One hundred per cent legit! And you wouldn't want to spend good credits on a fake, right?"]]), option_yes, option_no) == 1 then
         if player.credits() >= reward * 0.15 then
            misn.npcRm(mem.sellnpc)
            player.pay( -reward * 0.15 )
            local c = commodity.new( N_("Artefact? C"), N_("An ancient artefact?") )
            mem.artifactC = misn.cargoAdd(c, 0)
            misn.markerRm(mem.markerC)
         else
            tk.msg(_("Not enough money!"), fmt.f(_("You can't currently afford to buy this artefact. You need {credits}."), {credits=fmt.credits(reward * 0.15)}))
         end
      end
   end
end

function enter()
   if system.cur() == baronsys then
      pinnacle = pilot.add("Proteron Gauss", "Independent", spob.get("Ulios"):pos() + vec2.new(-400,-400), _("Pinnacle"), {ai="trader"} )
      pinnacle:setInvincible(true)
      pinnacle:setFriendly()
      pinnacle:control()
      pinnacle:setHilight(true)
      pinnacle:moveto(spob.get("Ulios"):pos() + vec2.new( 500, -500), false, false)
      mem.idlehook = hook.pilot(pinnacle, "idle", "idle")
      mem.hhail = hook.pilot(pinnacle, "hail", "hail")
   elseif mem.artifactA ~= nil or mem.artifactB ~= nil or mem.artifactC ~= nil or mem.artifactReal ~= nil then
      -- Spawn artefact hunters, maybe.
      local choice = rnd.rnd(1, 5)
      local fleep
      local count
      local pilots = { "Hyena", "Hyena" }

      if choice == 2 then -- 60% chance of artefact hunters.
         pilots[2] = "Vendetta"
      elseif choice == 3 then
         pilots = { "Llama" }
         count = 3
      end
      if choice <= 3 then
         fleep = fleet.add( count, pilots, "Mercenary", nil, _("Artefact Hunter"), {ai="baddiepos"} )
         for i, j in ipairs(fleep) do
            j:setHostile(true)
            j:memory().guardpos = player.pos() -- Just go towards the player position and attack if they are around
         end
      end
   end
end

function idle()
   pinnacle:moveto(spob.get("Ulios"):pos() + vec2.new( 500,  500), false)
   pinnacle:moveto(spob.get("Ulios"):pos() + vec2.new(-500,  500), false)
   pinnacle:moveto(spob.get("Ulios"):pos() + vec2.new(-500, -500), false)
   pinnacle:moveto(spob.get("Ulios"):pos() + vec2.new( 500, -500), false)
end

function hail()
   tk.msg(_("Green light for docking"), _([[Your comm is answered by a communications officer on the bridge of the Pinnacle. You tell her you've got a delivery for the baron. She runs a few checks on a console off the screen, then tells you you've been cleared for docking and that the Pinnacle will be brought to a halt.]]))
   pinnacle:taskClear()
   pinnacle:brake()
   pinnacle:setActiveBoard(true)
   mem.boardhook = hook.pilot(pinnacle, "board", "board")
   hook.rm(mem.idlehook)
   hook.rm(mem.hhail)
   player.commClose()
end

-- Function that tries to misspell whatever string is passed to it.
-- TODO support Unicode to a certain point
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
