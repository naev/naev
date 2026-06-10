local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_soromid_poachers_2",
   title          = _("Wildlife Preservation"),
   desc           = _("Soromid security has asked Astra Vigilis to help again with their ongoing anti-poaching operations. A Kestrel has been spotted capturing live feral bioship young. Take it out."),
   escorts        = _("with a moderate fleet of poachers"),
   reward         = 1.9e6,
   system         = system.get("NGC-14479"),
   name           = _("Zoomonger"),
   payingfaction  = faction.get("Soromid"),
   reputation     = 100,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Pirate Kestrel Yuri's Kiss") },
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
      m.lootables["encrypted_data_matrix"] = 2
      m.capturable = true
      local saying = _("The money from those scientists is too good to pass up. You'll regret ever seeing me here!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.pirate, 250 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_soromid_poachers_1")
         and (var.peek("astra_vigilis_points") or 0) > 250
   end,
}
