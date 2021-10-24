--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Crimelord">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>3</priority>
   <chance>10</chance>
   <location>Bar</location>
   <cond>system.get("Ogat"):jumpDist() == 4 and player.jumps() &gt;= 4</cond>
  </avail>
  <notes>
   <tier>2</tier>
  </notes>
 </mission>
 --]]
--[[ Test for a chase mission
In this mission, you will be chased by a pirate fleet across several systems.

MISSION: Chase Test
DESCRIPTION: Pirates chase you to Ogat.
]]--

local fmt = require "format"
local fleet = require "fleet"

function create ()
   targetsystem = system.get("Ogat")

   misn.setNPC( _("A detective"), "neutral/unique/hunter.webp", _("A private detective is signalling you to come speak with him.") )
end

function accept ()
   -- Note: this mission does not make any system claims.
   if not tk.yesno( _("Crimelord"), string.format( _([[The private detective greets you and gets right down to business.
   "I have tracked down and collected evidence against a local crime lord," he says. "The evidence is on this data disk. He would love nothing more than to get his hands on this.
   I want you to bring this to my associates in the %s system. While the local authorities have proven corruptible, my associates will ensure that this man ends up in prison, where he belongs. I must warn you, however:
   He is a man of considerable influence. He has many friends, and no doubt will send some of his mercenaries to stop you. You'll need a fast ship to shake them off. My associates will compensate you generously when you reach %s.
   Regrettably, you are not the first pilot I've contacted regarding this matter. Your predecessor was intercepted when he landed en route to %s. The crime lord has many underlings lurking in nearby spaceports -- you must NOT land until you've delivered the data."
   Given the dangers, you're not sure whether the reward will make this worth your while. Do you accept?]]), targetsystem:name(),
         targetsystem:name(), targetsystem:name() ) ) then --if accepted
      misn.finish()
   end

   misn.accept()
   reward = 600e3
   tk.msg( _("Good luck"), _([[After quickly glancing around to make sure nobody's taken a particular interest, the detective presses the data stick into your hand.
   "Be careful out there. I doubt you'll be able to get far without being noticed."]]) )
   misn.setTitle( _("Crimelord") )
   misn.setReward( _("A generous compensation") )
   misn.setDesc( string.format( _("Evade the thugs and deliver the evidence to %s"), targetsystem:name() ) )
   misn.markerAdd( targetsystem, "low" )
   misn.osdCreate(_("Crimelord"), {_("Evade the thugs and deliver the evidence to %s"):format(targetsystem:name())})

   startsystem = system.cur() --needed to make thugs appear random in the first system
   last_system = system.cur() --ignore this one, it's just the initialization of the variable

   hook.enter("enter")
   hook.jumpout("jumpout")
   hook.land("land")

end

function enter ()
   hook.timer(4.0, "spawnBaddies")

   if system.cur() == targetsystem then
      local defenderships = { "Lancelot", "Lancelot", "Admonisher", "Pacifier", "Hawking", "Kestrel" }
      local jumpin = jump.pos(targetsystem, last_system)
      defenders = fleet.add( 1, defenderships, "Associates", jumpin )
      for pilot_number, pilot_object in pairs(defenders) do
         local rn = pilot_object:ship():nameRaw()
         if rn == "Lancelot" then
            pilot_object:rename(_("Associate Lancelot"))
         elseif rn == "Admonisher" then
            pilot_object:rename(_("Associate Admonisher"))
         elseif rn == "Pacifier" then
            pilot_object:rename(_("Associate Pacifier"))
         elseif rn == "Hawking" then
            pilot_object:rename(_("Associate Hawking"))
         elseif rn == "Kestrel" then
            pilot_object:rename(_("Associate Kestrel"))
         end

         pilot_object:setFriendly()
         pilot_object:setPos( pilot_object:pos() +
         vec2.new( rnd.rnd(400, 800) * (rnd.rnd(0,1) - 0.5) * 2,
         rnd.rnd(400, 800) * (rnd.rnd(0,1) - 0.5) * 2))
      end

      capship = defenders[#defenders]
      capship:setInvincible()
      capship:comm(_("We've got your back. Engaging hostiles."), true )
   end
end

function jumpout ()
   last_system = system.cur()
end

function spawnBaddies ()
   if last_system == startsystem then
      ai = "baddie"
   else
      ai = "baddie_norun"
   end

   local sp = nil
   if last_system ~= system.cur() then
      sp = last_system
   end

   local pp = player.pilot()
   thugs = fleet.add( 4, "Admonisher", "Thugs", sp, _("Thug"), {ai=ai} )
   for pilot_number, pilot_object in ipairs(thugs) do
      -- TODO Modern optimized equipping, or at least a manual equip from "naked"
      pilot_object:setHostile(true)
      pilot_object:outfitRm("all")
      pilot_object:outfitAdd("Ripper Cannon")
      pilot_object:outfitAdd("Plasma Blaster MK2", 2)
      pilot_object:outfitAdd("Vulcan Gun", 2)
      pilot_object:outfitAdd("Reactor Class II", 2)
      pilot_object:outfitAdd("Milspec Jammer")
      pilot_object:outfitAdd("Engine Reroute")
      pilot_object:outfitAdd("Shield Capacitor II")
      if system.cur() ~= targetsystem then
         -- TODO more sophisticated way of detecting the player
         if pilot_object:inrange( pp ) then
            pilot_object:control()
            pilot_object:attack( pp )
         end
      else
         thugs_alive = #thugs
         hook.pilot(pilot_object, "exploded", "pilotKilled")
      end
   end
   threats = {
      _("Surrender now and we'll let you live."),
      _("You're dead!"),
      _("You won't make it out alive!"),
      _("Get back here!")
   }
   thugs[1]:comm(threats[rnd.rnd(1,#threats)],true)
end

function pilotKilled ()
   thugs_alive = thugs_alive - 1
   if thugs_alive == 0 then
      capship:hailPlayer()
      hook.pilot(capship[1], "hail", "capHailed")
   end
end

function capHailed ()
   tk.msg( _("Mission Accomplished"), string.format( _("\"Excellent work. This data will ensure an arrest and swift prosecution. You've certainly done your part towards cleaning up the region. As for your compensation, I've had %s transferred to you.\""), fmt.credits( reward ) ) )
   player.pay( reward )
   player.commClose()
   misn.finish(true)
end

function land ()
   tk.msg( _("He told you so..."), _("As you step out of your ship and seal the airlock, you spot a burly man purposefully heading towards you. You turn to flee, but there are others closing in on your position. Surrounded, and with several laser pistols trained on you, you see no option but to surrender the evidence."))
   misn.finish(false)
end
