local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_dvaered_flf_2",
   title          = _("Raising the Dead"),
   desc           = _("'Ghost' is back, and now in command of some kind of previously unseen battleship, deeper into the Nebula. We don't know where he found it, but the Dvaered can not allow these FLF terrorists to possess this kind of technology."),
   escorts        = _("with a terrorist fleet"),
   reward         = 2.8e6,
   system         = system.get("Zylex"),
   name           = _("Ghost"),
   payingfaction  = faction.get("Dvaered"),
   reputation     = 200,
   targetfaction  = faction.get("FLF"),
   alive_only     = false,
   ships          = { ship.get("Thurion Certitude") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      equipopt.thurion( p )
      p:outfitAddIntrinsic("Escape Pod")
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 2
      m.capturable = true
      local saying = _("I'm so close to finding it! And once I do, we'll have the power to crush the Dvaered and Empire alike!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.flf, 250 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_dvaered_flf_1")
         and (var.peek("astra_vigilis_points") or 0) > 400
   end,
}
