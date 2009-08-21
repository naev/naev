--[[

   The beginner player tutorial.

   Does simple stuff like teach the player to fly around or use his communications system.

   Step 1
      Basic flight.

   Step 2
      Braking.

   Step 3
      Targeting.

   Step 4
      Dummy combat.

   Step 5
      Board dummy.

   Step 6
      Real pirate.

   Step 7
      Landing.

   Step 8
      Hyperspace.

   Step 9
      Finished.

]]--

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english

   title = {}
   text = {}
   title[1] = "Tutorial"
   text[1] = "Would you like to run the Tutorial to learn how to play NAEV?"
   text[2] = [[Welcome to the NAEV Tutorial, %s.]]
   -- Step 1
   title[2] = "Flight Tutorial"
   text[3] = [[We'll start off by flying around. Use %s and %s to turn, and %s to accelerate. Try flying around the planet.]]
   -- Step 2
   text[4] = [[Now we'll try braking. Since you can only accelerate forward, you'll have to turn around and accelerate to brake. To make this easier you can use %s to turn around automatically. The stars in the background are a good indication of your velocity. Try braking to a stop now.]]
   text[5] = [[Now you're getting the hang of it.]]
   -- Step 3
   title[3] = "Targeting Tutorial"
   text[6] = [[Now we'll work on targeting. You have multiple options.
   %s cycles through ships in the system.
   %s cycles through ships backwards.
   %s targets the nearest ship.
   %s targets the nearest enemy.]]
   text[7] = [[I'll bring in a couple of ships so you can try targeting. You'll notice your targeted ship is highlighted on the radar (Top right). You can use the %s and %s keys to change the scale of the radar. Yellow is neutral, red is hostile, green is friendly and grey is disabled. The new ships will be yellow.]]
   -- Step 4
   title[4] = "Combat Tutorial"
   text[8] = [[Now, let's try using the weapon systems. There are primarily three keys:
   %s fires primary weapons.
   %s selects secondary weapon.
   %s fires secondary weapon.]]
   text[9] = [[Your ship, a Llama, has two Laser Cannons as its primary weapons, so we'll try using them. I'll put in a Llama so you can try disabling it. Target it with %s or %s, and then approach it and shoot it with %s until it is disabled. You can also use %s to auto-face the target you have selected.]]
   -- Step 5
   text[10] = [[Good. Now that the Llama is disabled you can board it. To board a ship you must target it and then stop directly above it. Once you're on top of the ship you can press %s which will allow you to board the ship to steal resources.
Try to board the ship now.]]
   -- Back to Step 4
   text[11] = [[You weren't supposed to destroy the Llama!  I'll add another so you can try again.]]
   text[12] = [[Good. If the Llama had money or cargo, you could attempt to steal it. Your probability of success is based on the size of your crew versus theirs. The larger your crew, the higher your odds of success. Since the ship has nothing just leave it be and we'll introduce real enemies.]]
   -- Step 6
   text[13] = [[Now comes the real challenge. I'll bring in a real pirate to see how you fare with him. Don't worry, help is on the way.]]
   text[14] = [[That wasn't so scary was it?  Now we'll learn more about other things like Landing. Landing allows you to not only regenerate your shield and armour, but to trade goods, purchase outfits for your ship and buy new ships. The only downside is that you take a while to land, so if you're in a hurry for some mission, it's good to try not to land.]]
   -- Step 7
   title[5] = "Landing Tutorial"
   text[15] = [[Landing is also how you save in NAEV. Every time you land, it'll save your game. So don't land if you just screwed up an important mission or pissed off a big faction. The keys involved with landing are:
   %s cycles through planets.f
   %s lands or targets the nearest planet if none selected. 
   The first press acquires landing clearance, the second lands the ship.]]
   text[16] = [[In order to land, you need to come to a near-stop above the planet or station you're trying to land on, much like boarding a ship. 
   
   Try landing now.]]
   text[17] = [[This is the land window. Here, you can see an image of where you are, along with a description. Depending on the location, you'll have various services available. Services include:
   * Commodity Exchange
   * Spaceport Bar
   * Mission Computer
   * Outfitting
   * Shipyard]]
   title[6] = "Commodity Exchange"
   text[18] = [[In the Commodity Exchange you can buy or sell goods. The goods available depend on where you are. Not every place has everything.]]
   title[7] = "Spaceport Bar"
   text[19] = [[The Spaceport Bar is a great place to find all sorts of esoteric missions. If you are looking for adventure you should always check the Spaceport Bar as soon as you land. Don't forget to have some cargo space available as most missions will use it.]]
   title[8] = "Mission Computer"
   text[20] = [[The Mission Computer is a place where you can find many missions available for getting quick money. Since these missions are created by computer systems they tend to not be too interesting compared to what you can find in the Spaceport Bar. They're a great way to start exploring the universe though.]]
   title[9] = "Outfitting"
   text[21] = [[The more advanced planets and stations will have Outfitting available. There you can modify your ship and add all sorts of gadgets, allowing you to increase your ship's efficiency and giving you an edge over the rest. Not every place has every outfit so it's good to travel all over to see what's available. It's advisable to always buy maps if they're available.]]
   title[10] = "Shipyard"
   text[22] = [[In the Shipyard you can buy ships. You can also store the ships you already own and switch between them or sell them. It's very expensive to transport ships between systems, so it's usually best to go to where the ship is.]]
   text[23] = [[Try exploring a bit around the planet. When you're ready to leave click on 'Takeoff' and we'll continue the tutorial.]]
   -- Step 8
   title[11] = "Navigation Tutorial"
   text[24] = [[In this part of the tutorial we'll deal with long-distance navigation. All ships in NAEV are equipped with a hyperspace drive and universe map. Important keys to remember:
   %s opens the system map.
   %s cycles through hyperspace targets.
   %s attempts to enter hyperspace or aborts an attempt.
   %s activates autopilot.]]
   title[12] = "System Map"
   text[25] = [[We'll first talk about the map. When you open your map you'll notice it's barren. As you explore or buy star maps, the visible area will expand. Each circle represents a system, and the interconnecting lines represent hyperspace routes. You can click on a system to select it as a hyperspace target. If it's far away, the autonav system will make a route to it which your autopilot can use. The colour of each jump indicates whether or not you have enough fuel to make it.]]
   title[13] = "Nav System"
   text[26] = [[Once you have a target you'll notice it in your navigation system. If you are far enough to jump it will be green, otherwise it will be grey. It's impossible to jump near large gravity wells, such as planets and stations. Once you're far enough from any gravity wells, you can jump to another system.]]
   text[27] = [[Now we'll try to jump. Here's an overview of how it works:
   Select a target with the map, or cycle through targets with %s.
   Move away from gravity wells until navigation turns green, or use autopilot with %s.
   Use %s to initialize the jump.
   
Try doing this now. Since you haven't explored any systems, just pick any one and try jumping.]]
   -- Stage 9
   title[14] = "Tutorial Finished"
   text[28] = [[This concludes the tutorial. You should now know how to fly, target, fight, land, and hyperspace between systems.

You can start earning credits by accepting cargo missions, available at mission computers, which will also help you explore the universe.

Enjoy the game!]]
   -- Mission details
   misn_title = "NAEV Tutorial"
   misn_reward = "Knowledge of how to play the game."
   misn_desc = "New Player Tutorial to learn how survive in the universe."
   -- Aborted mission
   msg_abortTitle = "Tutorial Aborted"
   msg_abort = [[You seem to know more than is needed for the tutorial. Tutorial aborting.]]
   -- OSD stuff
   osd_title = {}
   osd_msg   = {}
   osd_title[1] = "Tutorial - Flight"
   osd_msg[1]   = {
      "Basic Movement",
      "Braking"
   }
   osd_title[2] = "Tutorial - Targeting"
   osd_msg[2]   = {
      "Target ships that appear with:",
      naev.getKey("target_next") .. " cycles through ships.",
      naev.getKey("target_prev") .. " cycles backwards.",
      naev.getKey("target_nearest") .. " targets nearest ship.",
      naev.getKey("target_hostile") .. " targets nearest hostile."
   }
   
   osd_title[3] = "Tutorial - Combat"
   osd_msg[3]   = {
      "Disable the Llama",
      "Board the Llama",
      "Fight off a Hyena",
   }
   osd_title[4] = "Tutorial - Landing"
   osd_msg[4]   = {
      "Land on %s",
      "Keys to land:",
      naev.getKey("target_planet") .. " cycles through planets.",
      naev.getKey("land") .. " lands or gets acknowledgement."
   }
   osd_title[5] = "Tutorial - Navigation"
   osd_msg[5]   = {
      "Open the map with " .. naev.getKey("starmap") .. " and select a target.",
      "Move away from the planet.",
      "Use " .. naev.getKey("jump") .. " to initialize the jump."
   }
end

      
function create ()

   if tk.yesno( title[1], text[1] )
      then
      misn.accept()

      -- Clear area of enemies.
      pilot.clear()
      pilot.toggleSpawn(false)

      -- Set basic mission information.
      misn.setTitle( misn_title )
      misn.setReward( misn_reward )
      misn.setDesc( misn_desc )

      -- Create OSD
      misn.osdCreate( osd_title[1], osd_msg[1] )

      -- Give indications on how to fly.
      misn_stage = 1
      tk.msg( title[1], string.format(text[2], player.name()))
	  tk.msg(title[2], string.format(text[3], naev.getKey("left"), naev.getKey("right"), naev.getKey("accel")))
      misn.timerStart( "flightOver", 10000 ) -- 10 second timer to fly around

      -- Set Hooks
      hook.land( "tutLand" )
      hook.takeoff( "tutTakeoff" )
      hook.enter( "tutEnter" )
   end
end


function flightOver ()
   -- Update OSD
   misn.osdActive( 1 )

   -- Update mission stuff
   misn_stage = 2
   tk.msg( title[2], string.format(text[4], naev.getKey("reverse")))
   misn.timerStart( "brakeOver", 1000 )
end


function brakeOver ()
   player = pilot.player()

   -- Check if player successfully braked
   if player:vel():mod() < 10 then

      -- New OSD
      misn.osdCreate( osd_title[2], osd_msg[2] )

      -- Text and mission stuff
      misn_stage = 3
      tk.msg( title[2], text[5] )
      tk.msg( title[3], string.format(text[6], naev.getKey("target_next"), naev.getKey("target_prev"), naev.getKey("target_nearest"), naev.getKey("target_hostile")))
      tk.msg( title[3], string.format(text[7], naev.getKey("mapzoomin"), naev.getKey("mapzoomout")))
      pilots = pilot.add( "Sml Trader Convoy", "dummy" )
      for k,v in ipairs(pilots) do
         v:setFaction("Dummy")
         v:rename("Dummy")
      end
      misn.timerStart( "targetOver", 20000 ) -- 20 seconds to target
   else
      -- Keep on trying until he braked
      misn.timerStart( "brakeOver", 1000 )
   end
end


function targetOver ()
   misn_stage = 4

   -- New OSD
   misn.osdCreate( osd_title[3], osd_msg[3] )

   -- Tell about combat.
   tk.msg( title[4], string.format(text[8], naev.getKey("primary"), naev.getKey("secondary_next"), naev.getKey("secondary")))
   tk.msg( title[4], string.format(text[9], naev.getKey("target_nearest"), naev.getKey("target_next"), naev.getKey("primary"), naev.getKey("face")))

   -- Clear pilots again.
   pilot.clear()

   addLlamaDummy()
end


function addLlamaDummy ()
   -- Add the combat dummy.
   pilots = pilot.add( "Trader Llama", "dummy" )
   for k,v in ipairs(pilots) do
      v:setFaction("Dummy")
      v:rename("Dummy")
      hook.pilot( v, "disable", "llamaDisabled" )
      hook.pilot( v, "death", "llamaDead" )
      hook.pilot( v, "board", "llamaBoard" )
   end
end


function llamaDisabled ()
   -- Update OSD
   misn.osdActive( 1 )

   misn_stage = 5
   tk.msg( title[4], string.format(text[10], naev.getKey("board")))
end


function llamaDead ()
   if misn_stage < 6 then
      -- New OSD
      misn.osdCreate( osd_title[3], osd_msg[3] )

      misn_stage = 4
      tk.msg( title[4], text[11] )
      addLlamaDummy()
   end
end


function llamaBoard ()
   -- Update OSD
   misn.osdActive( 2 )

   misn_stage = 6
   tk.msg( title[4], text[12] )
   misn.timerStart( "boardOver", 3000 )
end


function boardOver ()
   tk.msg( title[4], text[13] )
   pilot.clear() -- Get rid of disabled llama
   pilots = pilot.add( "Pirate Hyena" )
   for k,v in ipairs(pilots) do
       v:rmOutfit( "Laser Cannon", 1 ) -- Make it weaker
       hook.pilot( v, "death", "hyenaDead" )
       hook.pilot( v, "jump", "hyenaDead" ) -- Treat jump as dead
   end
   misn.timerStart( "bringHelp", 5000 ) -- Player "should" surive 5 seconds
end


function bringHelp ()
   pilot.add( "Empire Lancelot" )
   pilot.add( "Empire Lancelot" ) -- Lancelot crushes Hyena
end


function hyenaDead ()
   misn_stage = 7

   -- Create OSD
   osd_msg[4][1] = string.format( osd_msg[4][1], system.get():planets()[1]:name() )
   misn.osdCreate( osd_title[4], osd_msg[4] )

   -- Messages
   tk.msg( title[4], text[14] )
   tk.msg( title[5], string.format(text[15], naev.getKey("target_planet"), naev.getKey("land")))
   tk.msg( title[5], text[16] )
end


function tutLand ()
   -- Shouldn't be landing yet.
   if misn_stage ~= 7 then
      tk.msg( msg_abortTitle, msg_abort )
      misn.finish(false)
   else
      misn_stage = 8
      tk.msg( title[5], text[17] )
      tk.msg( title[6], text[18] )
      tk.msg( title[7], text[19] )
      tk.msg( title[8], text[20] )
      tk.msg( title[9], text[21] )
      tk.msg( title[10], text[22] )
      tk.msg( title[5], text[23] )
   end
end


function tutTakeoff ()
   -- Create OSD
   misn.osdCreate( osd_title[5], osd_msg[5] )

   misn_stage = 8
   misn_sys = system.get()
   tk.msg( title[11], string.format(text[24], naev.getKey("starmap"), naev.getKey("thyperspace"), naev.getKey("jump"), naev.getKey("autonav")))
   tk.msg( title[12], text[25] )
   tk.msg( title[13], text[26] )
   tk.msg( title[13], string.format(text[27], naev.getKey("thyperspace"), naev.getKey("autonav"), naev.getKey("jump")))
end


function tutEnter ()
   enter_sys = system.get()
   if misn_stage ~= 8 then
      tk.msg( msg_abortTitle, msg_abort )
      misn.finish(false)
   elseif enter_sys ~= misn_sys then
	     misn.timerStart( "tutEnd", 5000 )
	  end
   end
   
function tutEnd ()
	misn_stage = 9
	tk.msg( title[14], text [28] )
	misn.finish(true)
   
end
