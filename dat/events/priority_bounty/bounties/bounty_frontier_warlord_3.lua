local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_frontier_warlord_3",
   title          = _("Wrath and Railguns"),
   desc           = _("Lord Flakfinger has been dealt with, but now his aunt, Lady Railfist, is out for vengeance. Put a stop to this problem, once and for all."),
   escorts        = _("with a dangerous fleet"),
   reward         = 3.75e6,
   system         = system.get("Chraan"),
   name           = _("Lady Railfist"),
   payingfaction  = faction.get("Frontier"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Dvaered Goddard") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, { naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.dvaered( p, {
         outfits_add={"Hyena Bay"},
         prefer={
            ["Repeating Railgun"] = 100
         },
         type_range = {
            ["Bolt Weapon"] = { max = 4 }
         },
      } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 3
      m.capturable = true
      local saying = _("Fury and vengeance, wrath and flame! I will take everything from you, as you took my nephew from me!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.dvaered, 500 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_frontier_warlord_2")
         and (var.peek("astra_vigilis_points") or 0) > 400
   end,
}
