local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_goddard_espionage_2",
   title          = _("Stolen Blueprints"),
   desc           = _("House Goddard has interrogated the spy you captured for them, and learned that another foreign agent has just managed to breach Goddard systems and has copied secure House Goddard blueprints for an unreleased technology in development. It may not be too late to catch the spy. Kill them if necessary."),
   escorts        = _("with a small fleet"),
   reward         = 1.1e6,
   system         = system.get("Overture"),
   name           = _("Shadowrunner"),
   payingfaction  = faction.get("Goddard"),
   reputation     = 100,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Pirate Starbridge") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.dvaered( p, {
         outfits_add = {
            "Milspec Scrambler",
            "Neural Accelerator Interface"
         },
         prefer = {
            ["Milspec Scrambler"] = 100,
            ["Neural Accelerator Interface"] = 100
         },
      } )
      local m = p:memory()
      if not m.lootables then
      m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 2
      m.capturable = true
      local saying = _("These prototype weapon plans are worth a lot to my buyer!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 150 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,  
   cond = function ()
      return var.peek("bounty_goddard_espionage_1")
         and (var.peek("astra_vigilis_points") or 0) > 200
   end,
}