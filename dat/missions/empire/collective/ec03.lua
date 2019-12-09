--[[

   Collective Distraction

   Author: bobbens
      minor edits by Infiltrator

   Fourth mission in the collective mini campaign.

   You must distract the collective forces by breaching into their systems
    while a commando lands to monitor wireless.

]]--

include "numstring.lua"

bar_desc = _("You see Lt. Commander Dimitri at the bar as usual.")
misn_title = _("Collective Distraction")
misn_reward = _("%s credits")
misn_desc = {}
misn_desc[1] = _("Go to draw the Collective's attention in the %s system.")
misn_desc[2] = _("Travel back to %s in %s.")
title = {}
title[1] = _("Collective Espionage")
title[2] = _("Mission Accomplished")
text = {}
text[1] = _([[As you approach Lt. Commander Dimitri you notice he seems somewhat excited.
    "It looks like you got something. It's not very clear because of %s's atmosphere creating a lot of noise, but it does seem to be similar to Empire transmissions. We've got another plan to try for a cleaner signal. It'll be uglier then the last one. You in?"]])
text[2] = _([["Here's the plan: we want to drop a commando team on %s to set up more sophisticated surveillance. We've already got a team assembled. Your job will be to provide a distraction."
    "The idea would be to have you fly deep into Collective territory and kick up some trouble. A few dead drones should draw their attention. This is no suicide mission, so you'll have to fly back when things start getting ugly. Meanwhile we'll send a fast convoy with the commandos to %s, to start monitoring."]])
text[3] = _([["If all goes well, the commandos will return here with the results after 10 periods. Then we'll have a definitive answer on the communications issues. We aren't anticipating problems on the return, but we'll have some ships ready just in case they're pursued."
    "Good luck and be careful out there," he adds, before saluting you off onto your mission.]])
text[4] = _([[Your ship touches ground and you once again see the face of Lt. Commander Dimitri.
    "How was the trip? I trust you didn't have too many issues evading the Collective. We won't hear from the commandos until 10 periods from now when they get back, but I believe everything went well."
    "Stay alert. We'll probably need your assistance when they get back. Take the free time as a vacation. I heard the weather on Caladan is pretty nice this time of year, maybe you should visit them. We'll keep in touch."]])

osd_msg = {}
osd_msg[1] = _("Fly to the %s system")
osd_msg[2] = _("")
osd_msg[3] = _("Return to %s")
osd_msg["__save"] = true
osd_msg2 = _("Destroy at least %d drones (%d remaining)")


function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( _("Dimitri"), "empire/unique/dimitri" )
   misn.setDesc( bar_desc )
end


function accept ()

   commando_planet = "Eiroik"
   credits = 1000000

   -- Intro text
   if tk.yesno( title[1], string.format(text[1], commando_planet) )
      then
      misn.accept()

      misn_stage = 0
      dronequota = 5 -- The amount of drones the player must whack to win
      droneleft = dronequota
      misn_base, misn_base_sys = planet.get("Omega Station")
      misn_target_sys = system.get("C-59")
      misn_marker = misn.markerAdd( misn_target_sys, "low" )

      -- Mission details
      misn.setTitle(misn_title)
      misn.setReward( misn_reward:format( numstring( credits ) ) )
      misn.setDesc( string.format(misn_desc[1], misn_target_sys:name() ))

      tk.msg( title[1], string.format(text[2], commando_planet, commando_planet ) )
      tk.msg( title[1], text[3] )
      osd_msg[1] = osd_msg[1]:format(misn_target_sys:name())
      osd_msg[2] = osd_msg2:format(dronequota, droneleft)
      osd_msg[3] = osd_msg[3]:format(misn_base:name())
      misn.osdCreate(misn_title, osd_msg)

      hook.enter("jumpin")
      hook.land("land")
   end
end

-- Handles jumping to target system
function jumpin()
    if misn_stage == 0 and system.cur() == misn_target_sys then
        misn.osdActive(2)
        hook.pilot(nil, "death", "death")
    elseif misn_stage == 0 then
        misn.osdActive(1)
    end
end

function death(pilot)
    if pilot:faction() == faction.get("Collective") and (pilot:ship() == ship.get("Drone") or pilot:ship() == ship.get("Heavy Drone")) and droneleft > 0 then
        droneleft = droneleft - 1
        osd_msg[2] = osd_msg2:format(dronequota, droneleft)
        misn.osdCreate(misn_title, osd_msg)
        misn.osdActive(2)
        if droneleft == 0 then
            misn_stage = 1
            misn.osdActive(3)
            misn.markerMove(misn_marker, misn_base_sys)
        end
    end
end

-- Handles arrival back to base
function land()
   if misn_stage == 1 and planet.cur() == misn_base then
      tk.msg(title[2], text[4])

      -- Store time commando theoretically landed
      var.push( "emp_commando", time.tonumber(time.get() + time.create( 0, 10, 0 )) )

      -- Rewards
      player.pay(credits)
      faction.modPlayerSingle("Empire",5)

      misn.finish(true)
   end
end
