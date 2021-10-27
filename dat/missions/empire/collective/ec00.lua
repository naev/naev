--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Collective Scouting">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <cond>var.peek("es_cargo") == true and faction.playerStanding("Empire") &gt; 5</cond>
   <chance>40</chance>
   <location>Bar</location>
   <done>Empire Shipping 3</done>
   <planet>Omega Station</planet>
  </avail>
  <notes>
   <campaign>Collective</campaign>
  </notes>
 </mission>
 --]]
--[[

   Collective Scout

   Author: bobbens
      minor edits by Infiltrator

   Starts the collective mini campaign.

   You must inspect a stray drone.

]]--
require "proximity"
local fmt = require "format"
local emp = require "common.empire"

text = {}
text[1] = _([[You approach the Lt. Commander.
   "Hello %s, we have a reconnaissance mission you might be interested in. Commander Soldner said you'd make a good candidate for the mission. You up for the challenge?"]])
text[2] = _([["I don't think we've met. I'm Lt. Commander Dimitri. If all goes well you'll be reporting to me for the next assignments.
    "You've heard about the Collective right?  From what we know, the Collective seems to be a sort of 'hive' of robots. They're a recent menace; had the timing to arrive more or less when the Incident occurred, otherwise they would have been wiped out by the Emperor's Armada without a sweat. They completely wiped out all human life in Eiroik, and the other worlds they hit. We managed to stop them here, in %s, and constructed this base. Since then it's been more or less a stalemate."]])
text[3] = _([["Collective activity has increased heavily the last few decaperiods. We've been trying to contain them, but a scout broke through to the jump point. It was last detected by a patrol in %s, which saw it jumping out to %s. You are to locate the scout and report back to %s in the %s system. It seems like the Collective is planning something and we want to follow their game a little closer.
    "It is of vital importance that you do not engage the drone. Report back as soon as you locate it."]])
text[4] = _([[After landing, you head to the Empire military headquarters and find Lt. Commander Dimitri there.
    "Well it seems like the drone has some strange fixation on %s. We aren't quite sure what to make of it, but intelligence is working on it. Report back to the bar in a bit and we'll see what we can do about the Collective."]])

function create ()
   misn_nearby = system.get("Acheron")
   misn_target = system.get("Merisi")
   misn_base,misn_base_sys = planet.getS("Omega Station")

    missys = {misn_target}
    if not misn.claim(missys) then
        abort()
    end

   misn.setNPC( _("Lt. Commander"), "empire/unique/dimitri.webp", _("You see an Empire Lt. Commander who seems to be motioning you over to the counter.") )
end


function accept ()
   -- Intro text
   if not tk.yesno( _("Empire Officer"), string.format(text[1], player.name()) ) then
      misn.finish()
   end

   -- Accept mission
   misn.accept()

   misn_stage = 0
   misn_marker = misn.markerAdd( misn_target, "low" )
   credits = 500e3

   -- Mission details
   misn.setTitle(_("Collective Scout"))
   misn.setReward( fmt.credits( credits ) )
   misn.setDesc( string.format(_("Find a scout last seen in the %s system"), misn_nearby:name()))

   -- Flavour text and mini-briefing
   tk.msg( _("Briefing"), string.format( text[2], misn_base_sys:name() ) )
   emp.addCollectiveLog( text[2]:format( misn_base_sys:name() ) )
   tk.msg( _("Briefing"), string.format( text[3], misn_nearby:name(),
         misn_target:name(), misn_base:name(), misn_base_sys:name() ))

   misn.osdCreate(_("Collective Scout"), {
      fmt.f(_("Fly to the {sys} system"), {sys=misn_target}),
      _("Locate the Collective drone, but do not engage."),
      fmt.f(_("Report back to {pnt} in the {sys} system"), {pnt=misn_base, sys=misn_base_sys}),
   })
   hook.enter("enter")
   hook.jumpout("jumpout")
   hook.land("land")
end


function enter()
   sys = system.cur()

   if sys == misn_target and misn_stage == 0 then
      -- Force Collective music (note: must clear these later on).
      var.push("music_ambient_force", "Collective")
      var.push("music_combat_force", "Collective")

      pilot.clear()
      pilot.toggleSpawn(false)
      misn.osdActive(2)
      hook.timer(0.5, "proximity", {location = vec2.new(8000, -20000), radius = 5000, funcname = "spotdrone"})
   elseif misn_stage == 0 then
      misn.osdActive(1)
   end
end


function jumpout()
   if system.cur() == misn_target then
      var.pop("music_ambient_force")
      var.pop("music_combat_force")
   end
end


function spotdrone()
   p = pilot.add( "Drone", "Collective", vec2.new(8000, -20000), _("Collective Drone"), {ai="scout"} )
   p:control()
   p:setHilight(true)
   idle()

   hook.pilot( p, "death", "kill")
   hook.pilot( p, "idle", "idle")

   -- update mission
   misn.osdActive(3)
   player.msg(_("Drone spotted!"))
   misn.setDesc( string.format(_("Travel back to %s in %s"), misn_base:name(), misn_base_sys:name()) )
   misn_stage = 1
   misn.markerMove( misn_marker, misn_base_sys )
end

function land()
   pnt = planet.cur()

   if misn_stage == 1 and  pnt == misn_base then
      tk.msg( _("Mission Accomplished"), string.format(text[4], misn_target:name()) )
      faction.modPlayerSingle("Empire",5)
      player.pay(credits)
      emp.addCollectiveLog( _([[You scouted out a Collective drone on behalf of the Empire. Lt. Commander Dimitri told you to report back to the bar on Omega Station for your next mission.]]) )
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
        p:moveto(location + newlocation, false, false)
        hook.timer(5.0, "idle")
    end
end

function kill()
   player.msg( _("Mission Failed: You weren't supposed to kill the drone!") )
   var.push( "collective_fail", true )
   emp.addCollectiveLog( _([[You killed the Collective drone you were supposed to scout out and failed your mission for the Empire as a result.]]) )
   misn.finish(true)
end

function abort()
   if system.cur() == misn_target then
      var.pop("music_ambient_force")
      var.pop("music_combat_force")
   end
   misn.finish(false)
end
