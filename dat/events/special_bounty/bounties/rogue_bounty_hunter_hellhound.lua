local bhelp = require "events.special_bounty.helpers"
local bounty = require "common.bounty"
return {
   var            = "bounty_hellhound", -- To control whether or not the player did it
   title          = _("To Hell and Back"),
   desc           = _("An ex-Astra Vigilis guild member has gone rogue after betraying a high priority bounty target. They are flying the Hellhound and believed to be accompanied by other rogue members. The Astra Vigilis wants them captured alive."),
   escorts        = _("with heavy escorts"),
   reward         = 1.3e6,
   system         = system.get("Alteris"),
   name           = _("Hellhound"),
   payingfaction  = faction.get("Traders Society"),
   reputation     = 50,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = true,
   ships          = { ship.get("Starbridge Sigma") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname )
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 200 )) do
         local e = pilot.add( s, fct, params )
         e:setLeader(p)
      end
      return p
   end,
}
