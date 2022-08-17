local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local presence = scur:presences()["Soromid"] or 0
   if presence <= 0 then
      return nil -- Need at least some presence
   end

   -- Medium ships here
   if presence > 100 then
      for k,v in ipairs{
         {
            spawn = function ()
               -- Hau = maori unisex name that means wind
               local p = pilot.add("Soromid Arx", "Soromid", nil, _("Elder Hau"), {naked=true, ai="pers"})
               equipopt.soromid( p, { bioship_stage=12,
                     bioship_skills={
                        "bite1","bite2","bite3","bite4","bite5",
                        "health1","health2","health3","health4","health5"} } )
               local m = p:memory()
               m.comm_greet = _([["Do you feel the ebb of the universe? Only through harmony will we surpass our frail selves."]])
               m.taunt = _("You shall make a good sacrifice to my bioship!")
               m.bribe_no = _("We do not deal with the tainted.")
               return p
            end,
         },
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
