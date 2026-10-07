local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_goddard_espionage_4",
   title          = _("Godwedded"),
   desc           = _("A rogue cell has recently captured an experimental carrier variant of the Goddard Battleship refitted with fighter optimization technology. While the ship is old and House Goddard would not normally be too fussed about one of their mothballed hulks being hauled off, there are worries that the carrier was stolen by a rogue Za'lek cell with an interest in reverse-engineering the proprietary technology. The ship must be either destroyed or taken off the hands of anyone interested in researching it."),
   msg_subdue     = { _("As you break into the ship, you shoot down a number of personal defense drones. The ship seems severely understaffed, with small maintenance drones making up the bulk of the workforce. With them soundly defeated, you push in. The person at the helm bears Za'lek cybernetic augmentations, but otherwise is wearing much more raggy, pirate-appropriate attire. They attempt to start a long-winded diatribe about the value of the technology aboard the vessel, but you're not in the mood for an unintelligible lecture today, subduing them with a shot in the arm and rapidly taking them into custody. It seems none of their implements were built to help them in a fight."), },
   escorts        = _("with a sizable escort"),
   reward         = 2e6,
   system         = system.get("Attaria"),
   name           = _("Godfather"),
   payingfaction  = faction.get("Goddard"),
   reputation     = 250,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Godmother") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.dvaered( p, {
         outfits_add = {
            "Ancestor Bomber Bay",
            "Pirate Ancestor Dock"
         },
         prefer = {
            ["Ancestor Bomber Bay"] = 100,
            ["Pirate Ancestor Dock"] = 100,
         },
         type_range = {
            ["Fighter Bay"] = { min = 4 },
         },
      } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 3
      m.capturable = true
      local saying = _("You have no idea what sort of technology we're dealing with here!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 60 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_goddard_espionage_3")
         and bhelp.bounty_done() >= 10
   end,
   completefunc = function ()
      return true -- Doesn't block normal finishing
   end,
}
