local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local presence = scur:presences()["Soromid"] or 0
   if presence <= 0 then
      return nil -- Need at least some presence
   end

   -- Medium ships here
   if presence > 20 then
      for k,v in ipairs{
         {
            spawn = function ()
               -- Hau = maori unisex name that means wind
               local p = pilot.add("Soromid Arx", "Soromid", nil, _("Elder Hau"), {naked=true, ai="pers"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.soromid( p, { bioship_stage=12,
                     bioship_skills={
                        "bite1","bite2","bite3","bite4","bite5",
                        "health1","health2","health3","health4","health5"} } )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["Do you feel the ebb of the universe? Only through harmony will we surpass our frail selves."]])
               m.taunt = _("You shall make a good sacrifice to my bioship!")
               m.bribe_no = _([["We do not deal with the tainted."]])
               return p
            end,
         }, {
            spawn = function ()
               -- Irala = maori unisex name that means god wrestler
               local p = pilot.add("Soromid Odium", "Soromid", nil, _("Witch Doctor Irala"), {naked=true, ai="pers"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.soromid( p, { bioship_stage=8,
                     bioship_skills={
                        "attack1", "attack2", "attack3",
                        "plasma1", "plasma2", "plasma3",
                        "health1","health2"} } )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["The sky omens indicate troubled times ahead."]])
               m.taunt = _("You disrupt the order of nature!")
               m.bribe_no = _([["We do not deal with the tainted."]])
               return p
            end,
         }, {
            spawn = function ()
               local p = pilot.add("Soromid Nyx", "Soromid", nil, _("Warrior Renata"), {naked=true, ai="pers_patrol"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.soromid( p, { bioship_stage=9,
                     bioship_skills={
                        "bite1", "bite2", "bite3", "bite4",
                        "plasma1", "plasma2", "plasma3",
                        "health1","health2"},
                     outfits_add={"Plasma Eruptor"},
                     prefer={
                        ["Plasma Eruptor"] = 100,
                     },
                  } )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["These are troubled times. Keep safe wanderer."]])
               m.taunt = _("Finally, a challenge!")
               m.bribe_no = _([["Do not patronize me with your grubby credits."]])
               return p
            end,
         }, {
            spawn = function ()
               local p = pilot.add("Soromid Odium", "Soromid", nil, _("Mosi the Spry"), {naked=true, ai="pers_patrol"})
               p:outfitAddIntrinsic("Escape Pod")
               equipopt.soromid( p, { bioship_stage=8,
                     bioship_skills={
                        "bite1", "bite2", "bite3",
                        "move1", "move2", "move3", "move4",
                        "stealth1",},
                     outfits_add={"Orion Lance"},
                     prefer={
                        ["Orion Lance"] = 100,
                     },
                  } )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["I've always wanted to try to eat some jellywhale."]])
               m.taunt = _("Your head shall make a good trophy.")
               m.bribe_no = _([["Credits will not get me a fluffier tail!"]])
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
