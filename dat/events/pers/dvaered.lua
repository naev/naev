local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local presence = scur:presences()["Dvaered"] or 0
   if presence <= 0 then
      return nil -- Need at least some presence
   end

   -- Medium ships here
   if presence > 100 then
      for k,v in ipairs{
         {
            spawn = function ()
               -- Gets bonuses based on how many of their "pack" vendettas are alive
               local npack = 8
               local p = pilot.add("Dvaered Vigilance", "Dvaered", nil, _("Packleader"), {naked=true, ai="pers_patrol"})
               p:intrinsicSet( "armour_mod", 50*(npack-1) )
               p:intrinsicSet( "absorb", 10*(npack-1) )
               equipopt.dvaered( p, {turret=0} )
               local m = p:memory()
               m.comm_greet = _([["Think not of what the pack will do for you, but only what you can do for the pack."]])
               m.taunt = _("The pack will tear you limb from limb!")
               m.bribe_no = _([["We only wish for blood!"]])
               m.formation = "wedge"
               local pos = p:pos()
               local vel = p:vel()
               local pack = {p}
               for i=1,npack do
                  local e = pilot.add("Dvaered Vendetta", "Dvaered", pos )
                  local em = p:memory()
                  em.comm_no = _("*BARK*")
                  em.__packleader = p
                  e:setVel(vel)
                  e:setLeader(p)
                  table.insert( pack, p )
               end
               return pack
            end,
            ondeathany = function( _attacker, pt )
               local packleader = pt.p:memory().__packleader
               if packleader and packleader:exists() then
                  packleader:intrinsicSet( "armour_mod", -50 )
                  packleader:intrinsicSet( "absorb", -10 )
               end
            end,
         },
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
