local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
return {
   var            = "bounty_empire_collective_2",
   title          = _("A Collective Conspiracy"),
   desc           = _("SECRET: A deserter is attempting to make off with a squadron of experimental drone fighters. You are to proceed to Polack and destroy them. Once they are dealt with, you will forget anything you know about the traitor or this drone technology."),
   escorts        = _("with a few light drone escorts."),
   reward         = 1.4e6,
   system         = system.get("Polack"),
   name           = _("Major Gray"),
   payingfaction  = faction.get("Empire"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Empire Pacifier") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, { ai = "baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 2
      m.capturable = true
      local saying = _("You're just a bump in the road, merc. Once we have the Collective under our full control, the Emperor is next.")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.collective, 200 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return not diff.isApplied("collective_dead") --Only available pre-Collective plot. Serves as worldbuilding, Empire rep, and to guide players gently to finding it.
         and var.peek("bounty_empire_collective_1")
         and (var.peek("astra_vigilis_points") or 0) > 150
   end,
}
