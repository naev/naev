local equipopt = require "equipopt"

return function ()
   local pers = {}

   local scur = system.cur()
   local presence = scur:presences()["Sirius"] or 0
   if presence <= 0 then
      return nil -- Need at least some presence
   end

   -- Medium ships here
   if presence > 100 then
      for k,v in ipairs{
         {
            spawn = function ()
               local p = pilot.add("Sirius Dogma", "Sirius", nil, _("Serra Scion"), {naked=true, ai="pers_patrol"})
               equipopt.sirius( p, { flow_ability=outfit.get("Avatar of Sirichana") } )
               local m = p:memory()
               m.comm_greet = _([["The Serra Echelon is the pride of House Sirius. As a Scion, I bear the weight of responsibility truthfully."]])
               m.taunt = _("You dare test my faith?! The Serra Scion shall not waiver!")
               m.bribe_no = _([["You think you can buy my faith with mere credits?"]])
               return p
            end,
         }, {
            spawn = function ()
               local p = pilot.add("Sirius Preacher", "Sirius", nil, _("Zealot Sri Chatri"), {naked=true, ai="pers_patrol"})
               equipopt.sirius( p, { flow_ability=outfit.get("Avatar of Sirichana") } )
               local m = p:memory()
               m.ad = {
                  _("Stay vigilant, for we are always tested against wickedness."),
                  _("We must be beacons of faith in the eternal darkness."),
               }
               m.comm_greet = _([["There will never be rest as long as the wicked remain unpunished. I will strive as long as faith drives me."]])
               m.taunt = _("Prepare for retribution!")
               m.bribe_no = _([["I am not swayed by your petty coins."]])
               m.uselanes = false
               for i=1,2 do
                  local s = pilot.add("Sirius Shaman", "Sirius", p:pos(), _("Squire"), {naked=true, ai="pers"})
                  equipopt.sirius( s, { noflow=true } )
                  s:setVel( p:vel() )
                  s:setLeader( p )
               end
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
