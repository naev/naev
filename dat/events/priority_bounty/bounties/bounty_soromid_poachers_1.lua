local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_soromid_poachers_1",
   title          = _("Alternative Medicine"),
   desc           = _("The Soromid government has a poacher problem. Did you know there's wild populations of feral bioships at the edge of Soromid space? It seems some people believe that bioship organs have healing properties, and there's a thriving black market trade in them. The Soromid want these poachers dead."),
   escorts        = _("with a few small escorts"),
   reward         = 1.2e6,
   system         = system.get("NGC-4131"),
   name           = _("Doc Goodstuff"),
   payingfaction  = faction.get("Soromid"),
   reputation     = 100,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Pirate Starbridge") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.pirate( p, {
         outfits_add = {
			 "Plasma Eruptor"
		 },
         prefer = {
			["Plasma Eruptor"] = 100
		 },
         type_range = {
			["Plasma Weapon"] = { max = 2 },
		 },
	  } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("Look man, they're just animals! Nothing wrong with earning an honest living!")
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
