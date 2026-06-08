local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_sirius_starbridge_herald",
   title          = _("Nubulous Iconoclast"),
   desc           = _("The Sirius government has placed a bounty on one Sister Brightstar, wanted dead or alive for heretical teachings."),
   escorts        = _("with her congregation"),
   reward         = 1.4e6,
   system         = system.get("Atlantis"),
   name           = _("Sister Brightstar"),
   payingfaction  = faction.get("Sirius"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Starbridge Herald") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, { naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.sirius( p, {
         outfits_add={"Pincushion Battery"},
         prefer={["Pincushion Battery"] = 100},
         type_range = {["Launcher"] = { max = 2 } }, } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("The Sirichana is dead, and the Nebula is his corpse!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.sirius, 150 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = bhelp.cond_bounty_points( 250 ),
}
