function name()
   local names = {
      _("Pathfinder"),
      _("Death Trap"),
      _("Little Rascal"),
      _("Gunboat Diplomat"),
      _("Attitude Adjuster"),
      _("Vagabond"),
      _("Sky Cutter"),
      _("Blind Jump"),
      _("Terminal Velocity"),
      _("Eclipse"),
      _("Windjammer"),
      _("Icarus"),
      _("Heart of Lead"),
      _("Exitprise"),
      _("Commuter"),
      _("Serendipity"),
      _("Aluminum Mallard"), -- Because we might as well allude to an existing parody. Proper spelling would be _("Aluminium"), by the way.
      _("Titanic MLXVII"),
      _("Planet Jumper"),
      _("Outward Bound"),
      _("Shove Off"),
      _("Opportunity"),
      _("Myrmidon"),
      _("Fire Hazard"), -- 22 names ought to be enough for anyone.
      _("Armchair Traveller") -- Needed 23.
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
