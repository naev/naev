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
            m.norun = true
            local pos = p:pos()
            local vel = p:vel()
            for i=1,3 do
               local e = pilot.add("Shark", "Mercenary", pos, _("Follower of Jeanne"), {naked=true})
               equipopt.generic( e, {beam=10}, "elite" )
               e:setLeader( p )
               e:setVel( vel )
            end
         end
      }, {
         spawn = function ()
            local p = pilot.add("Starbridge", "Mercenary", nil, _("Bloodhound"), {naked=true, ai="pers_patrol"})
            p:intrinsicSet( "ew_detect", 50 )
            equipopt.sirius( p )
            local m = p:memory()
            m.comm_greet = _([["Space seems much less vast when you can track like a bloodhound."]])
            m.taunt = _("And now for your death!")
            return p
         end
      }, {
         spawn = function ()
            local p = pilot.add("Sirius Dogma", "Mercenary", nil, _("White Shield"), {naked=true, ai="pers_patrol"})
            equipopt.sirius( p )
            local m = p:memory()
            m.comm_greet = _([["The White Shield brings justice for al!"]])
            m.taunt = _("Prepare to face justice!")
            m.bribe_no = _("There is nothing but justice!")
            m.whiteknight = true
            m.norun = true
            m.atk_kill = false
            return p
         end
      },
   }
end
