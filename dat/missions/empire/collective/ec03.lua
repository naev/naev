--[[

   Collective Espionage III

   Author: bobbens
      minor edits by Infiltrator

   Fourth mission in the collective mini campaign.

   You must distract the collective forces by breaching into their systems
    while a commando lands to monitor wireless.

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   bar_desc = "You see Lt. Commander Dimitri at the bar as usual."
   misn_title = "Collective Espionage"
   misn_reward = "None"
   misn_desc = {}
   misn_desc[1] = "Go to draw the Collective's attention in the %s system."
   misn_desc[2] = "Travel back to %s in %s."
   title = {}
   title[1] = "Collective Espionage"
   title[2] = "Mission Accomplished"
   text = {}
   text[1] = [[As you approach Lt. Commander Dimitri you notice he seems somewhat excited.
"It looks like you got something. It's not very clear because of %s's atmosphere creating a lot of noise, but it does seem to be similar to Empire transmissions. We've got another plan to try for a cleaner signal. It'll be uglier then the last one. You in?"]]
   text[2] = [["Here's the plan: we want to drop a commando team on %s to set up more sophisticated surveillance. We've already got a team assembled. Your job will be to provide a distraction."
"The idea would be to have you fly deep into Collective territory and start causing disturbances. This is no suicide mission, so you'll have to fly back when things start getting ugly. Meanwhile we'll send a fast convoy with the commandos to %s, to start monitoring."]]
   text[3] = [["If all goes well, the commandos will return here with the results after 10 STU. Then we'll have a definitive answer on the communications issues. We aren't anticipating problems on the return, but we'll have some ships ready just in case they're pursued."
"Good luck and be careful out there," he adds, before saluting you off onto your mission.]]
   text[4] = [[Your ship touches ground and you once again see the face of Lt. Commander Dimitri.
"How was the trip? I trust you didn't have too many issues evading the Collective. We won't hear from the commandos until 10 STU from now when they get back, but I believe everything went well."
"Stay alert. We'll probably need your assistance when they get back. Take the free time as a vacation. I heard the weather on Caladan is pretty nice this time of year, maybe you should visit them. We'll keep in touch."]]
end


function create ()
   misn.setNPC( "Dimitri", "dimitri" )
   misn.setDesc( bar_desc )
end


function accept ()

   commando_planet = "Eiroik"

   -- Intro text
   if tk.yesno( title[1], string.format(text[1], commando_planet) )
      then
      misn.accept()

      misn_stage = 0      
      misn_base, misn_base_sys = planet.get("Omega Station")
      misn_target_sys = system.get("C-28")
      misn.setMarker(misn_target_sys)

      -- Mission details
      misn.setTitle(misn_title)
      misn.setReward( misn_reward )
      misn.setDesc( string.format(misn_desc[1], misn_target_sys:name() ))

      tk.msg( title[1], string.format(text[2], commando_planet, commando_planet ) )
      tk.msg( title[1], text[3] )

      hook.enter("jump")
   end
end

-- Handles jumping to target system
function jump()
   local sys = system.get()
   local factions = sys:faction()

   -- First mission part is landing on the planet
   if misn_stage == 0 and sys == misn_target_sys then

      -- Maybe introducing a delay here would be interesting.
      misn_stage = 1
      misn.setDesc( string.format(misn_desc[2], misn_base:name(), misn_base_sys:name() ))
      misn.setMarker(misn_base_sys)
      hook.land("land")

   -- Create some opposition
   elseif misn_stage == 1 and (sys:name() == "Hades" or factions["Collective"]) then
      pilot.add("Collective Sml Swarm")
   end
end

-- Handles arrival back to base
function land()
   pnt = planet.get()

   if misn_stage == 1 and pnt == misn_base then
      tk.msg( title[2], text[4] )

      -- Store time commando theoretically landed
      var.push( "emp_commando", time.get() + time.units(10) )

      -- Rewards
      player.modFaction("Empire",5)

      misn.finish(true)
   end
end
