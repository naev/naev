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
   text[2] = [[Welcome to the NAEV Tutorial.  This Tutorial assumes you are using the default keybindings, if you aren't please use whatever keybindings you switched to instead of the defaults.]]
   -- Step 1
   title[2] = "Flight Tutorial"
   text[3] = [[We'll start off by flying around.  Use the 'left' and 'right' keys to turn and the 'up' key to accelerate.  Try flying around the planet, it's hard at first but you'll get the hang of it eventually.]]
   -- Step 2
   text[4] = [[Good.  Now we'll try braking.  Since you can only accelerate forward, you'll have to turn around and accelerate to brake.  To make this easier you can use the 'down' key to turn around automatically.  Then you accelerate until you stop.  Look at the stars in the background for an indication of your velocity.  Try braking to a complete stop now.]]
   text[5] = [[ Now you're getting the hang of it.  It'll still take a while until you can fly well, but this is the start. ]]
   -- Step 3
   title[3] = "Targeting Tutorial"
   text[6] = [[Now we'll work on targeting.  To target you have multiple options.
   'tab' cycles through ships.
   'ctrl + tab' cycles through ships backwards.
   't' targets nearest ship.
   'r' targets nearest hostile.]]
   text[7] = [[I'll bring in a couple of ships so you can try playing around with targeting.  You'll notice your targeted ship gets highlighted blue on the radar (top-right).  You can use the + and - keys on the keypad to change the zoom on the map.  Yellow is neutral, red is hostile, green is friendly and grey is inert.  The new ships will be yellow.]]
   -- Step 4
   title[4] = "Combat Tutorial"
   text[8] = [[Now let's try using the weapon systems.  There are primarily three keys:
   'space' shoots primary weapons.
   'w' selects secondary weapon.
   'shift' shoots secondary weapon.]]
   text[9] = [[Your ship (Llama) only has a single Laser Cannon as a primary weapon, so we'll try using that.  I'll put in a Dummy Llama so you can try destroying it.  So try targeting it with 'tab' or 't' and then approach it and shoot it with 'space' until it is disabled.  You can also use 'a' to autoface the target you have selected.]]
   -- Step 5
   text[10] = [[Good.  Now that the Llama is disabled you can try to board it.  To board a ship you must target it and then stop directly above it.  Once you're on top of the ship you can hit 'b' which will allow you to board the ship to steal resources.
Try to board the ship now.]]
   -- Back to Step 4
   text[11] = [[You weren't supposed to destroy the Llama!  I'll add another so you can try again.]]
   text[12] = [[Good.  Now if the Llama had money or cargo you could attempt to steal it.  Your probability of success is based on your crew versus their crew.  The more crew you have the higher your chance of success.  Since the ship has nothing just leave it be and we'll introduce real enemies.]]
   -- Step 6
   text[13] = [[Now comes the real challenge.  I'll bring in a real pirate to see how you fare with him.  Don't worry, help is on the way.]]
   text[14] = [[That wasn't so scary was it?  Now we'll learn more about other things like Landing.  Landing allows you to not only regenerate your shield and armour, but to trade goods, purchase outfits for your ship and buy new ships.  The only downside is that you take a while to land, so if you're in a hurry for some mission, it's good to try not to land.]]
   -- Step 7
   title[5] = "Landing Tutorial"
   text[15] = [[Landing is also how you save in NAEV.  Every time you land, it'll save your game.  So don't land if you just screwed up an important mission or pissed off a big faction.  The keys involved with landing are:
   'p' cycles through planets.
   'l' lands or targets the nearest planet if none selected.  Needed to get confirmation.]]
   text[16] = [[The easiest way to land is to hit 'l' to get the first land target and get the landing acknowledgment.  Then you have to "board" the planet, meaning go and brake on top of it.  Then you hit 'l' again to land.  Try doing this now.]]
   text[17] = [[This is the land window.  Here you can see an image of where you are with a description of where you landed.  Depending on the planet you'll have various services available.  Services include:
   * Commodity Exchange
   * Spaceport Bar
   * Mission Computer
   * Outfitting
   * Shipyard]]
   title[6] = "Commodity Exchange"
   text[18] = [[In the Commodity Exchange you can buy or sell goods.  The goods available depend on where you are.  Not every place has everything.]]
   title[7] = "Spaceport Bar"
   text[19] = [[The Spaceport Bar is a great place to find all sorts of esoteric missions.  If you are looking for adventure you should always check the Spaceport Bar as soon as you land.  Don't forget to have some cargo space available as most missions will use it.]]
   title[8] = "Mission Computer"
   text[20] = [[The Mission Computer is a place where you can find many missions available for getting quick money.  Since these missions are created by computer systems they tend to not be too interesting compared to what you can find in the Spaceport Bar.  They're a great way to start exploring the universe though.]]
   title[9] = "Outfitting"
   text[21] = [[The more advanced planets and stations will have Outfitting available.  There you can modify your ship and add all sorts of gadgets, allowing you to increase your ship's efficiency and giving you an edge over the rest.  Not every place has every outfit so it's good to travel all over to see what's available.  It's advisable to always buy maps if they're available.]]
   title[10] = "Shipyard"
   text[22] = [[In the Shipyard you can buy ships.  You can also store the ships you already own and switch between them or sell them.  It's very expensive to transport ships between systems, so it's usually best to go to where the ship is.]]
   text[23] = [[Try exploring a bit around the planet.  When you're ready to leave click on 'Takeoff' and we'll continue the tutorial.]]
   -- Step 8
   title[11] = "Navigation Tutorial"
   text[24] = [[In this final part of the tutorial we'll deal with long-distance navigation.  All ships in NAEV are equipped with a hyperspace drive and universe map.  Important keys to remember:
   'm' opens the system map.
   'h' cycles through hyperspace targets.
   'j' attempts to enter hyperspace or aborts an attempt.
   'ctrl + j' activates autonavigation pilot.]]
   title[12] = "System Map"
   text[25] = [[We'll first talk about the map.  When you open your map you'll notice it's very empty.  That's because you haven't explored much yet.  As you explore stuff or buy star maps it'll expand.  Each circle represents a system, and the lines represent hyperspace routes.  You can click on a system to select it as a hyperspace target.  If it's far away the autonav system will make a route to it which your autonavigation pilot can use.  The colour of each jump indicates whether or not you have enough fuel to make it.]]
   title[13] = "Nav System"
   text[26] = [[Once you have a target you'll notice it in your nav system.  If you are far enough to jump it'll be green, otherwise it'll be grey.  You can't jump near big gravity centers, meaning basically planets and space stations.  Once you get far enough away it'll turn green and you can initialize the jump.]]
   text[27] = [[Now we'll try to jump. Here's an overview of how it works:
   1) Select target with map ('m') or cycle through targets with 'h'.
   2) Get away from gravity wells until navigation turns green or use autopilot 'ctrl + j'.
   3) Use 'j' to initialize the jump.
   
Try doing this now. Since you haven't explored any systems just pick any one and try jumping.  I'd recommend the one on the bottom right if you don't want trouble.]]
   -- Stage 9
   title[14] = "Tutorial Finished"
   text[28] = [[And this concludes the tutorial.  You should now know how to:
   * Fly
   * Target
   * Fight
   * Land
   * Jump

You should start getting better by getting cargo missions at Mission Computers, which will also help you explore the universe.

Enjoy the game!]]
   -- Mission details
   misn_title = "NAEV Tutorial"
   misn_reward = "Knowledge of how to play the game."
   misn_desc = "New Player Tutorial to learn how survive in the universe."
   -- Aborted mission
   msg_abortTitle = "Tutorial Aborted"
   msg_abort = [[You seem to know more than is needed for the tutorial.  Tutorial aborting.]]
   -- OSD stuff
   osd_title = {}
   osd_msg   = {}
   osd_title[1] = "Tutorial - Flight"
   osd_msg[1]   = {
      "Fly around with the arrow keys",
      "Brake turning around and accelerating"
   }
   osd_title[2] = "Tutorial - Targeting"
   osd_msg[2]   = {
      "Target ships that appear with:",
      "\t'tab' cycles through ships.",
      "\t'ctrl + tab' cycles through ships backwards.",
      "\t't' targets nearest ship.",
      "\t'r' targets nearest hostile."
   }
   osd_title[3] = "Tutorial - Combat"
   osd_msg[3]   = {
      "Disable the Llama",
      "Board the Llama",
      "Fight off a Hyena",
      "Combat controls are the following:",
      "\t'space' shoots primary weapons.",
      "\t'w' selects secondary weapon.",
      "\t'shift' shoots secondary weapon.",
      "To target:",
      "\t'tab' cycles through ships.",
      "To autoface:",
      "\t'a' autofaces target.",
      "To board:",
      "\t'b' boards target."
   }
   osd_title[4] = "Tutorial - Landing"
   osd_msg[4]   = {
      "Land on %s",
      "Keys to land:",
      "\t'p' cycles through planets.",
      "\t'l' lands or gets acknowledgement."
   }
   osd_title[5] = "Tutorial - Navigation"
   osd_msg[5]   = {
      "1) Select target with map 'm'",
      "2) Get away from planet.",
      "3) Use 'j' to initialize the jump."
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
      tk.msg( title[1], text[2] )
      tk.msg( title[2], text[3] )
      misn.timerStart( "flightOver", 15000 ) -- 15 second timer to fly around

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
   tk.msg( title[2], text[4] )
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
      tk.msg( title[3], text[6] )
      tk.msg( title[3], text[7] )
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
   tk.msg( title[4], text[8] )
   tk.msg( title[4], text[9] )

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
   tk.msg( title[4], text[10] )
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
   tk.msg( title[5], text[15] )
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
   tk.msg( title[11], text[24] )
   tk.msg( title[12], text[25] )
   tk.msg( title[13], text[26] )
   tk.msg( title[13], text[27] )
end


function tutEnter ()
   enter_sys = system.get()
   if misn_stage ~= 8 then
      tk.msg( msg_abortTitle, msg_abort )
      misn.finish(false)
   elseif enter_sys ~= misn_sys then
      misn_stage = 9
      tk.msg( title[14], text[28] )
      misn.finish(true)
   end
end
