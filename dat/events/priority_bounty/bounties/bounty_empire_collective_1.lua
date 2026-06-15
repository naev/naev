local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_empire_collective_1",
   title          = _("Haywire Drones"),
   desc           = _("SECRET: An experimental drone fighter has been misplaced, and the Empire would like it destroyed quietly and without any fuss. Your prompt attention to and silence on this matter is appreciated."),
   escorts        = _("with a few light drone escorts."),
   reward         = 0.8e6,
   system         = system.get("Chloe"),
   name           = _("Heavy Drone"),
   payingfaction  = faction.get("Empire"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Heavy Drone") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      equipopt.collective( p )
      p:outfitAddIntrinsic("Escape Pod")
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("BEEP. BEEP. BEEP.")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.collective, 100 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return not diff.isApplied("collective_dead")
         and (var.peek("astra_vigilis_points") or 0) > 100
   end,
}
