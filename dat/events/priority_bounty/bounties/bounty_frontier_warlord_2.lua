local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_frontier_warlord_2",
   title          = _("Flak 2, Explosive Boogaloo"),
   desc           = _("Lord Flakfinger's rich aunt paid his bail, and he went right back to shooting at Frontier merchant traffic, this time in a larger ship. Dead or alive this time."),
   escorts        = _("with a hired fleet"),
   reward         = 2.3e6,
   system         = system.get("Nougat"),
   name           = _("Lord Flakfinger"),
   payingfaction  = faction.get("Frontier"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Dvaered Retribution") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.dvaered( p, {
         outfits_add = {
            "Flak Gun"
         },
         prefer = {
            ["Flak Gun"] = 100,
            ["Super-Fast Collider Launcher"] = 100
         },
         type_range = {
            ["Bolt Weapon"] = { min = 4 },
            ["Launcher"] = { max = 2 },
         },
      } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 2
      m.capturable = true
      local saying = _("You peasants will pay for what you did to me!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.dvaered, 300 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_frontier_warlord_1")
         and (var.peek("astra_vigilis_points") or 0) > 300
   end,
}
