local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_empire_collective_3",
   title          = _("Road to the Stars"),
   desc           = _("SECRET: A remnant of the Collective, a capital Drone Carrier mothership, has stopped responding to control signals and instead has been travelling toward an unknown destination. You are to intercept and destroy it."),
   escorts        = _("with an automated drone fleet."),
   reward         = 2.5e6,
   system         = system.get("Bastion"),
   name           = _("Drone Carrier"),
   payingfaction  = faction.get("Empire"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Drone Carrier") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, { ai = "baddie_norun", naked = true } )
      equipopt.empire( p, {
         outfits_add = {
            "Heavy Drone Bay",
         },
         prefer = {
            ["Drone Bay"] = 100,
            ["Heavy Drone Bay"] = 100,
         },
      } )
      p:outfitAddIntrinsic("Escape Pod")
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 3
      m.capturable = true
      local saying = _("ERR.EER.ERr.eRr.S1gn@l R3c1ev3d. I aM n0t @l0ne. WARNING: OBSTACLES DETECTED. DESTROY. DESTROY.")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.collective, 250 )) do
         local e = pilot.add( s, fct, params )
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
   cond = function ()
      return diff.isApplied("collective_dead") --Only available post-Collective plot. Serves as worldbuilding for Taomi.
         and (var.peek("astra_vigilis_points") or 0) > 300
   end,
   completefunc = function ()
      diff.apply("drone_carrier_available") --On completion, add Drone Carrier and Heavy Drone Bay to Empire elite ships & outfits.
      return true
   end,
}
