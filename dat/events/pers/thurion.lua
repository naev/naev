--local equipopt = require "equipopt"
local strmess = require "strmess"

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
               m.capturable = true
               -- TODO have these actually mean something
               m.greet = strmess.tobinary( "You should not be seeing me.", true )
               m.taunt = strmess.tobinary( "Bye.", true )
               m.bribe_no = strmess.tobinary( "Why?", true )
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
