include "dat/events/tutorial/tutorial-common.lua"

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
   title = {}
   text  = {}

   title[1] = "Flying License Acquired!"
   text[1]  = [["Congratulations, %s," your flying instructor says through the radio. "You have earned your flying license! I must say, you are an excellent pilot, and one of my best students to date. In fact, you can keep that ship as a free gift."]]

   title[2] = "Refresher"
   text[2]  = [["I've got some extra time," says your flight instructor. "Would you like a quick refresher on the flight controls and the universe, just to make sure you've got it all down?"]]

   title[3] = "No, thanks"
   text[3]  = [["Ha, I guess you're eager to start, eh? Well, I won't hold you back, %s. Good luck!" And with that, you set off on your journey.]]

   title[4] = "I'll Take Some Refreshments"
   text[4]  = [["Alright. Let's review the flight controls first: %s accelerates, and %s and %s steer. You can also press %s and your ship will automatically follow the mouse pointer."]]

   text[5]  = [["You can target planets, ships, and jump points by clicking on them, either on your view or on your minimap. %s targets the nearest hostile ship."]]

   text[6]  = [["You can fire your primary weapon with %s, and your secondary weapon with %s. Don't forget that for homing missiles, you also have to face your target until you get a lock."]]

   text[7]  = [["Press %s to hail the target ship. If you are being hailed by another ship, you can press %s to hail them back."]]

   text[8]  = [["Press %s to board the target ship. You normally need to disable the ship before you can board it."]]

   text[9]  = [["Press %s to request permission to land on the target planet, and then press it again to land. Remember, you can't land on all planets! Planets are where you can meet people, read the news, equip your ship, buy new ships, and get missions."]]

   text[10] = [["To use a jump gate (marked on your map as a triangle icon) to go to another system, you can press %s while the jump gate is selected."]]

   text[11] = [["Pressing %s will bring up the overlay map. You can right-click anywhere on this map to engage Autonav and fly to that location."]]

   text[12] = [["Pressing %s will bring up the star map, where you can look at the systems you know about and set Autonav to take you to one of them."]]

   text[13] = [["Pressing %s will bring up the small menu, where you can quit the game or access the options menu."]]

   text[14] = [["And finally, pressing %s will bring up your information screen, where you can view stats and missions, and also adjust how your weapons are controlled."]]

   text[15] = [["Of course, there are more, but they aren't essential right now. You can review those later on in the Options menu. Good luck, %s!" And with that, you set off on your journey.]]
end


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
      "Heart of Lead",
      "Exitprise",
      "Commuter",
      "Serendipity",
      "Aluminum Mallard", -- Because we might as well allude to an existing parody. Proper spelling would be "Aluminium", by the way.
      "Titanic MLXVII",
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

   tk.msg( title[1], text[1]:format( player.name() ) )
   if tk.yesno( title[2], text[2] ) then
      tk.msg( title[4], text[4]:format( tutGetKey("accel"), tutGetKey("left"), tutGetKey("right"), tutGetKey("mousefly") ) )
      tk.msg( title[4], text[5]:format( tutGetKey("target_hostile") ) )
      tk.msg( title[4], text[6]:format( tutGetKey("primary"), tutGetKey("secondary") ) )
      tk.msg( title[4], text[7]:format( tutGetKey("hail"), tutGetKey("autohail") ) )
      tk.msg( title[4], text[8]:format( tutGetKey("board") ) )
      tk.msg( title[4], text[9]:format( tutGetKey("land") ) )
      tk.msg( title[4], text[10]:format( tutGetKey("jump") ) )
      tk.msg( title[4], text[11]:format( tutGetKey("overlay") ) )
      tk.msg( title[4], text[12]:format( tutGetKey("starmap") ) )
      tk.msg( title[4], text[13]:format( tutGetKey("menu") ) )
      tk.msg( title[4], text[14]:format( tutGetKey("info") ) )
      tk.msg( title[4], text[15]:format( player.name() ) )
   else
      tk.msg( title[3], text[3]:format( player.name() ) )
   end
   evt.finish( true )
end
