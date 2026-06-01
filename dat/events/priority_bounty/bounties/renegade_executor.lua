local fmt      = require "format"
local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_executor",
   title          = _("Executing the Executioner"),
   desc           = fmt.f(_([[SECURITY CLASSIFICATION SECRET.
The Astra Vigilis has received an exceptional order to eliminate a renegade Executioner. Target is flying an Empire Peacemaker and is expected to be extremely dangerous. The target is equipped with an experimental {outfit}. roceed with caution.]]), {
   outfit = "#o".._("shield aura generator").."#0",
} ),
   escorts        = _("with support"),
   reward         = 2e6,
   system         = system.get("Merisi"),
   name           = _("Executor Sable"),
   payingfaction  = faction.get("Empire"),
   reputation     = 200,
   targetfaction  = faction.get("Mercenary"),
   alive_only     = false,
   ships          = { ship.get("Empire Peacemaker") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, { naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.empire( p, {
         outfits_add={"Executor Shield Aura"},
         prefer={["Executor Shield Aura"] = 100}} )
      local m = p:memory()
      m.lootable_outfit = outfit.get("Executor Shield Aura")
      m.capturable = true
      local saying = _("It seems like the Empire can't even do the dirty work itself.")
      m.taunt = saying
      m.comm_greet = saying
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.empire, 200 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
      end
      return p
   end,
   cond = bhelp.cond_bounty_points( 500 ),
}
