--[[

Za'lek Cargo Run. adapted from Drunkard Mission
]]--

include "dat/scripts/numstring.lua"

-- Bar Description
bar_desc = "You see a drunkard at the bar mumbling about how he was so close to getting his break."

-- Mission Details
misn_title = "Za'lek Shipping Delivery"
misn_reward = "A handsome payment."
misn_desc = "You agreed to help a Za'lek scientist pick up some cargo way out in the sticks. Hopefully this'll be worth it."

-- OSD
OSDtitle = "Za'lek Cargo Monkey"
OSDdesc = {}
OSDdesc[1] = "Go pick up some equipment at %s in the %s system."
OSDdesc[2] = "Drop off the equipment at %s in the %s system."

payment = 250000

-- Cargo Details
cargo = "Equipment"
cargoAmount = 20

title = {}  --stage titles
text = {}   --mission text

title[1] = "Spaceport Bar"
text[1] = [[    You walk into the bar and see a scientist wearing a Za'lek badge looking for someone. Unusual, as Za'lek intellectuals rarely leave their own space.
After a while of listening to him go around, you surmise that he is looking for someone to pick up and drop off some cargo for him. Eventually he sees you comes up politely. "Excuse me sir, but do you happen to know a ship captain who can help me with something?"]]

title[2] = "A long haul through tough territory"
text[2] = [[    You say that he is looking at one right now. Rather surprisingly, the pompous-looking man laughs slightly, looking relieved. "Thank you, captain! I was hoping that you could help me. My name is Dr. Andrej Logan, by the way. I warn you, it'll be simple, but rather dangerous, but I will pay you handsomely for your services.
    I need a load of equipment that is stuck at %s in the %s system. Unfortunately, that is going through the pirate-infested fringe space between the Empire, Za'lek and Dvaered. Seeing as no one can agree whose responsibility it is to clean the vermin out, the pirates have made that route dangerous to traverse. I have had a devil of a time finding someone willing to take the mission on. Once you get the equipment, please deliver it to %s in the %s system. I will meet you there.]]

title[3] = "Deliver the Equipment"
text[3] = [[    Once planetside, you find the acceptance area and type in the code Dr. Logan gave you to pick up the equipment. Swiftly, a group of droids backed by a human overseer loads the heavy reinforced crates on your ship and you are ready to go.]]

title[4] = "Success"
text[4] = [[    You land on the Za'lek planet and find your ship swarmed by dockhands in red and advanced-looking droids rapidly. They unload the equipment and direct you to a rambling edifice for payment.
    When you enter, your head spins at a combination of the highly advanced and esoteric-looking tech on display so casually as well as the utter bedlam of the facility. It takes you a solid ten minutes before someone comes to you asking what you need, looking quite frazzled. You state that you delivered some equipment and are looking for payment. The man types in a wrist-pad for a few seconds and says that the person you are looking for has not landed yet. He gives you a code to act as a beacon so you can catch the shuttle in-bound]]

title[5] = "Takeoff"
text[5] = [[    You feel a little agitated as you leave the atmosphere, but you guess you can't blame the scientist for being late. Especially given the lack of organization you've seen on the planet. Suddenly, a ping sounds on your console, signifying that someone's hailing you!]]

title[6] = "A Small Delay"
text[6] = [[    "Hello again. It's Dr. Logan. I am terribly sorry for the delay, but I got held up. As agreed, you will be paid your fee. I am pleased by your help, captain; I hope we meet again."]]

title[7] = "Bonus"
text[7] = [["For your trouble, I will add a bonus of %d credits to your fee. I am pleased by your help, captain; I hope we meet again."]]

title[8] = "Check Account"
text[8] = [[    You check your account balance as he closes the comm channel to find yourself %s credits richer. A good compensation indeed. You feel better already.]]

title[9] = "No Room"
text[9] = [[You don't have enough cargo space to accept this mission.]]

function create ()
   -- Note: this mission does not make any system claims.

   misn.setNPC( "Za'lek Scientist", "zalek_scientist_placeholder" )  -- creates the scientist at the bar
   misn.setDesc( bar_desc )           -- drunkard's description

   -- Planets
   pickupWorld, pickupSys  = planet.getLandable("Vilati Vilata")
   delivWorld, delivSys    = planet.getLandable("Oberon")
   if pickupWorld == nil or delivWorld == nil then -- Must be landable
      misn.finish(false)
   end
   origWorld, origSys      = planet.cur()

--   origtime = time.get()
end

function accept ()
   if not tk.yesno( title[1], text[1] ) then
      misn.finish()

   elseif player.pilot():cargoFree() < 20 then
      tk.msg( title[9], text[9] )  -- Not enough space
      misn.finish()

   else
      misn.accept()

      -- mission details
      misn.setTitle( misn_title )
      misn.setReward( misn_reward )
      misn.setDesc( misn_desc:format(pickupWorld:name(), pickupSys:name(), delivWorld:name(), delivSys:name() ) )

      -- OSD
      OSDdesc[1] =  OSDdesc[1]:format(pickupWorld:name(), pickupSys:name())
      OSDdesc[2] =  OSDdesc[2]:format(delivWorld:name(), delivSys:name())

      pickedup = false
      droppedoff = false

      marker = misn.markerAdd( pickupSys, "low" )  -- pickup
      misn.osdCreate( OSDtitle, OSDdesc )  -- OSD

      tk.msg( title[2], text[2]:format( pickupWorld:name(), pickupSys:name(), delivWorld:name(), delivSys:name() ) )

      landhook = hook.land ("land")
      flyhook = hook.takeoff ("takeoff")
   end
end

function land ()
   if planet.cur() == pickupWorld and not pickedup then
      if player.pilot():cargoFree() < 20 then
         tk.msg( title[9], text[9] )  -- Not enough space
         misn.finish()

      else

         tk.msg( title[3], text[3] )
         cargoID = misn.cargoAdd(cargo, cargoAmount)  -- adds cargo
         pickedup = true

         misn.markerMove( marker, delivSys )  -- destination

         misn.osdActive(2)  --OSD
      end
   elseif planet.cur() == delivWorld and pickedup and not droppedoff then
      tk.msg( title[4], text[4] )
      misn.cargoRm (cargoID)

      misn.markerRm(marker)
      misn.osdDestroy ()

      droppedoff = true
   end
end

function takeoff()
   if system.cur() == delivSys and droppedoff then

      logan = pilot.add( "Civilian Gawain", "civilian", player.pilot():pos() + vec2.new(-500,-500))[1]
      logan:rename("Dr Logan")
      logan:setFaction("Za'lek")
      logan:setFriendly()
      logan:setInvincible()
      logan:setVisplayer()
      logan:setHilight(true)
      logan:hailPlayer()
      logan:control()
      logan:goto(player.pilot():pos() + vec2.new( 150, 75), true)
      tk.msg( title[5], text[5] )
      hailhook = hook.pilot(logan, "hail", "hail")
   end
end

function hail()
   tk.msg( title[6], text[6] )

--   eventually I'll implement a bonus
--   tk.msg( title[7], text[7]:format( bonus ) )

   hook.timer("1", "closehail")
end

function closehail()
   bonus = 0
   player.pay( payment )
   tk.msg( title[8], text[8]:format( numstring(payment) ) )
   logan:setVisplayer(false)
   logan:setHilight(false)
   logan:setInvincible(false) 
   logan:hyperspace()
   faction.modPlayerSingle("Za'lek",5);
   misn.finish(true)
end

function abort()
   hook.rm(landhook)
   hook.rm(flyhook)
   if hailhook then hook.rm(hailhook) end
   misn.finish()
end
