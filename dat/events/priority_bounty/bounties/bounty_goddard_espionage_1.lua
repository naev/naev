local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_goddard_espionage_1",
   title          = _("Intellectual Property"),
   desc           = _("House Goddard, the shipyard responsible for the Goddard battleship and related intellectual property, has asked us to apprehend one 'Norm Alphurson', nominally there as a purchaser for an independent station. Apparently he and his escort have been lingering around the Goddard shipyards, taking suspicious deep scans of Goddard property."),
   escorts        = _("with a small escort"),
   reward         = 0.6e6,
   system         = system.get("Goddard"),
   name           = _("Norm Alphurson"),
   payingfaction  = faction.get("Goddard"),
   reputation     = 100,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = true,
   ships          = { ship.get("Admonisher ΩIIa") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.dvaered( p, {
         outfits_add = {
            "Heavy Ripper Cannon",
            "Blink Drive",
            "Emergency Stasis Inducer"
         },
         prefer = {
            ["Heavy Ripper Cannon"] = 100,
            ["Blink Drive"] = 100,
            ["Emergency Stasis Inducer"] = 100
         },
      } )
      local m = p:memory()
      if not m.lootables then
      m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("There's nothing wrong with just looking, is there?")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 60 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = bhelp.cond_bounty_points( 50 ),
}
