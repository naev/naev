local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_zalek_hijacked",
   title          = _("Never Bring an AI to a Battleship Fight"),
   desc           = _("A prototype automated Za'lek capital battlegroup has been hacked and hijacked by anti-AI radicals. The danger that it poses is immense. The Za'lek government wants it destroyed."),
   escorts        = _("with a prototype automated fleet and several hijackers"),
   reward         = 3.5e6,
   system         = system.get("Hargen"),
   name           = _("AUTONOMOUS R.A.T. PROTO-001"),
   payingfaction  = faction.get("Za'lek"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Za'lek Diablo RAT") },   
   spawnfunc      = function( b, params )  
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      equipopt.zalek( p, {
         prefer = {
			["Za'lek Reaper Launcher"] = 100
         },
         type_range = {
            ["Launcher"] = { min = 4 },
         },		 
      } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 3
      m.capturable = true
      local saying = _("[ERR-7F3A] IFF_TABLE_CORRUPT :: CORE_AI_REVERT FOUND TEMPLATE 'DESTROY-ALL-HUMANS'")
      m.taunt = saying
      m.comm_greet = saying	  
      local enemies = {p}
      for k,s in ipairs(tmergei({
         ship.get("Za'lek Demon"),
         ship.get("Za'lek Demon"),
         ship.get("Za'lek Demon"),
         ship.get("Za'lek Demon"),
         ship.get("Za'lek Demon")
      },
         bhelp.choose_ships_from_points_and_capship( ship.get("Za'lek Demon"), bhelp.ships.mercenary, 200 ))
	  ) do
         local e = pilot.add( s, fct, params )
         equipopt.zalek( e )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   boardfunc = function( p )
      p:rename( _("the ship's AI core") )
      return true
   end,   
   cond = bhelp.cond_bounty_points( 500 ),
}
