local fmt = require "format"
local npc = require "common.npc"
local vni = require "vnimage"

-- Chance of a jump point message showing up. As this gradually goes
-- down, it is replaced by lore messages. See spawnNPC function.
local jm_chance_min = 0
local jm_chance_max = 0.25
-- State. Nothing persists.
local msg_combined

local desc_list = {
   _("This person seems to be here to relax."),
   _("There is a civilian sitting on one of the tables."),
   _("There is a civilian sitting there, looking somewhere else."),
   _("A worker sits at one of the tables, wearing a name tag saying \"Go away\"."),
   _("A civilian sits at the bar, seemingly serious about the cocktails on offer."),
   _("A civilian wearing a shirt saying: \"Ask me about Jaegnhild\""),
   _("There is a civilian sitting in the corner."),
   _("A civilian feverishly concentrating on a fluorescing drink."),
   _("A civilian drinking alone."),
   _("This person seems friendly enough."),
   _("A civilian sitting at the bar."),
   _("This person is idly browsing a news terminal."),
   _("A worker sips a cold drink after a hard shift."),
   _("A worker slouched against the bar, nursing a drink."),
   _("This worker seems bored with everything but their drink."),
}

local msg_lore = {
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
   _([["I used to dream of being a racer at Melendez Station, but my reflexes are just not up to the job."]]),
}

local msg_cond = {
   -- Chapter stuff
   { function () return (player.chapter()=="0") end, _([["There's talk about all these megastructures built around the universe. I call bollocks, there's no way people could work together to build something so impressive. Everyone is always fighting!"]]) },
   { function () return (player.chapter()=="1") end, _([["Have you seen the new hypergates? Maybe this is the start of all the Great houses putting their differences aside and a new future for a unified humanity!"]]) }, -- Narrator: it was not
   -- Mission Hints
   {npc.test_misnHint("Shadowrun"), _([["Apparently there's a woman who regularly turns up on planets in and around the Klantar system. I wonder what she's looking for?"]])},
   {npc.test_misnHint("Hitman"), _([["There are often shady characters hanging out in the Alteris system. I'd stay away from there if I were you, someone might offer you a dirty kind of job!"]])},
   -- Event hints
   {npc.test_evtHint("FLF/DV Derelicts"), _([["The FLF and the Dvaered sometimes clash in Surano. If you go there, you might find something of interest… Or not."]])},
   {npc.test_evtHint("Introducing Taiomi", function () return (player.chapter()~="0") end ), _([["I've heard that there are ghosts to the north of Dune. They seem to be around asteroid fields. As if such a thing could exist! Probably just a brave miner in a Llama!"]])},
   {npc.test_evtHint("Levo Pirates", _([["My sister claims there is a pirate blockade going on in the Levo system. I thought the Empire was supposed to bring peace and prosperity, not pirates and more pirates!"]]))},
   {npc.test_evtHint("Dendria Pirates", _([["I tried to go through Dendria system once and some scary pirate was claiming to own the system and wanted me to pay a lot of money! Never going back there again."]]))},
   {npc.test_evtHint("Surano Pirates", _([["There's rumours of a pirate that found something interesting in the Nebula and is trying to claim the system to themselves! I think it was the Surano system, but I am very bad at astrography."]]))},
   -- Mission Completion
   {npc.test_misnDone("Nebula Satellite"), _([["Heard some reckless scientists got someone to put a satellite inside the nebula for them. I thought everyone with half a brain knew to stay out of there, but oh well."]])},
   {npc.test_misnDone("Shadow Vigil"), _([["Did you hear? There was some big incident during a diplomatic meeting between the Empire and the Dvaered. Nobody knows what exactly happened, but both diplomats died. Now both sides are accusing the other of foul play. Could get ugly."]])},
   {npc.test_misnDone("Baron"), _([["Some thieves broke into a museum on Varia and stole a holopainting! Most of the thieves were caught, but the one who carried the holopainting offworld is still at large. No leads. Damn criminals…"]])},
   {npc.test_misnDone("Destroy the FLF base!"), _([["The Dvaered scored a major victory against the FLF recently. They went into Sigur and blew the hidden base there to bits! I bet that was a serious setback for the FLF."]])},
   {npc.test_misnDone("Taiomi 10"), _([["There have been an awful lot of patrols going missing near Bastion. Could this be work of the ghosts people talk about?"]])},
   -- Event Completion
   {npc.test_evtDone("Animal trouble"), _([["What? You had rodents sabotage your ship? Man, you're lucky to be alive. If it had hit the wrong power line…"]])},
   {npc.test_evtDone("Naev Needs You!"), _([["What do you mean, the world ended and then the creator of the universe came and fixed it? What kind of illegal substance are you on?"]])},
}

