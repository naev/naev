--local fmt = require "format"
local vni = require "vnimage"
local npc = require "common.npc"

-- State. Nothing persists.
local msg_combined

local desc_list = {}
desc_list["generic"] = {
   _("The civilian is curling some weights while enjoying a drink."),
   _("You see a person who seems to be flexing their muscles while drinking."),
   _("The individual has a large assortment of empty glasses in front of them."),
   _("A civilian downing drinks one after another."),
   _("This person does their best to read the news terminal despite having a black eye."),
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
   _([[This worker wears a badge that reads "Miner of the Cycle".]]),
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
   _([["My great-great-great-grandfather fought in the Dvaered Revolts! We still have the holovids he made. I'm proud to be a Dvaered!"]]), -- First degree Dvaered apologists
   _([["You better not mess with House Dvaered. Our military is the largest and strongest in the galaxy. Nobody can stand up to us!"]]),
   _([["House Dvaered? House? The Empire is weak and useless, we don't need them anymore! I say we declare ourselves an independent faction today. What are they going to do, subjugate us? We all know how well that went last time! Ha!"]]),
   _([["I don't understand why other houses call us brutes. Nobody honours their fallen heroes like our Mace Rocket Ballets! The true brutes are those who don't respect the dead!"]]),
   _([["Did you know that most of the Frontier population actually wish they would become part of House Dvaered? That's the result of a survey done by 'Mace Rocket Radio' earlier this cycle."]]),
   _([["Dvaered society is the fairest possible because the strongest get the best places, and everyone can become strong if they work hard enough for it."]]),
   _([["Most major powers are led by a decadent oligarchy that scorn violence. But violence is a part of our lives whether we want it or not, and rejecting it would only make us unprepared when it inevitably knocks at our door."]]),
   _([["Do you know that on average, basic workers are paid twice more on Dvaered planets than on core Imperial ones? This is because our leaders know they would be nothing without workers, and while they call for loyalty from their subordinates, they are required to be grateful in return."]]),
   _([["I'm thinking about joining the military. Every time I see or hear news about those rotten FLF bastards, it makes my blood boil! They should all be pounded into space dust!"]]), -- Anti-FLF
   _([["FLF terrorists? I'm not too worried about them. You'll see, High Command will have smoked them out of their den soon enough, and then the Frontier will be ours."]]),
   _([["I fail to understand what the FLF exactly try to accomplish: they attack our patrols and supply convoys while all that we do is to patrol our own space, and some zones of the Frontier space as agreed in the 'Dvaered-Frontier common anti-piracy treaty'."]]),
   _([["Do you know the Freaking Losers Front? Yeah, that's what the letters FLF stand for!"]]),
   _([["I've got lots of civilian commendations! It's important to have commendations if you're a Dvaered."]]), -- Neutral-ish lore
   _([["Did you know that House Dvaered was named after a hero of the revolt? At least that's what my grandparents told me."]]),
   _([["Warlords like mace rockets so much that they decided to make really big ones for their capships. They're called Super-Fast Colliders or SFC for short!"]]),
   _([["Did you watch the bare-hand fight Ironhead VS Dodgeman? It ended at round 11, but Ironhead had no hope anymore after round 5, when he got both his arms torn out. Dodgeman's new cyber mandibles are really incredible!"]]),
   _([["Our Warlord is currently fighting for control over another planet. We all support him unconditionally, of course! Of course…"]]), -- Negative stuff
   _([["To be a good Dvaered, you have to be right, loyal and strong. I guess. At least that's what they say all day on holovision."]]),
   _([["Do you know the 'Dvaered-Frontier common anti-piracy treaty'? Dvaered High Command sent a huge fleet to the Frontier Council and forced them to sign it. This way, we can pretend our patrols in Frontier space are legal. We even got the right to build Fort Raglan that way."]]),
   _([["They say we are all equal. Only righteousness, loyalty and strength allow us to climb to the top… And probably also being the child of a general could not harm."]]),
   _([["I'm sorry to inform you that Dvaered society is decadent: many generals of the High Command have been helped in their early military career by privileges due to their parents being also generals."]]),
   _([["I've been again caught in a brawl this morning. I'm so tired of being a Dvaered."]]),
   _([["There are different ways to be a good citizen: you can either be a soldier, or an active member of the Dvaered Unified Labour Union, a martial arts champion, or a prolific procreator. And I am none of these. I guess I am a poor citizen then."]]),
   _([["The best way to solve our economic crisis is destruction! More things destroyed means more things have to be built, right? I am a genius."]]), -- Others
   _([["Do you know Totoran bare-hand fighter Elbowator? He got 75 percent of his brain amputated to lower the risk of receiving a KO. I totally want to become like him!"]]),
   _([["Have you seen our Warlord's Goddard battlecruisers? One day, I will buy one. I have computed that I will have enough money for that in 1500 cycles given my current salary. I just need to find a way to stop eating."]]),
   _([["My niece graduated from Vilati Vilata Academy of Fine Arts last cycle. It is the place where they draw the awesome posters you see everywhere. I am so proud of her!"]]),
}

local msg_tip = {
   _([["Kinetic weapons cause impact damage which is more effective against armour than shields. It also has incredible knocking back power!"]]),
   _([["House Dvaered is the only Great House that doesn't use fighter bays. It's much more honourable to do the job yourself!"]]),
   _([["Mace rockets are a must-have for fighter pilots. They can spectacularly boost your damage output. But you must also have cannons because mace launchers are extremely slow to reload once empty."]]),
}

local msg_cond = {
   {npc.test_misnDone("Destroy the FLF base!"), _([["Did you hear the news? The FLF main base in Sigur was blown up to bits! A wonderful day for House Dvaered!"]])},
   {npc.test_misnDone("Dvaered Shopping"), _([["Oh! You know Lord Fatgun, the guy who nuked Tora? They said on the news he has just been killed!"]])},
   { function () return (player.chapter()=="0") end, _([["If you like breaking down asteroids, they are looking for rare minerals at Dvaer. I would do it myself, but it seems to require too much precision for my taste."]]) },
   { function () return (player.chapter()~="0") end, _([["The megastructure at Dvaer is cool, but I would have preferred a gigantic orbital railgun over a Hypergate."]]) },
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

   local w = 0
   if cur:faction() == faction.get("Dvaered") then
      w = 1
   elseif presence>0 then
      w = 0.2
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
      local name
      if rnd.rnd() < 0.5 then
         name = _("Dvaered Civilian")
      else
         name = _("Dvaered Worker")
      end
      local desc = descriptions[ rnd.rnd(1,#descriptions) ]
      local image, prt = vni.dvaered()
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
