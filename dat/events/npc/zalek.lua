local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("You see a person casually solving some partial differential equations on a napkin."),
   _("A civilian who seems to be studying an ancient dialect."),
   _("This person is using a portable spectrometer to analyze their fizzy drink."),
   _("This civilian is staring blankly at the ceiling, while absentmindedly stirring their drink."),
   _("A civilian with three drinks in front of them who seems to be taking notes of the experience."),
   _("An individual reading a 500 page proof while drinking hard liquor."),
   _("A slightly tipsy individual who seems to be trying to do peer review."),
   _("A person idly writing a proof using non-Euclidean algebras."),
}
desc_list["agriculture"] = {
   _("The person seems to have a small beaker of algae in front of hem."),
   _("An individual that smells oddly of seaweed."),
}
desc_list["industrial"] = {
   _("The individual is analyzing some industrial schematics."),
}
desc_list["mining"] = {
   _("The person seems to have a set of rock samples in front of them. Some seem to have been licked."),
}
desc_list["tourism"] = {
   function () return fmt.f(_("The person is looking at a {spob} guidebook full of notes and bookmarks intensely."),{spob=spob.cur()}) end,
   _("A civilian with an expensive looking holo-recorder and a backpack."),
}
--desc_list["medical"]
--desc_list["trade"] = {}
--desc_list["old"] = {}
desc_list["immigration"] = {
   function () return fmt.f(_("A person looking at a {spob} homeowners manual."),{spob=spob.cur()}) end,
}
desc_list["prison"] = {
   _("A Za'lek citizen who is studying a sociology textbook on persuasive techniques."),
}
desc_list["station"] = {
   _("A civilian that seems to have never set foot on a planet."),
}
desc_list["government"] = {
   _("An individual who seems to be using Za'lek documents as a coaster."),
}

local msg_lore = {
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

local msg_cond = {
   { npc.test_misnHint("Za'lek Black Hole 1"), _([["Did you know there's a ton of tiny Za'lek research outposts throughout the galaxy? Since they have few or no staff, it is common for them to run into trouble."]]) },
   { npc.test_misnHint("Za'lek Black Hole 1"), _([["I heard that there have been some weird electromagnetic emissions recorded near the Anubis black hole. It is probably just poorly calibrated instruments."]]) },
   { npc.test_misnHint("Za'lek Particle Physics 1"), _([["It seems like Chairwoman Noona has run away to do science again. Nobody takes the Chairwoman job seriously anymore."]]) },
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
   local presence = scur:presences()["Za'lek"] or 0
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

   -- Create a list of conditional messages
   msg_combined = {}
   for k,msg in ipairs( msg_cond ) do
      if msg[1]() then
         table.insert( msg_combined, msg[2] )
      end
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
      local fct = "Za'lek"
      local name = _("Za'lek Civilian")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local prt  = portrait.get( fct )
      local image = portrait.getFullPath( prt )
      local msg
      local r = rnd.rnd()

      if r <= 0.45 then
         msg = getMessageLore()
      elseif r <= 0.7 then
         msg = getMessage( npc.msg_tip )
      else
         msg = getMessage( msg_combined )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc }
end
