local fmt      = require "format"
local bhelp    = require "events.priority_bounty.helpers"
local bounty   = require "common.bounty"
local equipopt = require "equipopt"
return {
   var            = "bounty_dvaered_flf_3",
   title          = _("Breaking Down the Graveyard Gate"),
   desc           = fmt.f(_([[SECURITY CLASSIFICATION SECRET.
Empire intelligence has shared information with Dvaered High Command and Astra Vigilis that a senior FLF military leader, codenamed Emerald Sword, has located and recovered some kind of ancient superweapon from somewhere in the depths of the Nebula. From what records the Empire has, it may have some kind of {outfit}. Capture them alive for interrogation.
This will be a difficult battle, captain.]]), {
   outfit = "#o".._("nuclear based armament and autonomous drone fighter-bombers").."#0",
} ),
   escorts        = _("with an enormous terrorist fleet"),
   reward         = 5e6,
   system         = system.get("Arandon"), --Fleet is on its way back from somewhere in the Nebula where the Emerald Sword was recovered. This system has enough volatility to discourage player fighter spam.
   name           = _("Emerald Sword"),
   payingfaction  = faction.get("Dvaered"),
   reputation     = 200,
   targetfaction  = faction.get("FLF"),
   alive_only     = true,
   ships          = { ship.get("Emerald Sword") },
   spawnfunc      = function( b, params )
      local fct = bounty.get_faction()
      local p = pilot.add( b.targetship[1], fct, params, b.targetname, {ai="baddie_norun", naked = true } )
      p:outfitAddIntrinsic("Escape Pod")
      equipopt.empire( p, {
         outfits_add = {
            "Black Diamond Bay",
            "Biometal Armour",
            "Nebula Resistant Coating",
            "Agility Combat AI"
         },
         prefer = {
            ["Black Diamond Bay"] = 100,
            ["Biometal Armour"]   = 100,
            ["Photo-Voltaic Nanobot Coating"] = 0,
            ["Nebula Resistant Coating"] = 100,
            ["Reactor Class III"] = 100,
            ["Hunting Combat AI"] = 0,
            ["Agility Combat AI"] = 100
         },
         type_range = {
            ["Armour Modifier"] = { max = 2 },
            ["Power Modifier"] = { min = 3 },
         },
      } )
      local m = p:memory()
      m.lootable_outfit = outfit.get("Black Diamond Bay")
      m.capturable = true
      local saying = _("Now witness the power of this fully armed and operational battleship!")
      m.taunt = saying
      m.comm_greet = saying
      local enemies = {p}
      for k,s in ipairs(bhelp.choose_ships_from_points_and_capship( p:ship(), bhelp.ships.flf, 600 )) do
         local e = pilot.add( s, fct, params )
         e:outfitAddIntrinsic("Wild Space Taint") --Help support fleet survive Arandon. Also, they've apparently spent a little too long in the deep Nebula...
         equipopt.pirate( e, {
            outfits_add={"Nebula Resistant Coating"},
            prefer={
               ["Nexus Concealment Coating"] = 0,
               ["Nebula Resistant Coating"] = 100,
            },
         } )
         e:memory().capturable = true
         e:setLeader(p)
         table.insert( enemies, e )
      end
      return enemies
   end,
   cond = function ()
      return var.peek("bounty_dvaered_flf_2")
         and (var.peek("astra_vigilis_points") or 0) > 600
         and player.fleetCapacity() > 0 -- To be able to capture
   end,
}
