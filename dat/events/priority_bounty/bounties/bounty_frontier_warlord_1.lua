local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_frontier_warlord_1",
   title          = _("Flak and Frenzy"),
   desc           = _("Travellers entering Frontier space have been complaining of harassment and shakedowns from a self-proclaimed 'warlord'. Capture him alive if possible."),
   escorts        = _("with small escort"),
   reward         = 1.6e6,
   system         = system.get("Koit"),
   name           = _("Lord Flakfinger"),
   payingfaction  = faction.get("Frontier"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = true,
   ships          = { ship.get("Dvaered Vigilance") },
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
			["Repeating Banshee Launcher"] = 100
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
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("You must be one of those Frontier weaklings, too!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.dvaered, 150 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = bhelp.cond_bounty_points( 200 ),
}
