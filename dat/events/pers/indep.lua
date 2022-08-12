local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local pres = scur:presences()["Independent"] or 0
   if pres <= 0 then
      return nil -- Need at least some presence
   end

   -- Larger ships can be there
   if pres > 0 then
      table.insert( pers, {
         spawn = function ()
            local p = pilot.add("Mule", "Trader", nil, _("Trader Drake"), {naked=true, ai="unique"})
            local c = equipopt.cores.get( p, { class="Destroyer", all="elite" } )
            equipopt.zalek( p, {cores=c, fighterbay=20} )
            p:cargoAdd( "Nebula Crystals", p:cargoFree() )
            local m = p:memory()
            --m.ad = _("")
            m.comm_greet = _([["Business with the Za'lek is booming!"]])
            m.taunt = _("Say hello to my little friends!")
            return p
         end
      } )
   end

   return pers
end
