local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"

-- Chance of a jump point message showing up. As this gradually goes
-- down, it is replaced by lore messages. See spawnNPC function.
local jm_chance_min = 0
local jm_chance_max = 0.25
-- State. Nothing persists.
local msg_combined, seltargets

local msg_lore = npc.msg_lore
msg_lore["generic"] = {
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

msg_lore["Independent"] = {
   _([["We're not part of any of the galactic superpowers. We can take care of ourselves!"]]),
   _([["Sometimes I worry that our lack of a standing military leaves us vulnerable to attack. I hope nobody will get any ideas about conquering us!"]]),
}

msg_lore["Empire"] = {
   _([["Things are getting worse by the cycle. What happened to the Empire? We used to be the lords and masters over the whole galaxy!"]]),
   _([["Did you know that House Za'lek was originally a Great Project initiated by the Empire? Well, now you do! There was also a Project Proteron, but that didn't go so well."]]),
   _([["The Emperor lives on a giant supercruiser in Gamma Polaris. It's said to be the biggest ship in the galaxy! I totally want one."]]),
   _([["I'm still waiting for my pilot license application to get through. Oh well, it's only been half a cycle, I just have to be patient."]]),
   _([["Between you and me, the laws the Council passes can get really ridiculous! Most planets find creative ways of ignoring them…"]]),
   _([["Don't pay attention to the naysayers. The Empire is still strong. Have you ever seen a Peacemaker up close? I doubt any ship fielded by any other power could stand up to one."]]),
}

msg_lore["Dvaered"] = {
   _([["Our Warlord is currently fighting for control over another planet. We all support him unconditionally, of course! Of course…"]]),
   _([["My great-great-great-grandfather fought in the Dvaered Revolts! We still have the holovids he made. I'm proud to be a Dvaered!"]]),
   _([["I've got lots of civilian commendations! It's important to have commendations if you're a Dvaered."]]),
   _([["You better not mess with House Dvaered. Our military is the largest and strongest in the galaxy. Nobody can stand up to us!"]]),
   _([["House Dvaered? House? The Empire is weak and useless, we don't need them anymore! I say we declare ourselves an independent faction today. What are they going to do, subjugate us? We all know how well that went last time! Ha!"]]),
   _([["I'm thinking about joining the military. Every time I see or hear news about those rotten FLF bastards, it makes my blood boil! They should all be pounded into space dust!"]]),
   _([["FLF terrorists? I'm not too worried about them. You'll see, High Command will have smoked them out of their den soon enough, and then the Frontier will be ours."]]),
   _([["Did you know that House Dvaered was named after a hero of the revolt? At least that's what my grandparents told me."]]),
}

msg_lore["Sirius"] = {
   _([["Greetings, traveler. May Sirichana's wisdom guide you as it guides me."]]),
   _([["I once met one of the Touched in person. Well, it wasn't really a meeting, our eyes simply met… But that instant alone was awe-inspiring."]]),
   _([["They say Sirichana lives and dies like any other man, but each new Sirichana is the same as the last. How is that possible?"]]),
   _([["My cousin was called to Mutris a cycle ago… He must be in Crater City by now. And one day, he will become one of the Touched!"]]),
   _([["Some people say Sirius society is unfair because our echelons are determined by birth. But even though we are different, we are all followers of Sirichana. Spiritually, we are equal."]]),
   _([["House Sirius is officially part of the Empire, but everyone knows that's only true on paper. The Emperor has nothing to say in these systems. We follow Sirichana, and no-one else."]]),
   _([["You can easily tell the different echelons apart. Every Sirian citizen and soldier wears clothing appropriate to his or her echelon."]]),
   _([["I hope to meet one of the Touched one day!"]]),
}

msg_lore["Soromid"] = {
   _("Hello. Can I interest you in one of our galaxy famous cosmetic gene treatments? You look like you could use them…"),
   _([["Can you believe it? I was going to visit Sorom to find my roots, and then boom! It got burnt to a crisp! Even now, cycles later, I still can't believe it."]]),
   _([["Yes, it's true, our military ships are alive. Us normal folk don't get to own bioships though, we have to make do with synthetic constructs just like everyone else."]]),
   _([["Everyone knows that we Soromid altered ourselves to survive the deadly conditions on Sorom during the Great Quarantine. What you don't hear so often is that billions of us died from the therapy itself. We paid a high price for survival."]]),
   _([["Our cosmetic gene treatments are even safer now for non-Soromids, with a rate of survival of 99.4%!"]]),
   _([["We have been rebuilding and enhancing our bodies for so long, I say we've become a new species, one above human."]]),
}

msg_lore["Za'lek"] =       {
   _([["It's not easy, dancing to those scientists' tunes. They give you the most impossible tasks! Like, where am I supposed to get a triple redundant helitron converter? Honestly."]]),
   _([["The Soromids? Hah! We Za'lek are the only true scientists in this galaxy."]]),
   _([["I don't understand why we bother sending our research results to the Empire. These asshats can't understand the simplest formulas!"]]),
   _([["Do you know why many optimization algorithms require your objective function to be convex? It's not only because of the question of local minima, but also because if your function is locally concave around the current iterate, the next one will lead to a greater value of your objective function. There are still too many people who don't know this!"]]),
   _([["There are so many algorithms for solving the non-linear eigenvalues problem, I never know which one to choose. Which one do you prefer?"]]),
   _([["I recently attended a very interesting conference about the history of applied mathematics before the space age. Even in those primitive times, people used to do numerical algebra. They didn't even have quantic computers back at that time! Imagine: they had to wait for hours to solve a problem with only a dozen billion degrees of freedom!"]]),
   _([["Last time I had to solve a deconvolution problem, its condition number was so high that its inverse reached numerical zero on Octuple Precision!"]]),
   _([["I am worried about my sister. She's on trial for 'abusive self-citing' and the public prosecutor has requested a life sentence."]]),
   _([["They opened two professor positions on precision machining in Atryssa Central Manufacturing Lab, and none in Bedimann Advanced Process Lab, but everyone knows that the BAPL needs reinforcement ever since three of its professors retired last cycle. People say it's because a member of Atryssa's lab posted a positive review of the president of the Za'lek central scientific recruitment committee."]]),
   _([["Even if our labs are the best in the galaxy, other factions have their own labs as well. For example, Dvaer Prime Lab for Advanced Mace Rocket Studies used to be very successful until it was nuked by mistake by a warlord during an invasion of the planet."]]),
}

msg_lore["Thurion"] = {
   _([["Did you know that even the slightest bit of brain damage can lead to death during the upload process? That's why we're very careful to not allow our brains to be damaged, even a little."]]),
   _([["My father unfortunately hit his head when he was young, so he couldn't be safely uploaded. It's okay, though; he had a long and fulfilling life, for a non-uploaded human, that is."]]),
   _([["One great thing once you're uploaded is that you can choose to forget things you don't want to remember. My great-grandfather had a movie spoiled for him before he could watch it, so once he got uploaded, he deleted that memory and watched it with a fresh perspective. Cool, huh?"]]),
   _([["The best part of our lives is after we're uploaded, but that doesn't mean we lead boring lives before then. We have quite easy and satisfying biological lives before uploading."]]),
   _([["Being uploaded allows you to live forever, but that doesn't mean you're forced to. Any uploaded Thurion can choose to end their own life if they want, though few have chosen to do so."]]),
   _([["Uploading is a choice in our society. No one is forced to do it. It's just that, well, what kind of person would turn down the chance to live a second life on the network?"]]),
   _([["We were lucky to not get touched by the Incident. In fact, we kind of benefited from it. The nebula that resulted gave us a great cover and sealed off the Empire from us. It also got rid of those dangerous Proterons."]]),
   _([["We don't desire galactic dominance. That being said, we do want to spread our way of life to the rest of the galaxy, so that everyone can experience the joy of being uploaded."]]),
   _([["I think you're from the outside, aren't you? That's awesome! I've never met a foreigner before. What's it like outside the nebula?"]]),
   _([["We actually make occasional trips outside of the nebula, though only rarely, and we always make sure to not get discovered by the Empire."]]),
   _([["The Soromid have a rough history. Have you read up on it? First the Empire confined them to a deadly planet and doomed them to extinction. Then, when they overcame those odds, the Incident blew up their homeworld. The fact that they're still thriving now despite that is phenomenal, I must say."]]),
}

msg_lore["Proteron"] = {
   _([["Our system of government is clearly superior to all others. Nothing could be more obvious."]]),
   _([["The Incident really set back our plan for galactic dominance, but that was only temporary."]]),
   _([["We don't have time for fun and games. The whole reason we're so great is because we're more productive than any other society."]]),
   _([["We are superior, so of course we deserve control over the galaxy. It's our destiny."]]),
   _([["The Empire is weak, obsolete. That is why we must replace them."]]),
   _([["Slaves? Of course we're not slaves. Slaves are beaten and starved. We are in top shape so we can serve our country better."]]),
   _([["I can't believe the Empire continues to allow families. So primitive. Obviously, all this does is make them less productive."]]),
   _([["The exact cause of the Incident is a tightly-kept secret, but the government says it was caused by the Empire's inferiority. I would expect nothing less."]]),
   _([["I came across some heathen a few months back who claimed, get this, that we Proterons were the cause of the Incident! What slanderous nonsense. Being the perfect society we are, of course we would never cause such a massive catastrophe."]]),
}

msg_lore["Frontier"] = {
   _([["We value our autonomy. We don't want to be ruled by those Dvaered Warlords! Can't they just shoot at each other instead of threatening us? If it wasn't for the Liberation Front…"]]),
   _([["Have you studied your galactic history? The Frontier worlds were the first to be colonized by humans. That makes our worlds the oldest human settlements in the galaxy, now that Earth is gone."]]),
   _([["We have the Dvaered encroaching on our territory on one side, and the Sirius zealots on the other. Sometimes I worry that in a few decacycles, the Frontier will no longer exist."]]),
   _([["Have you visited the Frontier Museum? They've got a scale model of a First Growth colony ship on display in one of the big rooms. Even scaled down like that, it's massive! Imagine how overwhelming the real ones must have been."]]),
   _([["There are twelve true Frontier worlds, because twelve colony ships successfully completed their journey in the First Growth. But did you know that there were twenty colony ships to begin with? Eight of them never made it. Some are said to have mysteriously disappeared. I wonder what happened to them?"]]),
   _([["We don't have much here in the Frontier, other than our long history leading directly back to Earth. But I don't mind. I'm happy living here, and I wouldn't want to move anywhere else."]]),
   _([["You know the Frontier Liberation Front? They're the guerilla movement that fights for the Frontier. Not to be confused with the Liberation Front of the Frontier, the Frontier Front for Liberation, or the Liberal Frontier's Front!"]]),
}

msg_lore["FLF"] = {
   _([["I can't stand Dvaereds. I just want to wipe them all off the map. Don't you?"]]),
   _([["One of these days, we will completely rid the Frontier of Dvaered oppressors. Mark my words!"]]),
   _([["Have you ever wondered about our chances of actually winning over the Dvaereds? Sometimes I worry a little."]]),
   _([["I was in charge of a bombing run recently. The mission was a success, but I lost a lot of comrades. Oh well… this is the sacrifice we must make to resist the oppressors."]]),
   _([["What after we beat the Dvaereds, you say? Well, our work is never truly done until the Frontier is completely safe from oppression. Even if the Dvaered threat is ended, we'll still have those Sirius freaks to worry about. I don't think our job will ever end in our lifetimes."]]),
   _([["Yeah, it's true, lots of Frontier officials fund our operations. If they didn't, we'd have a really hard time landing on Frontier planets, what with the kinds of operations we perform against the Dvaereds."]]),
   _([["Yeah, some civilians die because of our efforts, but that's just a sacrifice we have to make. It's for the greater good."]]),
   _([["No, we're not terrorists. We're soldiers. True terrorists kill and destroy without purpose. Our operations do have a purpose: to drive out the Dvaered oppressors from the Frontier."]]),
   _([["Riddle me this: how can we be terrorists if the Dvaereds started it by encroaching on Frontier territory? It's the most ridiculous thing I ever heard."]]),
   _([["Well, no, the Dvaereds never actually attacked Frontier ships, but that's not the point. They have their ships in Frontier territory. What other reason could they possibly have them there for if not to set up an invasion?"]]),
}

msg_lore["Pirate"] = {
   _([["Hi mate. Money or your life! Heh heh, just messing with you."]]),
   _([["Hey, look at these new scars I got!"]]),
   _([["Have you heard of the Pirates' Code? They're more guidelines than rules…"]]),
   _([["My gran once said to me, 'Never trust a pirate.' Well, she was right! I got a pretty credit chip outta her wallet last time I saw her, and I'd do it again."]]),
   _([["I don't understand why some pirates talk like 16th-century Earth pirates even though that planet is literally dead."]]),
   _([["I may be a pirate who blows up ships and steals for a living, but that inner nebula still kind of freaks me out."]]),
   _([["Damn Empire stopped my heist a few decaperiods ago. Just wait'll they see me again…"]]),
   _([["There's a pirate clanworld I really wanted to get to, but they wouldn't let me in because I'm a 'small-time pirate'! Sometimes I think I'll never make it in this line of work…"]]),
   _([["Just caught an old mate ferrying tourists for credits. Nearly puked out my grog! Your reputation won't survive for long working for our victims."]]),
   _([["I was around before Haven was destroyed, you know! Funny times. All the pirates were panicking and the Empire was cheering thinking that we were done for. Ha! As if! It barely even made a difference. We just relocated to New Haven and resumed business as usual."]]),
   _([["Y'know, I got into this business by accident to tell the truth. But what can you do? I could get a fake ID and pretend to be someone else but I'd get caught eventually and I'd lose my fame as a pirate."]]),
   _([["One of my favourite things to do is buy a fake ID and then deliver as much contraband as I can before I get caught. It's great fun, and finding out that my identity's been discovered gives me a rush!"]]),
   _([["Back when I started out in this business all you could do was go around delivering packages for other people. Becoming a pirate was real hard back then, but I got so bored I spent several decaperiods doing it. Nowadays things are way more exciting for normies, but I don't regret my choice one bit!"]]),
   _([["Flying a real big ship is impressive, but it's still no pirate ship. I mean, I respect ya more if you're flying a Goddard than if you're flying a civilian Lancelot, but the best pirates fly the good old Pirate Kestrel!"]]),
}

msg_lore["Trader"] = {
   _([["Just another link in the Great Chain, right?"]]),
   _([["You win some, you lose some, but if you don't try you're never going to win."]]),
   _([["If you don't watch the markets then you'll be hopping between planets in a jury-rigged ship in no time."]]),
   _([["Them blimming pirates, stopping honest folk from making an honest living - it's not like we're exploiting the needy!"]]),
}


-- Returns a lore message for the given faction.
local function getLoreMessage( fac )
   -- Select the faction messages for this NPC's faction, if it exists.
   local facmsg = msg_lore[fac]
   if facmsg == nil or #facmsg == 0 then
      facmsg = msg_lore["generic"]
      if facmsg == nil or #facmsg == 0 then
         return
      end
   end

   -- Select a string, then remove it from the list of valid strings. This ensures all NPCs have something different to say.
   local r = rnd.rnd(1, #facmsg)
   local pick = facmsg[r]
   table.remove(facmsg, r)
   return pick
end

-- Returns a jump point message and updates jump point known status accordingly. If all jumps are known by the player, defaults to a lore message.
local function getJmpMessage( fac )
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
   if #npc.msg_jmp == 0 then
      return getLoreMessage(fac), nil
   end
   local retmsg =  npc.msg_jmp[rnd.rnd(1, #npc.msg_jmp)]
   local sel = rnd.rnd(1, #mytargets)
   local myfunc = function( npcdata )
      if not npcdata.talked then
         mytargets[sel]:setKnown(true)
         mytargets[sel]:dest():setKnown(true, false)

         -- Reduce jump message chance
         local jm_chance = var.peek("npc_jm_chance") or jm_chance_max
         var.push( "npc_jm_chance", math.max( jm_chance - 0.025, jm_chance_min ) )
         npcdata.talked = true
      end
   end

   -- Don't need to remove messages from tables here, but add whatever jump point we selected to the "selected" table.
   seltargets[mytargets[sel]] = true
   return fmt.f( retmsg, {jmp=mytargets[sel]:dest()} ), myfunc
end

-- Returns a tip message.
local function getTipMessage( fac )
   -- All tip messages are valid always.
   if #npc.msg_tip == 0 then
      return getLoreMessage(fac)
   end
   local sel = rnd.rnd(1, #npc.msg_tip)
   local pick = npc.msg_tip[sel]
   table.remove(npc.msg_tip, sel)
   return pick
end

-- Returns a mission hint message, a mission after-care message, OR a lore message if no missionlikes are left.
local function getMissionLikeMessage( fac )
   if not msg_combined then
      msg_combined = {}

      -- Hints.
      -- Hint messages are only valid if the relevant mission has not been completed and is not currently active.
      for i, j in pairs(npc.msg_mhint) do
         if not (player.misnDone(j[1]) or player.misnActive(j[1])) then
            msg_combined[#msg_combined + 1] = j[2]
         end
      end
      for i, j in pairs(npc.msg_ehint) do
         if not(player.evtDone(j[1]) or player.evtActive(j[1])) then
            msg_combined[#msg_combined + 1] = j[2]
         end
      end

      -- After-care.
      -- After-care messages are only valid if the relevant mission has been completed.
      for i, j in pairs(npc.msg_mdone) do
         if player.misnDone(j[1]) then
            msg_combined[#msg_combined + 1] = j[2]
         end
      end
      for i, j in pairs(npc.msg_edone) do
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

-- Factions which will NOT get generic texts if possible.  Factions
-- listed here not spawn generic civilian NPCs or get aftercare texts.
-- Meant for factions which are either criminal (FLF, Pirate) or unaware
-- of the main universe (Thurion, Proteron).
local nongeneric_factions = {
   "Pirate",
   "Marauder",
   "Wild Ones",
   "Raven Clan",
   "Dreamer Clan",
   "Black Lotus",
   "FLF",
   "Thurion",
   "Proteron"
}

-- List to treat special factions differently
local override_list = {
   -- Treat pirate clans the same (at least for now)
   ["Wild Ones"]     = "Pirate",
   ["Raven Clan"]    = "Pirate",
   ["Dreamer Clan"]  = "Pirate",
   ["Black Lotus"]   = "Pirate",
   ["Marauder"]      = "Pirate",
}

return function ()
   local cur, scur = spob.cur()
   local presence = scur:presences()["Independent"] or 0

   -- Need independent presence in the system
   if presence < 0 then
      return nil
   end

   -- Don't appear on restricted assets
   if cur:tags().restricted then
      return nil
   end

   local function gen_npc()
      -- Choose faction, overriding if necessary
      local f = cur:faction()
      if not f then return nil end
      local of = override_list[f:nameRaw()]
      if of then f = faction.get(of) end

      local nongeneric = false
      local planfaction = (f ~= nil and f:nameRaw()) or nil
      local fct = nil
      local sel = rnd.rnd()
      if planfaction ~= nil then
         for i, j in ipairs(nongeneric_factions) do
            if j == planfaction then
               nongeneric = true
               break
            end
         end

         if nongeneric or sel >= 0.5 then
            fct = planfaction
         end
      end
      -- Empire is handled differently now
      if fct == "Empire" then fct = nil end

      -- Append the faction to the civilian name, unless there is no faction.
      local name
      if not fct then
         name = _("Civilian")
      else
         name = fmt.f( _("{fct} Civilian"), {fct=_(fct)} )
      end
      local desc = npc.desc_list[ rnd.rnd(1,#npc.desc_list) ]
      local prt  = portrait.get( fct )
      local image = portrait.getFullPath( prt )
      local msg, func
      local r = rnd.rnd()

      if r < (var.peek("npc_jm_chance") or jm_chance_max) then
         -- Jump point message.
         msg, func = getJmpMessage(fct)
      elseif r <= 0.55 then
         -- Lore message.
         msg = getLoreMessage(fct)
      elseif r <= 0.8 then
         -- Gameplay tip message.
         msg = getTipMessage(fct)
      else
         -- Mission hint message.
         if not nongeneric then
            msg = getMissionLikeMessage(fct)
         else
            msg = getLoreMessage(fct)
         end
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg, func=func }
   end

   return { create=gen_npc }
end
