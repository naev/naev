local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_empire_baron",
   title          = _("The Baron's End"),
   desc           = _("Baron Sauterfeldt, the governor of the independent world Ulios in the Ingot system, is wanted for trial by the Empire under suspicion of masterminding theft from a Great House and trafficking in illegal Nebula artefacts. He is known to be in command of a rare military model of pre-Incident destroyer and is likely to be supported by larger vessels from the Ulios planetary defense fleet."),
   msg_subdue     = { _("Cowed by your defeat of their fleet, the Baron's officers and crew stand down. You've been aboard this ship before, and lead your boarding party directly to Sauterfeldt's lavishly-appointed office. A low-yield detonation charge blasts open the reinforced door, and you find the Baron whimpering and cowering in the corner, holding a pile of his precious artefacts tightly to his chest. Your crew drags him away in cuffs."), },
   escorts        = _("supported by the Ulios planetary defense fleet"),
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
            ["Bolt Turret"] = { min = 1 },
            ["Point Defence"] = { min = 1, max = 1 },
            ["Launcher"] = { min = 1 },
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
            ship.get("Hawking"),
            ship.get("Hawking"),
            ship.get("Pacifier"),
            ship.get("Pacifier"),
         },
         bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 40 ))
      ) do --And some smaller than destroyer-size ships
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return player.misnDone("Prince") and player.misnDone("Sharkman Is Back") --Must have completed Baron Sauterfeldt missions & 1st two Shark campaign missions.
         -- TODO rewrite dv_bikers so we don't need it as a prereq
         and player.misnDone("DVaered Negotiation 2")
         and bhelp.bounty_done() >= 10
   end,
   completefunc = function ()
      diff.apply("baron_gone")
      return true
   end,
}
