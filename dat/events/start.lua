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

   text[6]  = "\"You can target planets, ships, and jump points by \027bleft-clicking\0270 on them, either on your view or on your minimap. %s targets the nearest hostile ship, which is very useful in the heat of battle.\""

   text[7]  = [["You can fire your primary weapon with %s, and your secondary weapon with %s. Don't forget that for homing missiles, you also have to face your target until you get a lock."]]

   text[8]  = [["Press %s to hail (contact) the target ship. If you are being hailed by another ship, you can press %s to hail them back. You can hail ships to ask them for some fuel, or you can hail pirates that you just can't beat and offer to pay them off."]]

   text[9]  = [["Press %s to board the target ship. You normally need to disable the ship before you can board it."]]

   text[10] = [["Press %s to request permission to land on the target planet, and then press it again to land. Remember, you can't land on all planets! Planets are where you can meet people, read the news, equip your ship, buy new ships, and accept missions."]]

   text[11] = [["To travel from one system to another with a jump gate (marked on your map as a triangle icon), you can press %s while the jump gate is selected. Of course, you have to be on top of the jump gate for this to work."]]

   text[12] = [["Pressing %s will bring up the overlay map. This map shows everything you know about in the current system. You can right-click anywhere on this map to engage Autonav, which will automatically fly you to that location."]]

   text[13] = [["Pressing %s will bring up the star map, where you can look at the systems you know about. From here, you can also use Autonav to easily and quickly travel between systems."]]

   text[14] = [["Pressing %s will bring up the small menu, where you can quit the game or access the options menu."]]

   text[15] = [["And finally, pressing %s will bring up your information screen, where you can view your stats and missions."]]

   text[16] = [["Well, that's pretty much it! There are more controls, but they aren't really essential. You can review them later on in the Options menu, or go through the tutorials from the main menu. Good luck, %s!" And with that, you set off on your journey.]]
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
      tk.msg( title[4], text[4] )
      tk.msg( title[4], text[5]:format( tutGetKey("accel"), tutGetKey("reverse"), tutGetKey("left"), tutGetKey("right"), tutGetKey("mousefly") ) )
      tk.msg( title[4], text[6]:format( tutGetKey("target_hostile") ) )
      tk.msg( title[4], text[7]:format( tutGetKey("primary"), tutGetKey("secondary") ) )
      tk.msg( title[4], text[8]:format( tutGetKey("hail"), tutGetKey("autohail") ) )
      tk.msg( title[4], text[9]:format( tutGetKey("board") ) )
      tk.msg( title[4], text[10]:format( tutGetKey("land") ) )
      tk.msg( title[4], text[11]:format( tutGetKey("jump") ) )
      tk.msg( title[4], text[12]:format( tutGetKey("overlay") ) )
      tk.msg( title[4], text[13]:format( tutGetKey("starmap") ) )
      tk.msg( title[4], text[14]:format( tutGetKey("menu") ) )
      tk.msg( title[4], text[15]:format( tutGetKey("info") ) )
      tk.msg( title[4], text[16]:format( player.name() ) )
   else
      tk.msg( title[3], text[3]:format( player.name() ) )
   end
   evt.finish( true )
end
