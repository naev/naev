include "cargo_common.lua"
include "jumpdist.lua"

--[[

   MISSION: Waste Dump (dat/empire/waste_dump.lua)
   DESCRIPTION: Take toxic waste and dump it over an uninhabited planet. 
      Mission conceived for the Shakar system.
      It would begin at the mission computer on Zembla Shakar
      To change the system, just change the mission.xml entry.
   NOTES: This mission requires balancing to make sure it works with all ship types, possibly normalisation of the fraction of the top speed required for mission success according to the top speed, turn, and acceleration of the player's ship.
--]]

lang = naev.lang()
if lang == "es" then
else

   misn_title = "Emergency atmospheric dump" 
   misn_reward = 10000 -- To be updated when Naev gets an economy.
   misn_desc = "Discreet pilot wanted for in-system shipment. Licensed haulers only."

-- Create the tables
   title = {}
   text = {}

-- Stage one, the player receives the cargo and has a choice
   title["intro"] = "Loading toxic waste"
   text["intro"] = [[A group of men begin loading the drums of toxic waste onto your ship. "Don't worry, the containers are sealed," explains the woman supervising the team, "but there are laws for the disposal of this kind of waste. Unfortunately, we've had a disagreement with our contractor and it's been building up."

She grimaces and rubs the bags under her eyes.

"Please shuttle it over to %s. Empire regulations allow waste containers to be dumped into the atmosphere of a dead planet. The best way to dispose of it is to conduct a high atmosphere burn. Fly in hot, top speed, then spin around and jettison the drums as you decelerate.

"I'll pay you half now. When you return, come find me at the prefecture, and I'll give you the rest."]]

-- num, chosen = tk.choice( title["intro"], text["intro"], "Agreed", "No. Pay me up front." )

-- Stage one, confirmation if the player agrees to terms
   title["agreed"] = [[The Handshake]]
   text["agreed"] = [["See you in a few days, captain."]]

-- Stage one, if the player insists to be paid up front
   title["up_front"] = [[Bargaining]]
   text["up_front"] = [[The woman throws up her hands. "Of course, up front. Everyone want to be paid up front." She glares at you for a moment, then slumps. "Fine, here. Just please do it right. The regulation says a high atmosphere burn. It's important."]]

-- Stage two, aborting the mission before dumping, while landed, and with money unpaid.
   title["abort1"] = [[Unload the cargo]]
   text["abort1"] = [[You locate an isolated part of %s where your toxic cargo will not attract attention. As you unload, one drum of waste slips on a patch of oil. It slides out of control and falls heavily from the ship. When it lands, the impact cracks the casing of the container. A viscous grey liquid oozes out.

All perfectly against regulation, no doubt, but the woman from the prefecture will never know. To rid yourself of this cargo, it is worth forgoing the second half of the modest fee.]]

-- Stage two, aborting the mission before dumping, while landed, paid up front.
   title["abort2"] = [[Unload the cargo]]
   text["abort2"] = [[You locate an isolated part of %s where your toxic cargo will not attract attention. As you unload, one drum of waste slips on a patch of oil. It slides out of control and falls heavily from the ship. When it lands, the impact cracks the casing of the container. A viscous grey liquid oozes out.

All perfectly against regulation, no doubt, but the woman from the prefecture will never know.]]

-- Stage two, completing the mission, with money waiting planetside
   title["burn1"] = [[In a hard, high burn]]
   text["burn1"] = [[The %s's forward sensors capture the image of the drums disintegrating in the high temperatures created by your manoeuvre.

The woman from the prefecture is waiting back on %s to pay you the balance of your fee.]]

-- Stage two, completing the mission after being paid up front
   title["burn2"] = [[In a hard, high burn]]
   text["burn2"] = [[The %s's forward sensors capture the image of the drums disintegrating in the high temperatures created by your manoeuvre.

The woman from the prefecture is waiting back on %s for the report of your success.]]

-- Stage three, return for payment
   title["touch-down"] = [[A windowless office, underground]]
   text["touch-down"] = [[The prefecture operates out of an imposing historical building constructed in early imperial style.

You find your employer, however, in an airless, undecorated office at the end of a long hallway in the second basement. She raises her head when you knock on the door frame.

"All well, captain? You executed it by the book?"]]

-- report = tk.yesno( title["touch-down"], text["touch-down"] )

-- Stage three, get paid
   title["paid"] = [[A windowless office, underground]]
   text["paid"] = [["A reliable star pilot, imagine that. You're one of a rare breed, %s. Our regular service should return soon, but just in case, maybe check back at the mission center next month. Thank you, captain. Fly safe."

As you leave, the woman bows over her desk and with great satisfaction crosses one item off a long list.]]

-- Stage three, don't get paid the whole fee because you say you didn't do the job
   title["unpaid1"] = [[A windowless office, underground]]
   text["unpaid1"] = [[The woman meets your gaze, unblinking, for several heartbeats. Then she shakes her head slowly and bends back over her work. "I don't have room in my budget to pay for services not rendered. Goodbye captain. You know the way out."]]

