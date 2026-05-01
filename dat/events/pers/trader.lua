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
      }, {
         -- S.T.S. stands for Space Trader Society
         spawn = function ()
            local p = pilot.add("Starbridge Sigma", "Trader", nil, _("S.T.S. Sonne"), {naked=true, ai="pers"})
            p:outfitAddIntrinsic("Escape Pod")
            local c = equipopt.cores.get( p, { all="elite" } )
            equipopt.empire( p, {cores=c, fighterbay=20} )
            local m = p:memory()
            m.capturable = true
            m.comm_greet = _([["Ya win some, ya lose some."]])
            m.taunt = _("Violence? How droll.")
            return p
         end
      }, {
         spawn = function ()
            local p = pilot.add("Plowshare", "Trader", nil, _("S.T.S. Olivia"), {naked=true, ai="pers"})
            p:outfitAddIntrinsic("Escape Pod")
            local c = equipopt.cores.get( p, { all="elite" } )
            equipopt.generic( p, {cores=c, fighterbay=20} )
            p:cargoAdd( "Vixilium", p:cargoFree() )
            local m = p:memory()
            m.capturable = true
            m.comm_greet = _([["Got business to make."]])
            m.taunt = _("Wait? That's illegal!")
            m.formation = "buffer"
            local pos = p:pos()
            local vel = p:vel()
            for i=1,4 do
               local e = pilot.add("Admonisher", "Trader", pos, nil, {naked=true})
               equipopt.empire( e )
               local em = e:memory()
               em.capturable = true
               e:setLeader( p )
               e:setVel( vel )
            end
            return p
         end
      }, {
         spawn = function ()
            local p = pilot.add("Zebra", "Trader", nil, _("S.T.S. Albatross"), {naked=true, ai="pers_runaway"})
            p:outfitAddIntrinsic("Escape Pod")
            local c = equipopt.cores.get( p, { all="elite" } )
            equipopt.empire( p, {cores=c, fighterbay=20, pointdefence=10} )
            p:cargoAdd( "Kermite", p:cargoFree() )
            local m = p:memory()
            m.capturable = true
            m.comm_greet = _([["Gotta be in it for the long haul, know what I mean?"]])
            m.taunt = _("C'mon! Not again!")
            return p
         end
      },
      -- Some Atra Vigilis
      {
         spawn = function ()
            local p = pilot.add("Vigilance", "Trader", nil, _("Crimson Star"), {naked=true, ai="pers_patrol"})
            p:outfitAddIntrinsic("Escape Pod")
            equipopt.soromid( p, {
               outfits_add={"Plasma Eruptor"},
               max_same_weap = 4,
               prefer={
                  ["Plasma Eruptor"] = 100,
               },
            } )
            local m = p:memory()
            m.capturable = true
            m.comm_greet = _([["Know where I can fry some pirates?"]])
            m.taunt = _("Get torched!")
            return p
         end
      }, {
         spawn = function ()
            local p = pilot.add("Admonisher ΩIIa", "Trader", nil, _("Astra Needle"), {naked=true, ai="pers_patrol"})
            p:outfitAddIntrinsic("Escape Pod")
            equipopt.empire( p, {
               outfits_add={"Pincushion Battery"},
               max_same_weap = 4,
               prefer={
                  ["Pincushion Battery"] = 100,
               },
            } )
            local m = p:memory()
            m.capturable = true
            m.comm_greet = _([["The Astra Vigilis always keeps an eye out for trouble."]])
            m.taunt = _("En garde!")
            return p
         end
      }, {
         spawn = function ()
            local p = pilot.add("Kestrel Sigma", "Trader", nil, _("Eulalia Lionheart"), {naked=true, ai="pers_patrol"})
            p:outfitAddIntrinsic("Escape Pod")
            p:intrinsicSet( "weapon_firerate", 25 )
            p:intrinsicSet( "energy_mod", 50 )
            p:intrinsicSet( "energy_regen_mod", 50 )
            p:intrinsicSet( "shield_mod", 50 )
            p:intrinsicSet( "shield_regen_mod", 50 )
            p:intrinsicSet( "armour_mod", 50 )
            equipopt.sirius( p, {
               pointdefence   = 10,
               fighterbay     = 0,
            } )
            local m = p:memory()
            m.capturable = true
            m.comm_greet = _([["The universe is a dangerous place, you better watch yourself."]])
            m.taunt = _("Have at thee!")
            return p
         end
      },
   }

   -- Miners yearn for the mines
   if #scur:asteroidFields() > 0 then
      table.insert( pers, {
         spawn = function ()
            local p = pilot.add("Mule Hardhat", "Trader", nil, _("Yearning Steve"), {naked=true, ai="pers_miner"})
            p:outfitAddIntrinsic("Escape Pod")
            equipopt.miner( p, {
               fighterbay=20,
            } )
            local m = p:memory()
            m.capturable = true
            m.comm_greet = _([["I yearn to mine!"]])
            m.taunt = _("Hey! I'm just mining some rocks here!")
            return p
         end
      } )
   end

   return pers
end
