local equipopt = require "equipopt"

return function ()
   local scur = system.cur()
   local pres = scur:presences()["Independent"] or 0
   if pres <= 0 then
      return nil -- Need at least some presence
   end

   local pers = {
      {
         spawn = function ()
            local p = pilot.add("Goddard Merchantman", "Trader", nil, _("Trader Drake"), {naked=true, ai="pers"})
            p:outfitAddIntrinsic("Escape Pod")
            local c = equipopt.cores.get( p, { all="elite" } )
            equipopt.zalek( p, {cores=c, fighterbay=20} )
            p:cargoAdd( "Nebula Crystals", p:cargoFree() )
            local m = p:memory()
            m.capturable = true
            --m.ad = _("")
            m.comm_greet = _([["Business with the Za'lek is booming!"]])
            m.taunt = _("Say hello to my little friends!")
            return p
         end
      },
   }

   return pers
end
