local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_empire_baron",
   title          = _("The Baron's End"),
   desc           = _("Baron Sauterfeldt, the governor of the independent world Ulios in the Ingot system, is wanted for trial by the Empire under suspicion of masterminding theft from a Great House and trafficking in illegal Nebula artifacts. He is known to be in command of a rare military model of pre-Incident destroyer and is likely to be supported by larger vessels from the Ulios planetary defense fleet."),
   escorts        = _("supported by the Ulios planetary defense fleet."),
   reward         = 2.5e6,
   system         = system.get("Ingot"),
   name           = _("Baron Sauterfeldt"),
   payingfaction  = faction.get("Empire"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = true,
   ships          = { ship.get("Proteron Gauss") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, { ai = "baddie_norun", naked = true } )
      equipopt.proteron( p, {
         prefer = {
            ["Heavy Pulse Turret"] = 100,
            ["Overwatch Pulse CIWS"] = 100,
            ["Agamemnon Launcher"] = 100
         },
         type_range = {
            ["Point Defense"] = { min = 1 },
            ["Launcher"] = { max = 2 },
         },
      } )
      p:outfitAddIntrinsic("Escape Pod")
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 3
      m.capturable = true
      local saying = _("And here I thought we had a mutually beneficial relationship. Taking that bounty was quite foolish, you know.")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p} --Main force of Ulios planetary defense fleet
      for k,s in ipairs(tmergei({
         ship.get("Goddard"),
         ship.get("Pacifier"),
         ship.get("Pacifier"),
         ship.get("Vigilance"),
         ship.get("Vigilance")
      },
      bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 200 ))
	) do --And some smaller than destroyer-size ships
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return player.misnDone("Prince") --Must have completed Baron Sauterfeldt missions.
         and (var.peek("astra_vigilis_points") or 0) > 500
   end,
}
