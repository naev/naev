local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_sirius_conversion_2",
   title          = _("New Disciples"),
   desc           = _("The Sirius government has placed a bounty on a pirate operating in the southern reaches of Sirian space. We have been told that this pirate has been 'leading the flock astray', and is to be delivered alive for re-education."),
   escorts        = _("with a small band of pirates"),
   reward         = 1.2e6,
   system         = system.get("Lapis"),
   name           = _("Captain Freejoy"),
   payingfaction  = faction.get("Sirius"),
   reputation     = 100,
   targetfaction  = faction.get("Pirate"),
   alive_only     = true,
   ships          = { ship.get("Pirate Bedivere") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.pirate( p, {
         outfits_add = {
            "Blink Drive",
            "Cyclic Combat AI",
            "Milspec Scrambler",
            "Reaver Cannon",
            "Milspec Impacto-Plastic Coating"
         },
         prefer = {
            ["Scanning Combat AI"] = 0,
            ["Blink Drive"] = 100,
            ["Cyclic Combat AI"] = 100,
            ["Milspec Scrambler"] = 100,
            ["Milspec Impacto-Plastic Coating"] = 100,
            ["Reaver Cannon"] = 100
         },
      } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("What's so wrong with a life of freedom? People should be able to do what they please!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.pirate, 150 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = bhelp.cond_bounty_points( 100 ), --does not require conversion_1, either can be done in any order. Both will need to be complete for conversion_3
}
