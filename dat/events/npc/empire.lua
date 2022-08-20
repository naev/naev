local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("A civilian is filling out paperwork."),
   _("An individual looks like they are dealing with Empire paperwork."),
   _("You see a civilian staring blankly at a table."),
   _("A civilian is swishing their drink."),
   _("An individual is sitting idly at the bar."),
}
desc_list["agriculture"] = {
   _("A civilian that smells of algae is drinking alone."),
   _("There is a civilian wearing heavy-duty boots caked in mud."),
}
desc_list["industrial"] = {
   _("A civilian with heavy duty boots."),
   _("There is a person that seems of industrial compounds."),
   _("This person seems to have a factory badge on their clothes."),
}
desc_list["mining"] = {
   _("An individual that seems covered in a fine layer of sparkling dust."),
   _("You see a person that has taken off protective mining gear and left it on the table."),
}
desc_list["tourism"] = {
   _("This individual seems to be here on tourism."),
   _("A civilian is perusing a travel guide book."),
}
--desc_list["medical"]
desc_list["trade"] = {
   _("This civilian seems to be filling out tax forms."),
   _("The person has a bag labelled commercial samples on the floor."),
}
desc_list["old"] = {
   function () return fmt.f(_("The person has an air of never having left {spob}."),{spob=spob.cur()}) end,
}
desc_list["immigration"] = {
   _("The individual has a lot of luggage next to them."),
}
desc_list["prison"] = {
   _("The person seems to have a stungun holstered."),
}
desc_list["station"] = {
   _("The person seems very used to the low gravity of the station."),
}
desc_list["government"] = {
   _("There is a civilian with an Empire issued document bag."),
   _("A civilian with an Empire issued bureaucrat ID tag."),
   _("This person seems to be distracted with a graphing holocalculator."),
   _("The individual seems to be preparing holoslides for a presentation."),
}

local msg_lore = {}
msg_lore["general"] = npc.msg_lore

msg_lore["Empire"] = {
   _([["Things are getting worse by the cycle. What happened to the Empire? We used to be the lords and masters over the whole galaxy!"]]),
   _([["Did you know that House Za'lek was originally a Great Project initiated by the Empire? Well, now you do! There was also a Project Proteron, but that didn't go so well."]]),
   _([["The Emperor lives on a giant supercruiser in Gamma Polaris. It's said to be the biggest ship in the galaxy! I totally want one."]]),
   _([["I'm still waiting for my pilot license application to get through. Oh well, it's only been half a cycle, I just have to be patient."]]),
   _([["Between you and me, the laws the Council passes can get really ridiculous! Most planets find creative ways of ignoring them…"]]),
   _([["Don't pay attention to the naysayers. The Empire is still strong. Have you ever seen a Peacemaker up close? I doubt any ship fielded by any other power could stand up to one."]]),
}

local msg_mhint = {
   {"Collective Espionage 1", _([["The Empire is trying to really do something about the Collective, I hear. Who knows, maybe you can even help them out if you make it to Omega Station."]])},
}

local msg_ehint = {
}

local msg_mdone = {
   {"Operation Cold Metal", _([["Hey, remember the Collective? They got wiped out! I feel so much better now that there aren't a bunch of robot ships out there to get me anymore."]])},
}

local msg_edone = {
}

-- Returns a lore message for the given faction.
local function getLoreMessage( fac )
   -- Select the faction messages for this NPC's faction, if it exists.
   local facmsg = msg_lore[fac]
   if facmsg == nil or #facmsg == 0 then
      facmsg = msg_lore["general"]
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

-- Returns a tip message.
local function getTipMessage( fct )
   -- All tip messages are valid always.
   if #npc.msg_tip == 0 then
      return getLoreMessage(fct)
   end
   local sel = rnd.rnd(1, #npc.msg_tip)
   local pick = npc.msg_tip[sel]
   table.remove(npc.msg_tip, sel)
   return pick
end

-- Returns a mission hint message, a mission after-care message, OR a lore message if no missionlikes are left.
local function getMissionLikeMessage( fct )
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
      return getLoreMessage(fct)
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
   local tags = cur:tags()

   -- Need presence in the system
   if presence < 0 then
      return nil
   end

   -- Don't appear on restricted assets
   if tags.restricted then
      -- TODO military personnel
      return nil
   end

   -- Add tag-appropriate descriptions
   local descriptions = tcopy( desc_list["generic"] )
   for t,v in pairs(tags) do
      local dl = desc_list[t]
      if dl then
         for k,d in ipairs(dl) do
            table.insert( descriptions, d )
         end
      end
   end

   local function gen_npc()
      -- Append the faction to the civilian name, unless there is no faction.
      local fct = "Empire"
      local name = _("Empire Civilian")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local prt  = portrait.get( fct )
      local image = portrait.getFullPath( prt )
      local msg
      local func = nil
      local r = rnd.rnd()

      if r <= 0.45 then
         -- Lore message.
         msg = getLoreMessage(fct)
      elseif r <= 0.7 then
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
