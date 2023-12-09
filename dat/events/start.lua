--[[
<?xml version='1.0' encoding='utf8'?>
<event name="start_event">
 <location>none</location>
</event>
--]]
--[[
   Event run when creating a new pilot.
--]]

local names = {
   _("Aluminum Mallard"), -- Because we might as well allude to an existing parody. Proper spelling would be "Aluminium", by the way.
   _("Armchair Traveller"),
   _("Attitude Adjuster"),
   _("Commuter"),
   _("Death Trap"),
   _("Eclipse"),
   _("Exitprise"),
   _("Fire Hazard"),
   _("Gunboat Diplomat"),
   _("Heart of Lead"),
   _("Icarus"),
   _("Little Rascal"),
   _("Myrmidon"),
   _("Opportunity"),
   _("Outward Bound"),
   _("Pathfinder"),
   _("Planet Jumper"),
   _("Rustbucket"),
   _("Serendipity"),
   _("Shove Off"),
   _("Sky Cutter"),
   _("Terminal Velocity"),
   _("Titanic MLXVII"),
   p_("ship name", "Vagabond"),
   _("Vindicator"),
   _("Windjammer"),
   _("Tuna Can"),
   _("Vermilion Dwarf"), -- Red Dwarf
}
local function name()
   return names[rnd.rnd(1,#names)]
end

function create()
   local pp = player.pilot()
   pp:rename( name() ) -- Assign a random name to the player's ship.
   pp:outfitRm( "all" ) -- Not naked at the start
   pp:outfitAdd( "Laser Cannon MK1", 1 ) -- Tutorials tell the player to buy and equip ion cannon later, need one weapon slot empty
   jump.setKnown( "Delta Polaris", "Jade" )

   -- Set player-specific settings
   var.push( "autonav_reset_shield", 1 )
   var.push( "autonav_reset_dist", 3e3 )
   var.push( "autonav_compr_speed", 5e3 )
   var.push( "autonav_compr_max", 50 )

   -- Give all GUIs
   -- TODO: Would be better to remove these outfits and the association,
   -- but they're so tightly integrated atm (with no other way to define
   -- GUIs as usable) that I'm implementing it this way for now.
   player.outfitAdd( "GUI - Brushed" )
   player.outfitAdd( "GUI - Slim" )
   player.outfitAdd( "GUI - Slimv2" )
   player.outfitAdd( "GUI - Legacy" )

   hook.timer(3, "timer_tutorial")
end

function timer_tutorial()
   naev.missionStart( "Tutorial" )
   evt.finish( true )
end
