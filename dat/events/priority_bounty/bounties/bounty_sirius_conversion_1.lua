local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_sirius_conversion_1",
   title          = _("Conversion Therapy"),
   desc           = _("The Sirius government has placed a bounty on a notorious smuggler who they say is 'destabilizing the social fabric of the community'. They insist that she be delivered alive for therapeutic intervention."),
   escorts        = _("with a few other undesirables"),
   reward         = 1.2e6,
   system         = system.get("Suna"),
   name           = _("Free Trader Haskins"),
   payingfaction  = faction.get("Sirius"),
   reputation     = 100,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = true,
   ships          = { ship.get("Pirate Rhino") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.pirate( p, {
         outfits_add = {
            "Blink Drive",
            "Bio-Neural Combat AI",
            "Milspec Scrambler",
            "Reconstructive Nanobot Coating",
            "Compact Lightsail",
         },
         prefer = {
			["Scanning Combat AI"] = 0,
			["Blink Drive"] = 100,
			["Bio-Neural Combat AI"] = 100,
			["Milspec Scrambler"] = 100,
			["Reconstructive Nanobot Coating"] = 100,
			["Compact Lightsail"] = 100,
			["Reactor Class III"] = 100,
			["Heavy Laser Turret"] = 100
         },
      } )
      local m = p:memory()
      if not m.lootables then
         m.lootables = {}
      end
      m.lootables["encrypted_data_matrix"] = 1
      m.capturable = true
      local saying = _("Think for yourself, what they do to people isn't right! I won't let you take me alive!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.mercenary, 150 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = bhelp.cond_bounty_points( 100 ),
}
