local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"
local pir = require "common.pirate"

-- Chance of a jump point message showing up. As this gradually goes
-- down, it is replaced by lore messages. See spawnNPC function.
local jm_chance_min = 0
local jm_chance_max = 0.25
-- State. Nothing persists.
local msg_combined, seltargets

local gfx_list = {
   "neutral/female1.webp",
   "neutral/female2.webp",
   "neutral/female2n_nogog.webp",
   "neutral/female3.webp",
   "neutral/female4.webp",
   "neutral/male1n.webp",
}

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

msg_lore["Trader"] = {
   _([["Just another link in the Great Chain, right?"]]),
   _([["You win some, you lose some, but if you don't try you're never going to win."]]),
   _([["If you don't watch the markets then you'll be hopping between planets in a jury-rigged ship in no time."]]),
   _([["Them blimming pirates, stopping honest folk from making an honest living - it's not like we're exploiting the needy!"]]),
}


-- Returns a lore message for the given faction.
local function getMessageLore( fct )
   local fctmsg = msg_lore[fct]
   if fctmsg == nil or #fctmsg == 0 then
      fctmsg = msg_lore["generic"]
   end
   return fctmsg[ rnd.rnd(1, #fctmsg) ]
end

-- Returns a jump point message and updates jump point known status accordingly. If all jumps are known by the player, defaults to a lore message.
local function getMessageJump( fct )
   -- Collect a table of jump points in the system the player does NOT know.
   local mytargets = {}
   seltargets = seltargets or {} -- We need to keep track of jump points NPCs will tell the player about so there are no duplicates.
   for _, j in ipairs(system.cur():jumps(true)) do
      if not j:known() and not j:hidden() and not seltargets[j] then
         table.insert(mytargets, j)
      end
   end

   -- The player already knows all jumps in this system or no messages
   if #mytargets == 0 or #npc.msg_jmp==0 then
      return getMessageLore( fct )
   end

   local retmsg =  npc.msg_jmp[rnd.rnd(1, #npc.msg_jmp)]
   local sel = rnd.rnd(1, #mytargets)
   local myfunc = function( npcdata )
      if npcdata.talked then
         return
      end
      mytargets[sel]:setKnown(true)
      mytargets[sel]:dest():setKnown(true, false)

      -- Reduce jump message chance
      local jm_chance = var.peek("npc_jm_chance") or jm_chance_max
      var.push( "npc_jm_chance", math.max( jm_chance - 0.025, jm_chance_min ) )
      npcdata.talked = true
   end

   -- Don't need to remove messages from tables here, but add whatever jump point we selected to the "selected" table.
   seltargets[mytargets[sel]] = true
   return fmt.f( retmsg, {jmp=mytargets[sel]:dest()} ), myfunc
end

local function getMessage( lst, fct )
   if #lst == 0 then
      return getMessageLore( fct )
   end
   return lst[ rnd.rnd(1, #lst) ]
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

return function ()
   local cur, scur = spob.cur()
   local presence = scur:presences()["Independent"] or 0

   -- Need independent presence in the system
   if presence <= 0 then
      return nil
   end

   -- Don't appear on restricted assets
   if cur:tags().restricted then
      return nil
   end

   -- Create a list of conditional messages
   msg_combined = npc.combine_cond( npc.msg_cond )

   local function gen_npc()
      -- Choose faction, overriding if necessary
      local f = cur:faction()
      if not f then return nil end

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
      -- Some factions are handled differently now
      if fct == "Empire" then fct = nil end
      if fct == "Za'lek" then fct = nil end
      if fct == "Dvaered" then fct = nil end
      if fct == "Soromid" then fct = nil end
      if pir.factionIsPirate( fct ) then return nil end

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
      -- TODO make this more proper
      if not fct and rnd.rnd() < 0.3 then
         prt = gfx_list[ rnd.rnd(1,#gfx_list) ]
         image = prt
      end
      local msg, func
      local r = rnd.rnd()

      if r < (var.peek("npc_jm_chance") or jm_chance_max) then
         -- Jump point message.
         msg, func = getMessageJump( fct )
      elseif r <= 0.55 then
         -- Lore message.
         msg = getMessageLore( fct )
      elseif r <= 0.8 then
         -- Gameplay tip message.
         msg = getMessage( npc.msg_tip, fct )
      else
         -- Mission hint message.
         if not nongeneric then
            msg = getMessage( msg_combined, fct )
         else
            msg = getMessageLore( fct )
         end
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg, func=func }
   end

   return { create=gen_npc }
end
