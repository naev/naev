--[[
<?xml version='1.0' encoding='utf8'?>
<event name="start_event">
 <trigger>none</trigger>
</event>
--]]
function name()
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
      _("Vagabond"),
      _("Vindicator"),
      _("Windjammer"),
      _("Tuna Can"),
      _("Vermilion Dwarf"), -- Red Dwarf
   }
   return names[rnd.rnd(1,#names)]
end


function create()
   local pp = player.pilot()
   pp:rename( name() ) -- Assign a random name to the player's ship.
   pp:outfitAdd( "Laser Cannon MK1", 1 )
   jump.setKnown( "Delta Polaris", "Jade" )

   -- Give all GUIs
   -- TODO: Would be better to remove these outfits and the association,
   -- but they're so tightly integrated atm (with no other way to define
   -- GUIs as usable) that I'm implementing it this way for now.
   player.outfitAdd( "GUI - Brushed" )
   player.outfitAdd( "GUI - Slim" )
   player.outfitAdd( "GUI - Slimv2" )
   player.outfitAdd( "GUI - Legacy" )

   -- TODO remove when releasing 0.9.0
   -- This line stops the tutorial updater from running
   var.push( "tut_update", "0.9.0" )

   hook.timer(5.0, "timer_tutorial")
end

function timer_tutorial()
   naev.missionStart( "Tutorial" )
   evt.finish( true )
end