-- Stage three, upset your employer by telling her you took her money and didn't do the job.
   title["unpaid2"] = [[A windowless office, underground]]
   text["unpaid2"] = [[The woman meets your gaze, unblinking, for several heartbeats. Her hand trembles as she picks up a palm display and bends back over her work. She barely gets the words out from behind clenched teeth. "And I trusted you. The government can't even buy loyalty these days. Not even from a nobody space rat. Waste of public funds. Get out of here, captain. You know the way."]]

-- Stage three, if you abort the job in space, before dumping, with money unpaid.
   title["abort3"] = [[Jettison the cargo]]
   text["abort3"] = [[The drums drift off into space, with no one the wiser. The waste joins many tons of space debris, natural and man-made, drifting through %s.

All perfectly against regulation, no doubt, but the woman from the prefecture will never know. To rid yourself of this cargo, it is worth forgoing the second half of the modest fee.]]

-- Stage three, if you abort the job in space, before dumping, paid up front.
   title["abort4"] = [[Jettison the cargo]]
   text["abort4"] = [[The drums drift off into space, with no one the wiser. The waste joins many tons of space debris, natural and man-made, drifting through %s.

All perfectly against regulation, no doubt, but the woman from the prefecture will never know.]]

-- Stage three, if you abort the job after dumping, with money unpaid.
   title["abort5"] = [[Abort the mission]]
   text["abort5"] = [[The woman from the prefecture may sit memorising regulations, and shuffling budgets as long as she wants. You forego the second half of your fee. As for her, she will have to resign herself to never knowing what became of her toxic sludge.]]

-- Stage three, if you abort the job after dumping, paid up front.
   title["abort6"] = [[Abort the mission]]
   text["abort6"] = [[The woman from the prefecture may sit memorising regulations, and shuffling budgets as long as she wants. She will have to resign herself to never knowing what became of her toxic sludge.]]

   full = {}
   full[1] = "No room in ship"
   full[2] = "You don't have enough cargo space to accept this mission. You need %d tons of free space (you need %d more)."

   osd = {}
   osd["title"] = "Waste disposal"
   osd["directions"] = "Fly to %s and dump the cargo into the atmosphere."
   osd["directions1"] = "Fly to %s and in the %s system and dump the cargo into the atmosphere."
   osd["protip"] = "Fly into %s at top speed. Spin around in atmo and throw on the brakes."
   osd["done1"] = "Return to %s to collect the rest of your payment."
   osd["done2"] = "Return to %s to report on your success."

end


function create ()
     
   misn.setTitle( misn_title)
   misn.setReward( misn_reward)
   misn.setDesc( misn_desc)

   mission_system = system.cur()
   mission_planet = planet.cur()
--   dumping_system = system.cur()
--   dumping_planet = planet.get( "Shakar I")
   cargo = "Waste Containers"
   amount = 50
   player_stats = player.pilot():stats()

--[[
To make this mission more future proof against changes in systems, it
determines the closest uninhabited planet to the starting planet,
whether in the system or another one, and sets it as mission destination.
Yes, this is very much the long way around.
--]]

