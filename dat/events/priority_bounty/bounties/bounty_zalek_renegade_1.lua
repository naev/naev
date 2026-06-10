local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_zalek_renegade_1",
   title          = _("AWOL Lab Partner"),
   desc           = _("A university graduate student has put a bounty out on their lab partner, who vanished in the middle of their thesis project on modular fuel synethesis dynamics. Bring her back alive, please."),
   escorts        = _("with a small group of shady pirates"),
   reward         = 1.1e6,
   system         = system.get("Severus"),
   name           = _("Graduate Student Sarah"),
   payingfaction  = faction.get("Za'lek"),
   reputation     = 100,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = true,
   ships          = { ship.get("Za'lek Sting Type IV") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.zalek( p )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("Hey, this was an important business meeting!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.pirate, 100 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = bhelp.cond_bounty_points( 200 ),
}
