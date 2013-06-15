--[[

   MISSION: Assault on Unicorn
   DESCRIPTION: Kill some pirates!

--]]

include "numstring.lua"

lang = naev.lang()
if lang == "es" then
else 
   misn_title = "DV: Assault on Unicorn" 
   misn_reward = "Variable"
   misn_desc = "It is time to put a dent in the pirates' forces. We have detected a strong pirate presence in the system of Unicorn. We are offering a small sum for each pirate killed. The maximum we will pay you is %s credits."

   title = {}
   text = {}
   title[2] = "Mission accomplished"
   text[2] = "As you land, you see a Dvaered military official approaching. Thanking you for your hard and diligent work, he hands you the bounty you've earned, a number of chips worth %s credits."

   osd_msg = {}
   osd_msg[1] = "Fly to the Unicorn system."
   osd_msg[2] = ""
   osd_msg2 = "Destroy some pirates! You have killed %d and have earned %s credits. If finished, return to %s."
   osd_msg3 = "You have reached your maximum payment. Return to %s."

end

function create ()
   rep = faction.playerStanding("Dvaered")
   -- Round the payment to the nearest thousand.
   max_payment = math.floor( (10000 * math.sqrt(rep) * rep/10 / 1e3) + .5 ) * 1e3
   misn_desc = misn_desc:format( numstring(max_payment) )
   misn.setTitle(misn_title)
   misn.setReward(misn_reward)
   misn.setDesc(misn_desc)

   misn_target_sys = system.get("Unicorn")
   misn_return_sys = system.get("Amaroq")
   marker = misn.markerAdd( misn_target_sys, "computer" )
   marker2 = misn.markerAdd( misn_return_sys, "low" )
end

function accept ()
   -- This mission makes no system claims.
   if misn.accept() then
      pirates_killed = 0
      -- Makes sure only one copy of the mission can run.
      var.push( "assault_on_unicorn_check", true)
      planet_start = planet.cur()
      planet_start_name = tostring(planet_start)
      pirates_killed = 0
      bounty_earned = 0
      misn_stage = 0
      misn_target_sys = system.get("Unicorn")

      osd_msg[2] = osd_msg2:format(pirates_killed, numstring(bounty_earned), planet_start_name)
      misn.osdCreate(misn_title,osd_msg)
      misn.osdActive(1)

      hook.enter("jumpin")
      hook.land("land")
   end
end

function jumpin()
   if system.cur() == misn_target_sys then
      misn.osdActive(2)
      hook.pilot(nil, "death", "death")
   end
end

function death(pilot,killer)
   if pilot:faction() == faction.get("Pirate") and killer == pilot.player() then
      reward_table = {
         ["Hyena"]             =  1000,
         ["Pirate Shark"]      =  2000,
         ["Pirate Vendetta"]   =  4000,
         ["Pirate Ancestor"]   =  5000,
         ["Pirate Rhino"]      = 10000,
         ["Pirate Phalanx"]    = 11000,
         ["Pirate Admonisher"] = 12000,
         ["Pirate Kestrel"]    = 24000
      }

      killed_ship = pilot:ship():name()
      reward_earned = reward_table[killed_ship]
      pirates_killed = pirates_killed + 1
      bounty_earned = math.min( max_payment, bounty_earned + reward_earned )
      if bounty_earned == max_payment then
         osd_msg[2] = osd_msg3:format(planet_start_name)
      else
         osd_msg[2] = osd_msg2:format(pirates_killed, numstring( bounty_earned ), planet_start_name)
      end
      misn.osdCreate(misn_title, osd_msg)
      misn.osdActive(2)
   end
end

function land()
   if planet.cur() == planet_start and pirates_killed > 0 then
      if var.peek("assault_on_unicorn_check") then
         var.pop( "assault_on_unicorn_check" )
      end

      tk.msg(title[2], text[2]:format( numstring( bounty_earned )))
      player.pay(bounty_earned)
      faction.modPlayerSingle( "Dvaered", math.pow( bounty_earned, 0.5 ) / 100 )
      misn.finish(true)
   end
end

function abort ()
   var.pop( "assault_on_unicorn_check" )
end
