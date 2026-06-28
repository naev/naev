local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_sirius_conversion_3",
   title          = _("The Wandering Lamb"),
   desc           = _("The Sirius government has placed an urgent bounty to capture a Dreamer Clan pirate lord. We have been cautioned that they are known to manifest powerful psychic abilities."),
   escorts        = _("with a sizable pirate crew"),
   reward         = 2e6,
   system         = system.get("Seifer"),
   name           = _("Dreamer Starheart"),
   payingfaction  = faction.get("Sirius"),
   reputation     = 200,
   targetfaction  = faction.get("Dreamer Clan"),
   alive_only     = true,
   ships          = { ship.get("Pirate Kestrel Galaxy Soul") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.pirate( p, {
         outfits_add = {
            "Emergency Stasis Inducer",
            "House of Mirrors"
         },
         prefer = {
            ["Emergency Stasis Inducer"] = 100,
            ["House of Mirrors"] = 100,
         },
         type_range = {
            ["Launcher"] = { max = 0 },
         },
      } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 2
      m.capturable = true
      local saying = _("I was one of them once, now my wings are no longer clipped. I can see so many colours in the astral.")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.pirate, 300 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_sirius_conversion_1")
         and var.peek("bounty_sirius_conversion_2")
   end,
}
