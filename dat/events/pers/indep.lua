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
            local p = pilot.add("Pacifier", "Independent", nil, _("Jeanne d'Arc"), {naked=true, ai="pers"})
            p:intrinsicSet( "shield_mod", 25 )
            p:intrinsicSet( "shield_regen_mod", 25 )
            p:intrinsicSet( "armour_mod", 25 )
            equipopt.generic( p, {beam=10}, "elite" )
            local m = p:memory()
            m.comm_greet = _([["Children say that people are hanged sometimes for speaking the truth."]])
            m.taunt = _("I am not afraid… I was born to do this.")
            m.bribe_no = _([["You must be reprimanded for your sins!"]])
            m.formation = "cross"
            m.norun = true
            local pos = p:pos()
            local vel = p:vel()
            for i=1,4 do
               local e = pilot.add("Shark", "Independent", pos, _("Follower of Jeanne"), {naked=true})
               equipopt.generic( e, {beam=10}, "elite" )
               e:setLeader( p )
               e:setVel( vel )
            end
         end
      }, {
         spawn = function ()
            local p = pilot.add("Starbridge", "Independent", nil, _("Bloodhound"), {naked=true, ai="pers_patrol"})
            p:intrinsicSet( "ew_detect", 50 )
            equipopt.sirius( p )
            local m = p:memory()
            m.comm_greet = _([["Space seems much less vast when you can track like a bloodhound."]])
            m.taunt = _("And now for your death!")
            return p
         end
      }, {
         spawn = function ()
            local p = pilot.add("Sirius Dogma", "Independent", nil, _("White Shield"), {naked=true, ai="pers_patrol"})
            equipopt.sirius( p )
            local m = p:memory()
            m.comm_greet = _([["The White Shield brings justice for all!"]])
            m.taunt = _("Prepare to face justice!")
            m.bribe_no = _([["There is nothing but justice!"]])
            m.whiteknight = true
            m.norun = true
            m.atk_kill = false
            return p
         end
      }, {
         spawn = function ()
            local p = pilot.add("Goddard", "Independent", nil, _("Iron Curtain"), {naked=true, ai="pers"})
            p:intrinsicSet( "armour_mod", 50 )
            equipopt.dvaered( p )
            local m = p:memory()
            m.comm_greet = _([["…"]])
            m.taunt = _("…")
            m.bribe_no = _("…")
            m.norun = true
            return p
         end
      }, {
         spawn = function ()
            local p = pilot.add("Za'lek Sting", "Independent", nil, _("Bee"), {naked=true, ai="pers"})
            p:intrinsicSet( "shield_mod", 50 )
            p:intrinsicSet( "fbay_capacity", 50 )
            p:intrinsicSet( "fbay_reload", 100 )
            equipopt.zalek( p, {
               fighterbay = 10,
               bolt = 0.1,
            } )
            local m = p:memory()
            m.comm_greet = _([["Humans like to use fancy technology to try to disguise the fact that they are basically poop-flinging monkeys."]])
            m.taunt = _("Why are you doing this to me!?")
            return p
         end
      },
   }

   if scur == system.get("Zied") then
      table.insert( pers, {
         spawn = function ()
            local p = pilot.add("Starbridge", "Independent", nil, _("Cap'n Lector"), {naked=true, ai="pers"})
            p:intrinsicSet( "shield_mod", 50 )
            p:intrinsicSet( "shield_regen_mod", 50 )
            p:intrinsicSet( "armour_mod", 50 )
            p:intrinsicSet( "launch_damage", 50 )
            p:intrinsicSet( "fwd_damage", 50 )
            p:intrinsicSet( "tur_damage", 50 )
            equipopt.pirate( p, {
               outfits_add={"Emergency Stasis Inducer"},
               prefer={["Emergency Stasis Inducer"] = 100}} )
            local m = p:memory()
            m.ad = _("Don't forget to pay your shareware registration fee!")
            m.taunt = _("Prepare to pay your shareware registration fee!")
            m.bribe_no = _([["You can't pay your shareware registration fee with credits, only blood!"]])
            m.norun = true
         end,
         w = 100, -- Almost ensured in Zied
      } )
   end

   return pers
end