-- **With Thanks to BTAxis and Deiz.**

   local uninhabited = {}
   for i,s in ipairs( getsysatdistance( system.cur(), 0, 6 )) do
      for i, v in ipairs(s:planets()) do
         local wi, hi = v:gfxSpace():dim()
         -- The set of possible dumping targets is limited to uninhabited planets of radius 190 or above.
         if not v:services()["inhabited"] then -- and wi + hi > 760 --[[and v:system():faction() == mission_planet:faction()--]] then
            uninhabited[#uninhabited + 1] = {v, s}
         end
      end
   end
     
   if #uninhabited == 0 then misn.finish( false) end -- Sanity in case no suitable planets are in range.
     
   local nearest
   for k, v in ipairs(uninhabited) do
--      local wid, hid = v[1]:gfxSpace():dim()
--      print( v[1]:name(), (wid + hid) /4)
      distance = cargo_calculateDistance( mission_system, mission_planet:pos(), v[1]:system(), v[1])
      if not nearest or distance < nearest then
         nearest = distance
         dumping_planet = v[1]
         dumping_system = dumping_planet:system()
      end
   end
   
   w, h = dumping_planet:gfxSpace():dim() -- Find the radius of the planet for aero-braking
   dumping_planet_radius = 1.0 * (w + h) / 4 
--   if dumping_planet_radius < ( 90 / player_stats.turn * player_stats.speed * 1.5) then misn.finish( false) end -- only offer mission if player ship can do it

   misn.markerAdd( dumping_system, "computer" )
     
end


function accept ()

   if player.pilot():cargoFree() < amount then
      tk.msg(full[1], full[2]:format(amount, amount - player.pilot():cargoFree()))
      misn.finish()
   end
   misn.accept()
   num, chosen = tk.choice( title["intro"], string.format( text["intro"], dumping_planet:name()), "Agreed", "No. Pay me up front.")
   if num == 1 then
      tk.msg( title["agreed"], text["agreed"])
      player.pay( misn_reward / 2)
   else
      tk.msg( title["up_front"], text["up_front"])
      player.pay( misn_reward)
      paid = true
   end

   -- How the osd reads will depend on the mission variables and whether the player negotiated.
   if mission_system == dumping_system then
      osd["directions"] = osd["directions"]:format(dumping_planet:name())
   else
      osd_msg["directions"] = osd_msg["directions1"]:format(dumping_planet:name(), dumping_system:name())
   end
   osd["protip"] = osd["protip"]:format( dumping_planet:name())
   osd["done1"] = osd["done1"]:format( mission_planet:name())
   osd["done2"] = osd["done2"]:format( mission_planet:name())
   if paid ~= true then
      misn.osdCreate( osd["title"], { osd["directions"], osd["protip"], osd["done1"]})
   else
      misn.osdCreate( osd["title"], { osd["directions"], osd["protip"], osd["done2"]})
   end
   misn.osdActive( 1)

   misn.cargoAdd( cargo, amount)
--   misn.osdCreate( osd["title"], {osd["directions"]})

   landed = true

   hook.enter( "enterWasteMission", dumping_system)
   hook.land( "landWasteMission")

end

function landWasteMission ()

   landed = true

end

function enterWasteMission ()

   landed = false
   dumpingZone()

end

-- Is the player over the planet, in range to dump?
function dumpingZone ()

   if vec2.dist( player.pilot():pos(), dumping_planet:pos()) <= dumping_planet_radius then
      entry = false
      stats = player.pilot():stats()
      pp_top_speed = 0.95 * stats.speed_max
      hardBurn()
   else
      hook.timer( 500, "dumpingZone")
   end

end

-- The function of the aerobraking / dumping procedure over the destination planet
function hardBurn ()

      misn.osdActive( 2)
   -- Possibilities:
   if ( vec2.dist( player.pilot():pos(), dumping_planet:pos()) > dumping_planet_radius) then
      -- The player can't stay over the planet.
--      print( "Atmospheric exit.") -- Used print statements for testing the conditions.
      entry = false
      dumpingZone()
   elseif entry == true and player.pilot():vel():mod() <= player_stats.speed - 3 then
      -- The player executes the manoeuvre.
      if paid ~= true then
         tk.msg( title["burn1"], string.format( text["burn1"], player.ship(), mission_planet:name()))
         misn.osdActive( 3)
         hook.land( "comeCollect")
      else
         tk.msg( title["burn2"], string.format( text["burn2"], player.ship(), mission_planet:name()))
         misn.osdActive( 3)
         hook.land( "comeCollect")
      end
   elseif player.pilot():vel():mod() >= pp_top_speed then
      -- The player starts the manoeuvre.
--      print( "Atmospheric entry.")
      entry = true
      hook.timer( 25, "hardBurn")
   elseif entry == true then
      -- The player is in the middle of the manoeuvre.
--      print( "...braking...")
      hook.timer( 25, "hardBurn")
   else
      -- But if the player enters with not enough speed, he has to fly away and give it another run up.
--      print( "Not going fast enough.")
      hook.timer( 1000, "dumpingZone")
   end

end

-- After dumping the cargo, the player can return to the planet where the mission started.
function comeCollect ()

   if tk.yesno( title["touch-down"], text["touch-down"] ) then
      tk.msg( title["paid"], string.format( text["paid"], player.name()))
      player.pay( misn_reward / 2)
      misn.finish( true)
   elseif paid ~= true then
      tk.msg( title["unpaid1"], text["unpaid1"])
      misn.finish( true)
   else
      tk.msg( title["unpaid2"], text["unpaid2"])
      misn.finish( true)
   end
end

function abort ()

   if dumped ~= true then -- Pre-dumping the player can enjoy littering...
      if landed == true then  -- ...and get messages suited to when the player is landed....
         if paid ~= true then -- ...when the player has not been paid up front...
            tk.msg( title["abort1"], string.format( text["abort1"], planet.name( planet.cur())))
            misn.finish( true)
         else -- or if the player has...
            tk.msg( title["abort2"], string.format( text["abort2"], planet.name( planet.cur())))
            misn.finish( true)
         end
      else -- when the player is in space...
         if paid ~= true then -- 
            tk.msg( title["abort3"], string.format( text["abort3"], system.name( system.cur())))
            misn.finish( true)
         else -- ...or not...
            tk.msg( title["abort4"], string.format( text["abort4"], system.name( system.cur())))
            misn.finish( true)
         end
      end
   else -- ...even after dumping.
      if paid ~= true then 
         tk.msg( title["abort5"], text["abort5"])
         misn.finish( true)
      else
         tk.msg( title["abort6"], text["abort6"])
         misn.finish( true)
      end
   end

end
