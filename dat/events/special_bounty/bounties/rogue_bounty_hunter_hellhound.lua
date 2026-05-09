local bhelp = require "events.special_bounty.helpers"
return {
   var            = "bounty_hellhound",
   title          = _("To Hell and Back"),
   desc           = _("An ex-Astra Vigilis guild member has gone rogue after betraying a high priority bounty target."),
   escorts        = _("with heavy escorts"),
   reward         = 800e3,
   system         = system.get("Alteris"),
   name           = _("Hellhound"),
   payingfaction  = faction.get("Traders Society"),
   reputation     = 50,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Starbridge Sigma") },
   spawnfunc      = function( b, params )
      local fct = b.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname )
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 200 )) do
         local e = pilot.add( s, fct, params )
         e:setLeader(p)
      end
      return p
   end,
}
