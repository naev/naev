local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
return {
   var            = "bounty_empire_collective_1",
   title          = _("Abominable Intelligence"),
   desc           = _("SECRET: An experimental wing of drone fightercraft have been misplaced, and the Empire would like them destroyed quietly and without any fuss. Your prompt attention to and silence on this matter is appreciated."),
   msg_subdue     = { _("You open an access hatch on the drone. The interior is cramped with equipment and a couple of spider-like maintenance bots which evidently shut down when the ship was disabled, but the vessel does appear to have inactive life support systems and an uncomfortable berth for a pilot. The cockpit has an empty seat, with a spherical device affixed to the dash and wired into all of the systems with a mess of cables. You unceremoniously rip it out. Maybe the Empire wants it back."), },
   msg_captured   = { _("Your Astra Vigilis contact seems surprised when you drop a fist-sized computer core in front of them."), },
   msg_killed     = { _("the target"), },
   escorts        = _("with a few light drone escorts."),
   reward         = 0.8e6,
   system         = system.get("Chloe"),
   name           = _("Heavy Drone"),
   payingfaction  = faction.get("Empire"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Heavy Drone") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, { ai = "baddie_norun" } )
      p:outfitAddIntrinsic("Escape Pod")
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("IFF_ERR[37601]: PARAM UNDEFINED. HOSTILES DETECTED. ENGAGING.")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.collective, 20 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return not diff.isApplied("collective_dead") --Only available pre-Collective plot. Serves as worldbuilding, Empire rep, and to guide players gently to finding it.
         and (var.peek("astra_vigilis_points") or 0) > 100
   end,
}
