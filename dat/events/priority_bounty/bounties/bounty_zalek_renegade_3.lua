local fmt      = require "format"
local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_zalek_renegade_3",
   title          = _("Buyer's Remorse"),
   desc           = fmt.f(_([[Za'lek intelligence has located the stolen {outfit} in the possession of a Black Lotus crime lord. Za'lek authorities don't care about the final disposition of the prototype, but this contract stipulates that it not be left in pirate hands.]]),
{
   outfit = "#o".._("prototype modular ramscoop").."#0",
} ),
   escorts        = _("with a dangerous pirate fleet"),
   reward         = 2.5e6,
   system         = system.get("Nunavut"),
   name           = _("The Kingpin"),
   payingfaction  = faction.get("Za'lek"),
   reputation     = 200,
   targetfaction  = faction.get("Black Lotus"),
   alive_only     = false,
   ships          = { ship.get("Dealbreaker") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.black_lotus( p, {
         outfits_add = {
            "Modular Ramscoop"
         },
         prefer = {
            ["Modular Ramscoop"] = 100
         },
         type_range = {
            ["Fuel Modifier"] = { max = 1 },
         },
      } )
      local m = p:memory()
      m.lootable_outfit = outfit.get("Modular Ramscoop")
      m.capturable = true
      local saying = _("This is my prize now, and you're only getting it over my dead body. Or, preferably, yours.")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.pirate, 400 )) do
         local e = pilot.add( s, fct, params )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_zalek_renegade_2")
         and (var.peek("astra_vigilis_points") or 0) > 400
         and player.fleetCapacity() > 0 -- Don't allow earning the prototype ramscoop until Chapter 1.
   end,
}
