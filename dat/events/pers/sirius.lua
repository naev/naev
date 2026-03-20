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
               p:outfitAddIntrinsic("Escape Pod")
               p:intrinsicSet("shield_mod",25)
               equipopt.sirius( p, { flow_ability=outfit.get("Avatar of the Sirichana") } )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["The Serra Echelon is the pride of House Sirius. As a Scion, I bear the weight of responsibility truthfully."]])
               m.taunt = _("You dare test my faith?! The Serra Scion shall not waiver!")
               m.bribe_no = _([["You think you can buy my faith with mere credits?"]])
               return p
            end,
         }, {
            spawn = function ()
               local p = pilot.add("Sirius Preacher", "Sirius", nil, _("Zealot Sri Chatri"), {naked=true, ai="pers_patrol"})
               p:outfitAddIntrinsic("Escape Pod")
               p:intrinsicSet("shield_mod",25)
               equipopt.sirius( p, { flow_ability=outfit.get("Avatar of the Sirichana") } )
               local m = p:memory()
               m.capturable = true
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
                  local em = s:memory()
                  em.capturable = true
                  equipopt.sirius( s, { noflow=true } )
                  s:setVel( p:vel() )
                  s:setLeader( p )
               end
               return p
            end,
         }, {
            spawn = function ()
               local p = pilot.add("Sirius Dogma", "Sirius", nil, _("Zealot Sri Kaeya"), {naked=true, ai="pers_patrol"})
               p:outfitAddIntrinsic("Escape Pod")
               p:intrinsicSet("shield_mod",25)
               equipopt.sirius( p, { flow_ability=outfit.get("Astral Projection") } )
               local m = p:memory()
               m.capturable = true
               m.ad = {
                  _("The Sirichana is all that keeps us from succumbing to evil! All Praise the Sirichana!"),
                  _("The hordes of darkness are coming! Your faith must not sway!"),
               }
               m.comm_greet = _([["The light of the Sirichana will protect us all."]])
               m.taunt = _("Time for cleansing!")
               m.bribe_no = _([["Grimy credits will not sway me."]])
               m.uselanes = false
               for i=1,3 do
                  local s = pilot.add("Sirius Preacher", "Sirius", p:pos(), _("Fidel"), {ai="pers"})
                  local em = s:memory()
                  em.capturable = true
                  s:setVel( p:vel() )
                  s:setLeader( p )
               end
               return p
            end,
         }, {
            spawn = function ()
               local p = pilot.add("Starbridge Herald", "Sirius", nil, _("Star Voyager"), {naked=true, ai="pers"})
               p:outfitAddIntrinsic("Escape Pod")
               p:intrinsicSet("shield_mod",25)
               equipopt.sirius( p, { flow_ability=outfit.get("Astral Projection") } )
               local m = p:memory()
               m.capturable = true
               m.comm_greet = _([["The skies of Mutris are ever so beautiful."]])
               m.taunt = _("Such senseless violence!")
               return p
            end,
         }
      } do
         table.insert( pers, v )
      end
   end

   return pers
end
