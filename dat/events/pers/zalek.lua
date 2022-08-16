local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local presence = scur:presences()["Za'lek"] or 0
   if presence <= 0 then
      return nil -- Need at least some presence
   end

   if presence > 100 then
      for k,v in ipairs{
         {
            spawn = function ()
               -- ZS stands for Za'lek ship
               local p = pilot.add("Za'lek Diablo", "Za'lek", nil, _("ZS Curie"), {naked=true, ai="pers_patrol"})
               equipopt.zalek( p, {
                  outfits_add={"Emergency Shield Booster"},
                  prefer={["Emergency Shield Booster"] = 100}} )
               local m = p:memory()
               m.comm_greet = _([["Nothing in life is to be feared; it is only to be understood."]])
               m.taunt = _("Your death shall be swift and easy!")
               m.bribe_no = _([["You must be eliminated. For science!"]])
               return p
            end,
         },
      } do
         table.insert( pers, v )
      end
   end

   if presence > 0 then
      for k,v in ipairs{
         {
            spawn = function ()
               -- ZHD stands for Za'lek Heavy Drone
               local p = pilot.add("Za'lek Heavy Drone", "Za'lek", nil, _("ZHD-08-5820"), {naked=true, ai="pers_patrol"})
               p:intrinsicSet( "shield_mod", 100 )
               equipopt.zalek( p )
               local m = p:memory()
               m.ad = _("Damn it's a good day today. Erm, I mean. *BEEP* *BEEP* SCANNING SYSTEM. *BEEP*")
               m.comm_greet = _([["Hello! I mean *BEEP* COMMUNICATION AUTHORIZED. *BEEP*"]])
               m.taunt = _("Die, scum! I mean *BEEP* EXTERMINATING *BEEP*")
               m.bribe_prompt = _([["I'll pretend to malfunction for {credits}, deal?"]])
               m.bribe_prompt_nearby = m.bribe_prompt
               local pos = p:pos()
               local vel = p:vel()
               for i=1,3 do
                  local e = pilot.add("Za'lek Bomber Drone", "Za'lek", pos )
                  e:setVel(vel)
                  e:setLeader(p)
               end
               return p
            end,
         },
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
