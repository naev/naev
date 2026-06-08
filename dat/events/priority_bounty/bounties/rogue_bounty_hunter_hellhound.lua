local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_hellhound", -- To control whether or not the player did it
   title          = _("A Bark in the Dark"),
   desc           = _("An ex-Astra Vigilis guild member known as Hellhound has gone rogue after betraying a high priority bounty target. They are flying the Hellhound and believed to be accompanied by other rogue members. The Astra Vigilis wants them captured alive."),
   escorts        = _("with heavy escorts"),
   reward         = 1e6,
   system         = system.get("Alteris"),
   name           = _("Hellhound"),
   payingfaction  = faction.get("Traders Society"),
   reputation     = 100,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = true,
   ships          = { ship.get("Starbridge") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, { naked = true } )
      equipopt.soromid( p )
      p:outfitAddIntrinsic("Escape Pod")
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("What? Astra Vigilis sticking bounty hunters on me?")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 150 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, p )
      end
      return enemies
   end,
   cond = bhelp.cond_bounty_points( 200 ),
}
