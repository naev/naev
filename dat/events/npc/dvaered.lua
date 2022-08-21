--local fmt = require "format"
local portrait = require "portrait"
local npc = require "common.npc"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("The civilian is curling some weights while enjoyinga drink."),
   _("You see a person who seems to be flexing their muscles while drinking."),
   _("The individual has a large assortment of empty glasses infront of them."),
   _("A civiilian downning drinks one after another."),
}
desc_list["agriculture"] = {
   _("The person has leaves stuck all over them."),
}
desc_list["industrial"] = {
   _("The individual is covered in smut."),
   _("A civilian with a thin layer of grime on them."),
}
desc_list["mining"] = {
   _("A worker with lots of small pebbles falling off of them."),
}
--desc_list["tourism"] = {}
--desc_list["medical"] = {}
--desc_list["trade"] = {}
--desc_list["old"] = {}
--desc_list["immigration"] = {}
--desc_list["prison"] = {}
--desc_list["station"] = {}
--desc_list["government"] = {}

local msg_lore = {
   _([["Our Warlord is currently fighting for control over another planet. We all support him unconditionally, of course! Of course…"]]),
   _([["My great-great-great-grandfather fought in the Dvaered Revolts! We still have the holovids he made. I'm proud to be a Dvaered!"]]),
   _([["I've got lots of civilian commendations! It's important to have commendations if you're a Dvaered."]]),
   _([["You better not mess with House Dvaered. Our military is the largest and strongest in the galaxy. Nobody can stand up to us!"]]),
   _([["House Dvaered? House? The Empire is weak and useless, we don't need them anymore! I say we declare ourselves an independent faction today. What are they going to do, subjugate us? We all know how well that went last time! Ha!"]]),
   _([["I'm thinking about joining the military. Every time I see or hear news about those rotten FLF bastards, it makes my blood boil! They should all be pounded into space dust!"]]),
   _([["FLF terrorists? I'm not too worried about them. You'll see, High Command will have smoked them out of their den soon enough, and then the Frontier will be ours."]]),
   _([["Did you know that House Dvaered was named after a hero of the revolt? At least that's what my grandparents told me."]]),
   _([["I don't understand why other houses call us brutes. Nobody honours their fallen heroes like our Mace Rocket Ballets! The true brutes are those who don't respect the dead!"]]),
   _([["The best way to solve our economic crisis is destruction! More things destroyed means more things have to be built, right? I am a genius."]]),
   _([["Warlords like mace rockets so much that they decided to make really big ones for their capships. They're called Super-Fast Colliders or SFC for short!"]]),
   -- Tips should be split out eventually
   _([["Kinetic weapons cause impact damage which is more effective against armour than shields. It also has incredible knocking back power!"]]),
   _([["House Dvaered is the only Great House that doesn't use fighter bays. It's much more honourable to do the job yourself!"]]),
}

local msg_cond = {
   {npc.test_misnDone("Destroy the FLF base!"), _([["Did you hear the news? The FLF main base in Sigur was blown up to bits! A wonderful day for House Dvaered!"]])},
   { function () return (player.chapter()=="0") end, _([["If you like breaking down asteroids, they are looking for rare minerals at Dvaer. I would do it myself, but it seems to require too much precision for my taste."]]) },
   { function () return (player.chapter()~="0") end, _([["The megastructure at Dvaer is cool, but I would have prefered a gigantic orbital railgun over a Hypergate."]]) },
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
   local presence = scur:presences()["Dvaered"] or 0
   local tags = cur:tags()

   -- Need presence in the system
   if presence <= 0 then
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
      -- Append the faction to the civilian name, unless there is no faction.
      local name = _("Dvaered Civilian")
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local prt  = portrait.get( "Dvaered" )
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
