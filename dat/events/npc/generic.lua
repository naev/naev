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
   "neutral/female1n.webp",
   "neutral/female2n.webp",
   "neutral/female2n_nogog.webp",
   "neutral/female3n.webp",
   "neutral/female4n.webp",
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
      if pir.factionIsPirate( fct ) then return nil end
      if fct == "FLF" then return nil end
      if inlist( {"Empire", "Za'lek", "Dvaered", "Soromid", "Sirius", "Frontier", "Proteron", "Thurion"}, fct ) then fct = nil end

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
