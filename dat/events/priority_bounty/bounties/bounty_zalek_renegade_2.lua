local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_zalek_renegade_2",
   title          = _("The Stolen Prototype"),
   desc           = _("Two students were working on a modular prototype ramscoop as their graduate thesis project. One of them stole it, and is believed to be trying to fence it on the black market. Za'lek authorities don't care about the renegade student or the prototype: they just don't want the Black Lotus Clan to have it. Crash the sale and blast everyone out of the sky."),
   escorts        = _("with a group of pirates"),
   reward         = 1.9e6,
   system         = system.get("Somard"),
   name           = _("Graduate Student Sarah"),
   payingfaction  = faction.get("Za'lek"),
   reputation     = 200,
   targetfaction  = faction.get("Black Lotus"),
   alive_only     = false,
   ships          = { ship.get("Za'lek Demon") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.black_lotus( p )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 2
      m.capturable = true
      local saying = _("Too late, I've already been paid! Screw academia, I'm set for life now!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.pirate, 200 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_zalek_renegade_1")
         and (var.peek("astra_vigilis_points") or 0) > 300
   end,
}
