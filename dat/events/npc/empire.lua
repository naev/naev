local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"

-- Chance of a jump point message showing up. As this gradually goes
-- down, it is replaced by lore messages. See spawnNPC function.
local jm_chance_min = 0
local jm_chance_max = 0.25
-- State. Nothing persists.
local msg_combined, seltargets

local desc_list = npc.desc_list

local msg_lore = {}
msg_lore["general"] = npc.msg_lore

msg_lore["Empire"] = {
   _([["Things are getting worse by the cycle. What happened to the Empire? We used to be the lords and masters over the whole galaxy!"]]),
   _([["Did you know that House Za'lek was originally a Great Project initiated by the Empire? Well, now you do! There was also a Project Proteron, but that didn't go so well."]]),
   _([["The Emperor lives on a giant supercruiser in Gamma Polaris. It's said to be the biggest ship in the galaxy! I totally want one."]]),
   _([["I'm still waiting for my pilot license application to get through. Oh well, it's only been half a cycle, I just have to be patient."]]),
   _([["Between you and me, the laws the Council passes can get really ridiculous! Most planets find creative ways of ignoring them..."]]),
   _([["Don't pay attention to the naysayers. The Empire is still strong. Have you ever seen a Peacemaker up close? I doubt any ship fielded by any other power could stand up to one."]]),
}

-- Returns a lore message for the given faction.
local function getLoreMessage( fac )
   -- Select the faction messages for this NPC's faction, if it exists.
   local facmsg = msg_lore[fac]
   if facmsg == nil or #facmsg == 0 then
      facmsg = msg_lore["general"]
      if facmsg == nil or #facmsg == 0 then
         evt.finish(false)
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
   local retmsg = npc.msg_jmp[rnd.rnd(1, #npc.msg_jmp)]
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

return function ()
   local cur, scur = spob.cur()
   local presence = scur:presences()["Empire"] or 0

   -- Need presence in the system
   if presence < 0 then
      return nil
   end

   -- Don't appear on restricted assets
   if cur:tags().restricted then
      -- TODO military personnel
      return nil
   end

   local function gen_npc()
      -- Append the faction to the civilian name, unless there is no faction.
      local fct = "Empire"
      local name = _("Empire Civilian")
      local desc = desc_list[ rnd.rnd(1,#desc_list) ]
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
         if rnd.rnd() < 0.5 then
            msg = getMissionLikeMessage(fct)
         else
            msg = getLoreMessage(fct)
         end
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg, func=func }
   end

   return { create=gen_npc }
end
