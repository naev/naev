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
         }, {
            spawn = function ()
               -- PI = principal investigator
               local p = pilot.add("Za'lek Diablo", "Za'lek", nil, _("PI Newton"), {naked=true, ai="pers_patrol"})
               equipopt.zalek( p, {
                  outfits_add={"Neural Accelerator Interface"},
                  prefer={["Neural Accelerator Interface"] = 100}} )
               local m = p:memory()
               m.comm_greet = _([["What do you want? Can't you see I'm busy writing a grant?"]])
               m.taunt = _("Do not get in the way of science!")
               m.bribe_prompt = _([["I could use {credits} more in funding."]])
               return p
            end,
         }
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
               local pos = p:pos()
               local vel = p:vel()
               for i=1,3 do
                  local e = pilot.add("Za'lek Bomber Drone", "Za'lek", pos )
                  e:setVel(vel)
                  e:setLeader(p)
               end
               return p
            end,
         }, {
            spawn = function ()
               -- Ananka is a female name that apparently stands for countless or infinite, like a postdoc
               local p = pilot.add("Za'lek Sting", "Za'lek", nil, _("Postdoc Ananka"), {naked=true, ai="pers"})
               equipopt.zalek( p, {
                  outfits_add={"Combat Hologram Projector"},
                  prefer={["Combat Hologram Projector"] = 100}} )
               local m = p:memory()
               m.ad = { _("Oh shit, did I miss another deadline?"),
                        _("I'll never get into tenure track with my current Z-index…"),
                        _("Seventh time in a row my papers was rejected…"), }
               m.comm_greet = _([["Even when I close my eyes, the endless deadlines haunt my dreams."]])
               m.taunt = _("Just put me out of this misery.")
               m.bribe_prompt = _([["If you pay off {credits} of my student loans, I'll go back to my deadlines."]])
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
