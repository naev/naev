--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Collective Distraction">
 <unique />
 <priority>2</priority>
 <cond>faction.playerStanding("Empire") &gt; 5 and var.peek("collective_fail") ~= true</cond>
 <done>Collective Espionage 2</done>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Omega Enclave</spob>
 <notes>
   <campaign>Collective</campaign>
   <tier>3</tier>
 </notes>
</mission>
 --]]
--[[

   Collective Distraction

   Author: bobbens
      minor edits by Infiltrator

   Fourth mission in the collective mini campaign.

   You must distract the collective forces by breaching into their systems
    while a commando lands to monitor wireless.

]]--
local fmt = require "format"
local emp = require "common.empire"

-- Mission constants
local dronequota = 5 -- The amount of drones the player must whack to win
local misn_base = spob.get("Omega Enclave")
local misn_target_sys = system.get("C-59")


mem.osd_msg = {}

local function setOSD (droneleft)
   local destroy_text, remaining_text
   destroy_text = gettext.ngettext(
         "Destroy at least %d drone",
         "Destroy at least %d drones",
         dronequota ):format( dronequota )
   remaining_text = gettext.ngettext(
         "%d remaining",
         "%d remaining",
         droneleft ):format( droneleft )
   mem.osd_msg[2] = _("%s (%s)"):format(destroy_text, remaining_text)
end


function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( _("Dimitri"), "empire/unique/dimitri.webp", _("You see Lt. Commander Dimitri at the bar, as usual.") )
end


function accept ()
   mem.commando_planet = spob.get("Eiroik")
   mem.credits = emp.rewards.ec03

   -- Intro text
   if tk.yesno( _("Collective Espionage"), fmt.f(_([[As you approach Lt. Commander Dimitri, you notice he seems somewhat excited.
    "It looks like you got something! It's not very clear because of {pnt}'s atmosphere creating a lot of noise, but it does seem to be similar to Empire transmissions. We've got another plan to try for a cleaner signal. It'll be uglier then the last one. You in?"]]), {pnt=mem.commando_planet}) )
      then
      misn.accept()

      mem.misn_stage = 0
      mem.droneleft = dronequota
      mem.misn_marker = misn.markerAdd( misn_target_sys, "low" )

      -- Mission details
      misn.setTitle(_("Collective Distraction"))
      misn.setReward( mem.credits )
      misn.setDesc( fmt.f(_("Go to draw the Collective's attention in the {sys} system"), {sys=misn_target_sys} ))

      tk.msg( _("Collective Espionage"), fmt.f(_([["Here's the plan: we want to drop a commando team on {pnt} to set up more sophisticated surveillance. We've already got a team assembled. Your job will be to provide a distraction.
    "The idea would be to have you fly deep into Collective territory and kick up some trouble. A few dead drones should draw their attention. This is no suicide mission, so you'll have to fly back when things start getting ugly. Meanwhile we'll send a fast convoy with the commandos to {pnt}, to start monitoring."]]), {pnt=mem.commando_planet} ) )
      tk.msg( _("Collective Espionage"), _([["If all goes well, the commandos will return here with the results after 10 periods. Then we'll have a definitive answer on the communications issues. We aren't anticipating problems on the return, but we'll have some ships ready just in case they're pursued.
    "Good luck and be careful out there," he adds, before saluting you off onto your mission.]]) )
      mem.osd_msg[1] = fmt.f(_("Fly to the {sys} system"), {sys=misn_target_sys})
      setOSD(mem.droneleft)
      mem.osd_msg[3] = fmt.f(_("Return to {pnt}"), {pnt=misn_base})
      misn.osdCreate(_("Collective Distraction"), mem.osd_msg)

      hook.enter("jumpin")
      hook.land("land")
   end
end

-- Handles jumping to target system
function jumpin()
    if mem.misn_stage == 0 and system.cur() == misn_target_sys then
        misn.osdActive(2)
        hook.pilot(nil, "death", "death")
    elseif mem.misn_stage == 0 then
        misn.osdActive(1)
    end
end

function death(pilot)
    if pilot:faction() == faction.get("Collective") and (pilot:ship() == ship.get("Drone") or pilot:ship() == ship.get("Heavy Drone")) and mem.droneleft > 0 then
        mem.droneleft = mem.droneleft - 1
        setOSD(mem.droneleft)
        misn.osdCreate(_("Collective Distraction"), mem.osd_msg)
        misn.osdActive(2)
        if mem.droneleft == 0 then
            mem.misn_stage = 1
            misn.osdActive(3)
            misn.markerMove(mem.misn_marker, misn_base)
        end
    end
end

-- Handles arrival back to base
function land()
   if mem.misn_stage == 1 and spob.cur() == misn_base then
      tk.msg(_("Mission Accomplished"), _([[Your ship touches ground and you once again see the face of Lt. Commander Dimitri.
    "How was the trip? I trust you didn't have too many issues evading the Collective. We won't hear from the commandos until 10 periods from now when they get back, but I believe everything went well.
    "Stay alert. We'll probably need your assistance when they get back. Take the free time as a vacation. I heard the weather on Caladan is pretty nice this time of year, maybe you should visit them. We'll keep in touch."]]))

      -- Store time commando theoretically landed
      var.push( "emp_commando", time.tonumber(time.get() + time.new( 0, 10, 0 )) )

      -- Rewards
      player.pay(mem.credits)
      faction.modPlayerSingle("Empire",5)

      emp.addCollectiveLog( _([[You provided a distraction while a commando team was inserted into Eiroik for the Empire to set up more sophisticated surveillance of the Collective. Lt. Commander Dimitri said that they should be back in about 10 periods and that the Empire will probably need your assistance on Omega Enclave again at that time.]]) )

      misn.finish(true)
   end
end
