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

local msg_lore = {
   _([["Things are getting worse by the cycle. What happened to the Empire? We used to be the lords and masters over the whole galaxy!"]]),
   _([["Did you know that House Za'lek was originally a Great Project initiated by the Empire? Well, now you do! There was also a Project Proteron, but that didn't go so well."]]),
   _([["The Emperor lives on a giant supercruiser in Gamma Polaris. It's said to be the biggest ship in the galaxy! I totally want one."]]),
   _([["I'm still waiting for my pilot license application to get through. Oh well, it's only been half a cycle, I just have to be patient."]]),
   _([["Between you and me, the laws the Council passes can get really ridiculous! Most planets find creative ways of ignoring them…"]]),
   _([["Don't pay attention to the naysayers. The Empire is still strong. Have you ever seen a Peacemaker up close? I doubt any ship fielded by any other power could stand up to one."]]),
   _([["I've been studying to become an Empire Combat Beaurocrat, but I keep on failing the paperwork exam. It's more fun to blow ships up than fill forms!"]]),
   _([["Have you ever seen an Executor up close? I heard they use special technology to create a shield aura around Imperial ships!"]]),
   _([["I really want to meet the Emperor's Iguana. It's supposed to be 3 times bigger than a human and breathe fire!"]]),
   _([["I'm fed up with all the paper work. I want to move to someplace more simple. Maybe a Dvaered planet would be good."]]),
   _([["I don't understand how the Soromid can live outside of the Empire. They should subjugate as a Great House or be eliminated!"]]),
   _([["House Za'lek used to be closed to the outside world. I wonder what made them change their mind?"]]),
   _([["The Dvaered are very boisterous and violent, however, their loyalty to the Empire is second to none!"]]),
   _([["Why can't the Empire do more against pirates? We have the best armada in the galaxy!"]]),
   _([["They tell us that if we work hard enough, we too can become nobles in the Imperial Court, but nobody I know has made it in!"]]),
   _([["In the Empire, the teach us to do paperwork as soon as we learn to read! Since it's so important, I think they should teach it even sooner!"]]),
}

local msg_tip = {
   _([["Lasers are very fast and have long range. Other Houses wish they had such good weapon technology!"]]),
   _([["Doing shipping for the Empire pays much better than other cargo missions. It is also a good way to curry favour with the Empire!"]]),
   _([["If you get in a pickle wih a pirate and no Empire ship is around to save you, you can always bribe them to go away. No paperwork needed!"]]),
}

local msg_cond = {
   { npc.test_misnHint("Empire Recruitment"), _([["Have you thought about doing shipping for the Empire? It's great work! You just need to find a recruiter to teach you the ropes."]])},
   { npc.test_misnHint("Empire Shipping 2"), _([["I hear you can get a Heavy Weapons License if you help out the Empire doing special shipping missions."]])},
   { npc.test_misnHint("Collective Espionage 1"), _([["I've heard that there seems to be lots of combat near Fortitude. You might even be able to help if you make it to Omega Enclave."]])},
   { npc.test_misnHint("Operation Cold Metal"), _([["I was getting rid of some documents the other day and found some old document about a project to create a fully autonomous self-replicating armada of robot ships. I wonder what happened with that?"]])},
   { function () return (player.chapter()=="0") end, _([["I hear the Empire is looking for rare minerals in Gamma Polaris. What could they be building?"]])},
   { function () return (player.chapter()~="0") end, _([["Did you see the incredible hypergate at Gamma Polaris? The Empire is still unrivaled by the Great Houses!"]])},
   { npc.test_misnDone("Operation Cold Metal"), _([["Hey, remember the Collective? Never really heard about them until I saw the news that they got wiped out! I feel so much better now that there aren't a bunch of robot ships out there to get me anymore."]])},
}

-- Returns a lore message for the given faction.
local function getMessageLore ()
   return msg_lore[ rnd.rnd(1,#msg_lore) ]
end

local function getMessage( lst )
   if #lst == 0 then
      return getMessageLore()
   end
   return lst[ rnd.rnd(1, #lst) ]
end

return function ()
   local cur, scur = spob.cur()
   local presence = scur:presences()["Empire"] or 0
   local tags = cur:tags()

   local w = 0
   if cur:faction() == faction.get("Empire") then
      w = 1
   elseif presence>0 then
      w = 0.2 -- Fewer NPC
   end

   -- Need positive weight
   if w <= 0 then
      return nil
   end

   -- Don't appear on restricted assets
   if tags.restricted then
      -- TODO military personnel
      return nil
   end

   -- Create a list of conditional messages
   msg_combined = npc.combine_cond( msg_cond )

   -- Add tag-appropriate descriptions
   local descriptions = npc.combine_desc( desc_list, tags )

   local function gen_npc()
      local name = _("Empire Citizen")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local prt  = portrait.get( "Empire" )
      local image = portrait.getFullPath( prt )
      local msg
      local r = rnd.rnd()
      if r <= 0.45 then
         msg = getMessageLore()
      elseif r <= 0.7 then
         if rnd.rnd() < 0.5 then
            msg = getMessage( msg_tip )
         else
            msg = getMessage( npc.msg_tip )
         end
      else
         msg = getMessage( msg_combined )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc, w=w }
end
