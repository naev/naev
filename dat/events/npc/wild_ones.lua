--local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"
local pir = require "common.pirate"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("An unruly Wild Ones clansperson."),
   _("You see a rough looking Wild Ones clansperson."),
   _("The Wild Ones clansperson looks like they've seen some real shit."),
   _("A scarred  Wild Ones clansperson, leaning back in their chair with an air of nonchalant confidence."),
   _("A weathered Wild Ones clansperson with a permanent sneer etched on their face."),
   _("A gruff Wild Ones clansperson quietly nursing their drink."),
   _("A wiry Wild Ones clansperson, fingers tapping restlessly on the bar counter"),
   _("A burly Wild Ones clansperson, arms crossed and scowling"),
}

local msg_lore = {
   _([["Them Black Lotus think they be better'n us cuz they knows them 'business'. The only 'business' you need be the trigger of your cannons!"]]),
   _([["We may seem like an unruly gang, but we get along well. Arr!"]]),
   _([["If a crime is only punishable with a fee, then it is only a crime for the poor."]]),
   _([["We are naught but the symptom of a broken society. I mean, Arrr!"]]),
   _([["There is not a merrier bunch of ragtags than the Wild Ones! Cast out by society we now find our purpose! Pillage!"]]),
   _([["Many of us are exiles from Soromid tribes or refugees from the Empire. The Wild Ones are the only ones who welcome us with open arms!"]]),
   _([["Have you seen the Pink Demon? They're so cool! I want to meet them in person."]]),
   _([["We get along well with the Raven Clan. They're quite reliable for pirates! Can't say the same about us. Harr!"]]),
   _([["I wish the Dreamer Clan were closer. They've got some real nice shit if you know what I mean."]]),
   _([["My sibling is in the Black Lotus clan. When we meet up at a pirate assembly, all they talk about is money until I can get them nice and drunk."]]),
   _([["Sometimes it's almost like pillaging is too easy. Like, you know, the Empire is letting us instead of being an incompetent mess of a bureaucracy."]]),
   _([["I'm looking forward to the next pirate assembly. Time to party!"]]),
   _([["It's not easy to get into raiding convoys, but in this eat-or-be-eaten world, you gotta do what you gotta do to survive. Ya'know?"]]),
   _([["Hey, I have a confession to make. I once got my hands on a Dreame Clan pineapple pizza. They're quite delicious. Don't tell anyone I told you!"]]),
}

local msg_tip = {
   _([["You have to watch out when raiding Soromid convoys, sometimes their ships bite hard!"]]),
   _([["Stealth and fake transponders are the best way to move around unnoticed. Pirate Admonishers are great at the job!"]]),
   _([["You've got to learn your enemies when raiding. Empire lasers have long range, while Soromid plasma is short range, but can corrode your ship in a few seconds!"]]),
}

local msg_cond = {
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
   local tags = cur:tags()
   local presence = scur:presences()["Wild Ones"] or 0

   if not pir.factionIsPirate( cur:faction () ) then
      return nil
   end

   local w = 0
   if cur:faction() == faction.get("Wild Ones") then
      w = 1
   elseif presence>0 then
      w = 0.2 -- Fewer NPC
   end

   -- Need presence
   if w <= 0 then
      return nil
   end

   -- Create a list of conditional messages
   msg_combined = npc.combine_cond( msg_cond )

   -- Add tag-appropriate descriptions
   local descriptions = npc.combine_desc( desc_list, tags )

   local function gen_npc()
      local name = _("Wild Ones Clansperson")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local prt  = portrait.get( "Pirate" )
      local image = portrait.getFullPath( prt )
      local msg
      local r = rnd.rnd()
      if r <= 0.45 then
         msg = getMessageLore()
      elseif r <= 0.7 then
         msg = getMessage( msg_tip )
      else
         msg = getMessage( msg_combined )
      end
      return { name=name, desc=desc, portrait=prt, image=image, msg=msg }
   end

   return { create=gen_npc, w=w }
end
