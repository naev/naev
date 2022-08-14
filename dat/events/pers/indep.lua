local equipopt = require "equipopt"

return function ()
   local scur = system.cur()
   local pres = scur:presences()["Independent"] or 0
   if pres <= 0 then
      return nil -- Need at least some presence
   end

   return {
      {
         spawn = function ()
            local p = pilot.add("Mule", "Trader", nil, _("Trader Drake"), {naked=true, ai="pers"})
            local c = equipopt.cores.get( p, { class="Destroyer", all="elite" } )
            equipopt.zalek( p, {cores=c, fighterbay=20} )
            p:cargoAdd( "Nebula Crystals", p:cargoFree() )
            local m = p:memory()
            --m.ad = _("")
            m.comm_greet = _([["Business with the Za'lek is booming!"]])
            m.taunt = _("Say hello to my little friends!")
            return p
         end
      }, {
         spawn = function ()
            local p = pilot.add("Pacifier", "Mercenary", nil, _("Jeanne d'Arc"), {naked=true, ai="pers"})
            p:intrinsicSet( "shield_mod", 25 )
            p:intrinsicSet( "shield_regen_mod", 25 )
            p:intrinsicSet( "armour_mod", 25 )
            equipopt.generic( p, {beam=10}, "elite" )
            local m = p:memory()
            m.comm_greet = _([["Children say that people are hanged sometimes for speaking the truth."]])
            m.taunt = _("I am not afraid… I was born to do this.")
            m.bribe_no = _("You must be reprimanded for your sins!")
            m.formation = "cross"
            local pos = p:pos()
            local vel = p:vel()
            for i=1,3 do
               local e = pilot.add("Shark", "Mercenary", pos, _("Follower of Jeanne"), {naked=true})
               equipopt.generic( e, {beam=10}, "elite" )
               e:setLeader( p )
               e:setVel( vel )
            end
         end
      }
   }
end
