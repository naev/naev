local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_hellhound2",
   title          = _("To Hell and Back"),
   desc           = _("The ex-Astra Vigilis guild member Hellhound has escaped their confinement and are once again on the loose. The Astra Vigilis needs them captured alive again."),
   escorts        = _("with heavier escorts"),
   reward         = 1.5e6,
   system         = system.get("Pas"),
   name           = _("Hellhound"),
   payingfaction  = faction.get("Traders Society"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = true,
   ships          = { ship.get("Starbridge Sigma") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, { naked = true } )
      equipopt.soromid( p )
      p:outfitAddIntrinsic("Escape Pod")
      local m = p:memory()
      m.capturable = true
      local saying = _("You again!?")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 300 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, p )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_hellhound")
         and (var.peek("astra_vigilis_points") or 0) > 400
   end,
}
