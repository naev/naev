--local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local presence = scur:presences()["Thurion"] or 0
   if presence <= 0 then
      return nil -- Need at least some presence
   end

   if presence > 0 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Thurion Perspicacity Beta", "Thurion", nil, _("Eye of the Hive"), {ai="pers_runaway"})
               local m = p:memory()
               -- TODO have these actually mean something
               m.greet = "00010110101011011111010100101010010010100101000101100111110100101010001001010101000101010101001110110010"
               m.taunt = "100101011101101100011010100101101010"
               m.bribe_no = "100101011101101100011010100101101010"
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
