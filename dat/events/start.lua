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
   }
   return names[rnd.rnd(1,#names)]
end


function create()
   player.pilot():rename( name() ) -- Assign a random name to the player's ship.
   player.pilot():addOutfit( "Laser Cannon MK1", 2 )
   jump.setKnown( "Hakoi", "Eneguoz" )

   -- Give all GUIs
   -- XXX: Would be better to remove these outfits and the association,
   -- but they're so tightly integrated atm (with no other way to define
   -- GUIs as usable) that I'm implementing it this way for now.
   player.addOutfit( "GUI - Brushed" )
   player.addOutfit( "GUI - Slim" )
   player.addOutfit( "GUI - Slimv2" )
   player.addOutfit( "GUI - Legacy" )

   hook.timer(1000, "timer_tutorial")
end

function timer_tutorial()
   naev.missionStart( "Tutorial" )
   evt.finish( true )
end
