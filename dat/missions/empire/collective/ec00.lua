--[[

   Collective Scout

   Author: bobbens
      minor edits by Infiltrator

   Starts the collective mini campaign.

   You must inspect a stray drone.

]]--

include "proximity.lua"
include "numstring.lua"

bar_desc = _("You see an Empire Lt. Commander who seems to be motioning you over to the counter.")
misn_title = _("Collective Scout")
misn_reward = _("%s credits")
misn_desc = {}
misn_desc[1] = _("Find a scout last seen in the %s system.")
misn_desc[2] = _("Travel back to %s in %s.")
title = {}
title[1] = _("Empire Officer")
title[2] = _("Briefing")
title[3] = _("Mission Accomplished")
text = {}
text[1] = _([[You approach the Lt. Commander.
   "Hello %s, we have a reconnaissance mission you might be interested in. Commander Soldner said you'd make a good candidate for the mission. You up for the challenge?"]])
text[2] = _([["I don't think we've met. I'm Lt. Commander Dimitri. If all goes well you'll be reporting to me for the next assignments."
    "You've heard about the Collective right?  From what we know, the Collective seems to be a sort of 'hive' of robots. They're a recent menace; had the timing to arrive more or less when the Incident occured, otherwise they would have been wiped out by the Emperor's Armada without a sweat. They completely wiped out all human life in Eiroik, and the other worlds they hit. We managed to stop them here, in %s, and constructed this base. Since then it's been more or less a stalemate."]])
text[3] = _([["This week Collective activity has increased heavily. We've been trying to contain them, but a scout broke through to the jump point. It was last detected by a patrol in %s, which saw it jumping out to %s. You are to locate the scout and report back to %s in the %s system. It seems like the Collective is planning something and we want to follow their game a little closer."
    "It is of vital importance that you do not engage the drone. Report back as soon as you locate it."]])
text[4] = _([[After landing, you head to the Empire military headquarters and find Lt. Commander Dimitri there.
    "Well it seems like the drone has some strange fixation on %s. We aren't quite sure what to make of it, but intelligence is working on it. Report back to the bar in a bit and we'll see what we can do about the Collective."]])
msg_spotdrone = _("Drone spotted!")
msg_killdrone = _("Mission Failed: You weren't supposed to kill the drone!")

osd_msg = {}
osd_msg[1] = _("Fly to the %s system")
osd_msg[2] = _("Locate the Collective drone, but do not engage.")
osd_msg[3] = _("Report back to %s in the %s system")
osd_msg["__save"] = true 


function create ()
   misn_nearby = system.get("Acheron")
   misn_target = system.get("Merisi")
   misn_base,misn_base_sys = planet.get("Omega Station")

    missys = {misn_target}
    if not misn.claim(missys) then
        abort()
    end  

   misn.setNPC( _("Lt. Commander"), "empire/unique/dimitri" )
   misn.setDesc( bar_desc )
end


function accept ()
   -- Intro text
   if not tk.yesno( title[1], string.format(text[1], player.name()) ) then
      misn.finish()
   end

   -- Accept mission
   misn.accept()

   misn_stage = 0
   misn_marker = misn.markerAdd( misn_target, "low" )
   credits = 500000

   -- Mission details
   misn.setTitle(misn_title)
   misn.setReward( misn_reward:format( numstring( credits ) ) )
   misn.setDesc( string.format(misn_desc[1],misn_nearby:name()))

   -- Flavour text and mini-briefing
   tk.msg( title[2], string.format( text[2], misn_base_sys:name() ) )
   tk.msg( title[2], string.format( text[3], misn_nearby:name(),
         misn_target:name(), misn_base:name(), misn_base_sys:name() ))

   osd_msg[1] = osd_msg[1]:format(misn_target:name())
   osd_msg[3] = osd_msg[3]:format(misn_base:name(), misn_base_sys:name())
   misn.osdCreate(misn_title, osd_msg)
   hook.enter("enter")
   hook.land("land")
end


function enter()
   sys = system.cur()

   if sys == misn_target and misn_stage == 0 then
      pilot.clear()
      pilot.toggleSpawn(false)
      misn.osdActive(2)
      hook.timer(500, "proximity", {location = vec2.new(8000, -20000), radius = 5000, funcname = "spotdrone"}) 
   elseif misn_stage == 0 then
      misn.osdActive(1)
   end
end

function spotdrone()
   p = pilot.add("Collective Drone", "scout", vec2.new(8000, -20000))[1]
   p:control()
   p:setHilight(true)
   idle()
   
   hook.pilot( p, "death", "kill")
   hook.pilot( p, "idle", "idle")

   -- update mission
   misn.osdActive(3)
   player.msg(msg_spotdrone)
   misn.setDesc( string.format(misn_desc[2],misn_base:name(),misn_base_sys:name()) )
   misn_stage = 1
   misn.markerMove( misn_marker, misn_base_sys )
end

function land()
   pnt = planet.cur()

   if misn_stage == 1 and  pnt == misn_base then
      tk.msg( title[3], string.format(text[4],misn_target:name()) )
      faction.modPlayerSingle("Empire",5)
      player.pay(credits)
      misn.finish(true)
   end
end

function idle()
    if p:exists() then
        local location = p:pos()
        local dist = 750
        local angle = rnd.rnd() * 2 * math.pi
        local newlocation = vec2.new(dist * math.cos(angle), dist * math.sin(angle)) -- New location is 750px away in a random direction
        p:taskClear()
        p:goto(location + newlocation, false, false)
        hook.timer(5000, "idle")
    end
end

function kill()
   player.msg( msg_killdrone )
   misn.finish(false)
   var.push( "collective_fail", true )
end

function abort()
   misn.finish(false)
end
