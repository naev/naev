local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
--local equipopt = require "equipopt"
return {
   var            = "bounty_rainbow_goatee",
   title          = _("One Goatee to Rule them All"),
   desc           = _("A pirate warlord known as Rainbow Goatee is planning a large scale revenge raid after his disciples were defeated. Seeing this as a major threat, several guilds of the Space Trader's Society have funded a large bounty to neutralize the upcoming menace."),
   escorts        = _("with heavy support"),
   reward         = 2.5e6,
   system         = system.get("Guntesh"),
   name           = _("Rainbow Goatee"),
   payingfaction  = faction.get("Traders Society"),
   reputation     = 150,
   targetfaction  = faction.get("Marauder"),
   alive_only     = false,
   ships          = { ship.get("Dealbreaker") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname )
      p:outfitAddIntrinsic("Escape Pod")
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 3
      m.capturable = true
      local saying = _("The Goatees shall never be forgotten!")
      m.taunt = saying
      m.comm_greet = saying

      -- So the escorts are actually all the previous goatees + the new boss
      local enemies = {p}
      for k,s in ipairs(tmergei({
         ship.get("Pirate Zebra"),
         ship.get("Pirate Kestrel"),
         ship.get("Pirate Kestrel"),
         ship.get("Pirate Starbridge"),
         ship.get("Pirate Starbridge")
      },
         -- Have the rest be smaller than the starbridge so there's a bit of weird cruft there
         bhelp.choose_ships_from_points_and_capship( ship.get("Pirate Starbridge"), bhelp.ships.mercenary, 200 ))
      ) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      -- Have to defeat all the goatees first
      return player.evtDone("Quai Pirates") and player.evtDone("Levo Pirates") and player.evtDone("Capricorn Pirates") and player.evtDone("Surano Pirates") and player.evtDone("Dendria Pirates")
   end ,
}
