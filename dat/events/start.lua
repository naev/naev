function name()
   local names = {
      "Pathfinder",
      "Death Trap",
      "Little Rascal",
      "Gunboat Diplomat",
      "Attitude Adjuster",
      "Vagabond",
      "Sky Cutter",
      "Blind Jump",
      "Terminal Velocity",
      "Eclipse",
      "Windjammer",
      "Icarus",
      "Heart of Gold", -- Yes, really.
      "Enterprise", -- Doubly so.
      "Voyager", -- Inferior to the former.
      "Serenity", -- I'm on a roll!
      "Century Eagle", -- I've heard George Lucas is pretty litigious.
      "Planet Jumper",
      "Outward Bound",
      "Shove Off",
      "Opportunity",
      "Myrmidon",
      "Fire Hazard", -- 22 names ought to be enough for anyone.
      "Armchair Traveller" -- Needed 23.
   }
   return names[rnd.rnd(1,#names)]
end


function create()
   player.pilot():rename( name() ) -- Assign a random name to the player's ship.
   player.pilot():addOutfit( "Laser Cannon MK1", 2 )
   jump.setKnown( "Hakoi", "Eneguoz" )
   evt.finish( true )
end
