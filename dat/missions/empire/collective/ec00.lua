--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Collective Scouting">
 <unique />
 <priority>2</priority>
 <cond>var.peek("es_cargo") == true and faction.playerStanding("Empire") &gt; 5</cond>
 <chance>40</chance>
 <location>Bar</location>
 <done>Empire Shipping 3</done>
 <spob>Omega Enclave</spob>
 <chapter>[^0]</chapter>
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

-- Mission constants
local misn_nearby = system.get("Acheron")
local misn_target = system.get("Merisi")
local misn_base, misn_base_sys = spob.getS("Omega Enclave")

local p -- Non-persistent state

function create ()
    local missys = {misn_target}
    if not misn.claim(missys) then
        abort()
    end

   misn.setNPC( _("Lt. Commander"), "empire/unique/dimitri.webp", _("You see an Empire Lt. Commander who seems to be motioning you over to the counter.") )
end


function accept ()
   -- Intro text
   if not tk.yesno( _("Empire Officer"), fmt.f(_([[You approach the Lt. Commander.
   "Hello {player}, we have a reconnaissance mission you might be interested in. Commander Soldner said you'd make a good candidate for the mission. You up for the challenge?"]]), {player=player.name()}) ) then
      misn.finish()
   end

   -- Accept mission
   misn.accept()

   mem.misn_stage = 0
   mem.misn_marker = misn.markerAdd( misn_target, "low" )
   mem.credits = emp.rewards.ec00

   -- Mission details
   misn.setTitle(_("Collective Scout"))
   misn.setReward( mem.credits )
   misn.setDesc( fmt.f(_("Find a scout last seen in the {sys} system"), {sys=misn_nearby}))

   -- Flavour text and mini-briefing
   local brief = fmt.f( _([["I don't think we've met. I'm Lt. Commander Dimitri. If all goes well, you'll be reporting to me for the next assignments.
    "You've heard about the Collective, right?  From what we know, the Collective seems to be a sort of 'hive' of robots. They're a recent menace; had the timing to arrive more or less when the Incident occurred, otherwise they would have been wiped out by the Emperor's Armada without a sweat. They completely wiped out all human life in Eiroik and the other worlds they hit. We managed to stop them here, in {sys}, and constructed this base. Since then it's been more or less a stalemate."]]), {sys=misn_base_sys} )
   tk.msg( _("Briefing"), brief )
   emp.addCollectiveLog( brief )
   tk.msg( _("Briefing"), fmt.f( _([["Collective activity has increased heavily the last few decaperiods. We've been trying to contain them, but a scout broke through to the jump point. It was last detected by a patrol in {misn_nearby}, which saw it jumping out to {misn_target}. You are to locate the scout and report back to {pnt} in the {sys} system. It seems like the Collective is planning something and we want to follow their game a little closer.
    "It is of vital importance that you do not engage the drone. Report back as soon as you locate it."]]), {misn_nearby=misn_nearby, misn_target=misn_target, pnt=misn_base, sys=misn_base_sys} ))

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
   local sys = system.cur()

   if sys == misn_target and mem.misn_stage == 0 then
      -- Force Collective music (note: must clear these later on).
      var.push("music_ambient_force", "Collective")
      var.push("music_combat_force", "Collective")

      pilot.clear()
      pilot.toggleSpawn(false)
      misn.osdActive(2)
      hook.timer(0.5, "proximity", {location = vec2.new(8000, -20000), radius = 5000, funcname = "spotdrone"})
   elseif mem.misn_stage == 0 then
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
   local m = p:memory()
   m.comm_no = _("No response.")
   p:control()
   p:setHilight(true)
   idle()

   hook.pilot( p, "death", "kill")
   hook.pilot( p, "idle", "idle")

   -- update mission
   misn.osdActive(3)
   player.msg(_("Drone spotted!"))
   misn.setDesc( fmt.f(_("Travel back to {pnt} in {sys}"), {pnt=misn_base, sys=misn_base_sys}) )
   mem.misn_stage = 1
   misn.markerMove( mem.misn_marker, misn_base )
end

function land()
   local pnt = spob.cur()

   if mem.misn_stage == 1 and  pnt == misn_base then
      tk.msg( _("Mission Accomplished"), fmt.f(_([[After landing, you head to the Empire military headquarters and find Lt. Commander Dimitri there.
    "Well it seems like the drone has some strange fixation on {sys}. We aren't quite sure what to make of it, but intelligence is working on it. Report back to the bar in a bit and we'll see what we can do about the Collective."]]), {sys=misn_target}) )
      faction.modPlayerSingle("Empire",5)
      player.pay(mem.credits)
      emp.addCollectiveLog( _([[You scouted out a Collective drone on behalf of the Empire. Lt. Commander Dimitri told you to report back to the bar on Omega Enclave for your next mission.]]) )
      misn.finish(true)
   end
end

function idle()
    if p:exists() then
        local location = p:pos()
        local newlocation = vec2.newP(750, rnd.angle())
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
