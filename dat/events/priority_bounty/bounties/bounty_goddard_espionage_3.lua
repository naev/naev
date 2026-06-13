local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_goddard_espionage_3",
   title          = _("Weapons Free"),
   desc           = _("Foreign agents have stolen House Goddard's test platform, an old demilitarized Goddard hull, which was currently loaded with experimental weapons not yet available to the public. They have two jobs for you: First, protect House Goddard intellectual property by preventing the escape of the ship. Second: Assuming you survive, report back on how effective the weapons were in a real firefight. If you die, they'll consider it a successful test."),
   escorts        = _("with a sizable escort"),
   reward         = 1.5e6,
   system         = system.get("Pas"),
   name           = _("Gunhead"),
   payingfaction  = faction.get("Goddard"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Goddard Merchantman") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.dvaered( p, {
         outfits_add = {
            "Railcannon"
         },
         prefer = {
            ["Railcannon"] = 100,
         },
         type_range = {
            ["Point Defense"] = { max = 0 },
            ["Launcher"] = { max = 0 },
         },
      } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 2
      m.capturable = true
      local saying = _("Just imagine these guns in every black market and on every Pirate Kestrel!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 250 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_goddard_espionage_2")
         and (var.peek("astra_vigilis_points") or 0) > 300
   end,
   completefunc = function ()
      diff.apply("railcannon_available")
      return true -- Doesn't block normal finishing
   end,
}