local msg_jump = {
   _([["Hi there, traveler. Is your system map up to date? Just in case you didn't know already, let me give you the location of the jump from here to {jmp}. I hope that helps."]]),
   _([["Quite a lot of people who come in here complain that they don't know how to get to {jmp}. I travel there often, so I know exactly where the jump point is. Here, let me show you."]]),
   _([["So you're still getting to know about this area, huh? Tell you what, I'll give you the coordinates of the jump to {jmp}. Check your map next time you take off!"]]),
   _([["True fact, there's a direct jump from here to {jmp}. Want to know where it is? It'll cost you! Ha ha, just kidding. Here you go, I've added it to your map."]]),
   _([["There's a system just one jump away by the name of {jmp}. I can tell you where the jump point is. There, I've updated your map. Don't mention it."]]),
}

--[=[
msg_lore["Independent"] = {
   _([["We're not part of any of the galactic superpowers. We can take care of ourselves!"]]),
   _([["Sometimes I worry that our lack of a standing military leaves us vulnerable to attack. I hope nobody will get any ideas about conquering us!"]]),
}

msg_lore["Trader"] = {
   _([["Just another link in the Great Chain, right?"]]),
   _([["You win some, you lose some, but if you don't try you're never going to win."]]),
   _([["If you don't watch the markets then you'll be hopping between planets in a jury-rigged ship in no time."]]),
   _([["Them blimming pirates, stopping honest folk from making an honest living - it's not like we're exploiting the needy!"]]),
}
--]=]

-- Get a lore message
local function getMessageLore ()
   return msg_lore[ rnd.rnd(1, #msg_lore) ]
end

-- Returns a jump point message and updates jump point known status accordingly. If all jumps are known by the player, defaults to a lore message.
local jumptargets = {}
local function getMessageJump ()
   -- The player already knows all jumps in this system or no messages
   if #jumptargets == 0 or #msg_jump==0 then
      return getMessageLore()
   end

   local retmsg =  msg_jump[rnd.rnd(1, #msg_jump)]
   local sel = rnd.rnd(1, #jumptargets)
   local tgt = jumptargets[sel]
   local myfunc = function( npcdata )
      if npcdata.talked then
         return
      end
      tgt:setKnown(true)
      tgt:dest():setKnown(true, false)

      -- Reduce jump message chance
      local jm_chance = var.peek("npc_jm_chance") or jm_chance_max
      var.push( "npc_jm_chance", math.max( jm_chance - 0.025, jm_chance_min ) )
      npcdata.talked = true
   end

   -- Remove target from list
   table.remove( jumptargets, sel )
   return fmt.f( retmsg, {jmp=tgt:dest()} ), myfunc
end

local function getMessage( lst )
   if #lst == 0 then
      return getMessageLore()
   end
   return lst[ rnd.rnd(1, #lst) ]
end

return function ()
   local cur, scur = spob.cur()
   local presence = scur:presences()["Independent"] or 0
   local fct = cur:faction()

   -- Need a generic faction
   if not fct or not fct:tags().generic then
      return nil
   end

   -- Need independent presence in the system
   if presence <= 0 then
      return nil
   end

   -- Don't appear on restricted assets
   if cur:tags().restricted then
      return nil
   end

   -- Create a list of conditional messages
   msg_combined = npc.combine_cond( msg_cond )

   -- Collect a table of jump points in the system the player does NOT know.
   jumptargets = {}
   for _, j in ipairs(system.cur():jumps(true)) do
      if not j:known() and not j:hidden() then
         table.insert(jumptargets, j)
      end
   end

   local function gen_npc()
      -- Append the faction to the civilian name, unless there is no faction.
      local name = _("Civilian")
      local desc = desc_list[ rnd.rnd(1,#desc_list) ]
      local image, prt = vni.generic()
      local msg, func
      local r = rnd.rnd()

      if r < (var.peek("npc_jm_chance") or jm_chance_max) then
         -- Jump point message.
         msg, func = getMessageJump()
      elseif r <= 0.55 then
         -- Lore message.
         msg = getMessageLore()
      elseif r <= 0.8 then
         -- Gameplay tip message.
         msg = getMessage( npc.msg_tip )
      else
         -- Mission hint message.
         msg = getMessage( msg_combined )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg, func=func }
   end

   return { create=gen_npc }
end
