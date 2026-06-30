local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_soromid_poachers_3",
   title          = _("Stop the Slaughter"),
   desc           = _("We've been given the last known location of one 'Doctor Slaughter', a Za'lek renegade who appears to be behind the largest poaching operation in Soromid space."),
   escorts        = _("with a large posse of poachers"),
   reward         = 2.5e6,
   system         = system.get("NGC-13322"),
   name           = _("Doctor Slaughter"),
   payingfaction  = faction.get("Soromid"),
   reputation     = 100,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Za'lek Diablo") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.soromid( p )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 3
      m.capturable = true
      local saying = _("Countless discoveries await locked inside these creatures' flesh and organs. What could be more important than the advancement of knowledge?")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.pirate, 400 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_soromid_poachers_2")
         and (var.peek("astra_vigilis_points") or 0) > 400
   end,
}
