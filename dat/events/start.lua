include "dat/events/tutorial/tutorial-common.lua"

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
   title = {}
   text  = {}

   title[1] = "Flying License Acquired!"
   text[1]  = [["Congratulations, %s," your flying instructor says through the radio. "You have earned your flying license! I must say, you are an excellent pilot, and one of my best students to date. You can keep that ship as a free gift."]]

   title[2] = "Refresher"
   text[2]  = [["I've got some extra time," says your flight instructor. "Would you like a quick refresher on the flight controls, just to make sure you've got it all down?"]]

   title[3] = "No, thanks"
   text[3]  = [["Ha, I guess you're eager to start, eh? Well, I won't hold you back. Just remember that you can review all of your ship's controls in the Options menu. Good luck!" And with that, you set off on your journey.]]

   title[4] = "I'll Take Some Refreshments"
   text[4]  = [["Alright. Let's review your flight controls, then."]]

   text[5]  = [["Move your ship with %s, %s, %s, and %s. You can also press %s and your ship will automatically follow the mouse pointer. Personally, I like to use the keyboard, but to each his own."]]

   text[6]  = "\"You can target planets, ships, and jump points by \027bleft-clicking\0270 on them on your view, minimap, or overlay map. %s targets the nearest hostile ship, which is very useful in the heat of battle.\""

   text[7]  = "\"You can do whatever action is most appropriate with a planet, ship, or jump point by \027bdouble-clicking\0270 on it or its indicator on your radar.\""

   text[8]  = [["You can fire your primary weapon with %s, and your secondary weapon with %s. Don't forget that some weapons won't work properly unless you target the ship first, and some homing missile weapons require you to face your target until you get a lock."]]

   text[9]  = [["Pressing %s will bring up the overlay map. This map shows everything you know about in the current system. You can right-click anywhere on this map to engage Autonav, which will automatically fly you to that location."]]

   text[10] = [["Pressing %s will bring up the star map, where you can look at the systems you know about. From here, you can also use Autonav to easily and quickly travel between systems."]]

   text[11] = [["Pressing %s will bring up the small menu, where you can quit the game or access the options menu."]]

   text[12] = [["And finally, pressing %s will bring up your information screen, where you can view your stats and missions."]]

   text[13] = [["Well, that's pretty much it! There are more controls, but they aren't really essential. You can review them later on in the Options menu if you want to. If you want a more in-depth overview, you can go through the tutorials from the main menu. Good luck, %s!" And with that, you set off on your journey.]]
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

   hook.timer(1000, "prompt")
end

function prompt()
   tk.msg( title[1], text[1]:format( player.name() ) )
   if tk.yesno( title[2], text[2] ) then
      tk.msg( title[4], text[4] )
      tk.msg( title[4], text[5]:format( tutGetKey("accel"), tutGetKey("reverse"), tutGetKey("left"), tutGetKey("right"), tutGetKey("mousefly") ) )
      tk.msg( title[4], text[6]:format( tutGetKey("target_hostile") ) )
      tk.msg( title[4], text[7] )
      tk.msg( title[4], text[8]:format( tutGetKey("primary"), tutGetKey("secondary") ) )
      tk.msg( title[4], text[9]:format( tutGetKey("overlay") ) )
      tk.msg( title[4], text[10]:format( tutGetKey("starmap") ) )
      tk.msg( title[4], text[11]:format( tutGetKey("menu") ) )
      tk.msg( title[4], text[12]:format( tutGetKey("info") ) )
      tk.msg( title[4], text[13]:format( player.name() ) )
   else
      tk.msg( title[3], text[3]:format( player.name() ) )
   end
   evt.finish( true )
end
