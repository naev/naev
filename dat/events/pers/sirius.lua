local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local presence = scur:presences()["Sirius"] or 0
   if presence <= 0 then
      return nil -- Need at least some presence
   end

   -- Medium ships here
   if presence > 100 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Sirius Dogma", "Sirius", nil, _("Serra Scion"), {naked=true, ai="pers"})
               equipopt.sirius( p, { flow_ability=outfit.get("Avatar of Sirichana") } )
               local m = p:memory()
               m.comm_greet = _([["The Serra Echelon is the pride of House Sirius. As a Scion, I bear the weight of responsibility truthfully."]])
               m.taunt = _("You dare test my faith?! The Serra Scion shall not waiver!")
               m.bribe_no = _([["You think you can buy my faith with mere credits?"]])
               return p
            end,
         },
         } do
      end
   end

   return pers
end
