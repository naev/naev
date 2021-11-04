--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="The Last Detail">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>3</priority>
  <done>A Journey To Arandon</done>
  <chance>50</chance>
  <location>Bar</location>
  <planet>Darkshed</planet>
  <cond>not diff.isApplied( "flf_dead" )</cond>
 </avail>
 <notes>
  <campaign>Nexus show their teeth</campaign>
 </notes>
</mission>
--]]
--[[
   This is the climax mission of the Shark's teeth campaign. The player has to kill 4 pirates.

   Stages :
   0) There are pirates to kill
   1) Way to Alteris
--]]
local pir = require "common.pirate"
local pilotname = require "pilotname"
local fmt = require "format"
local shark = require "common.shark"
local lmisn = require "lmisn"

osd_title = _("The Last Detail")

function create ()
   --Will now pick systems between min and max jumps in distance
   local min = 3
   local max = 7
   local systems = lmisn.getSysAtDistance(system.cur(), min, max)
   if #systems == 0 then
      local systems = lmisn.getSysAtDistance(system.cur(), 1, 15)
      if #systems == 0 then
         osd_title = _("Houston, we have a problem!")
         gawsys = system.get("Alteris")
         kersys1 = system.get("Alteris")
         kersys2 = system.get("Alteris")
         godsys = system.get("Alteris")
      end
   end
   systems = rnd.permutation( systems ) -- Avoid picking same system
   gawsys = systems[1]
   kersys1 = systems[2] or systems[ rnd.rnd(1,#systems) ]
   kersys2 = systems[3] or systems[ rnd.rnd(1,#systems) ]
   godsys = systems[4] or systems[ rnd.rnd(1,#systems) ]

   paypla, paysys = planet.getS("Darkshed")

   if not misn.claim({gawsys, kersys1, kersys2, godsys}) then
      misn.finish(false)
   end

   misn.setNPC(_("Arnold Smith"), "neutral/unique/arnoldsmith.webp", _([[Perhaps it would be worthwhile to see if he has another job for you.]]))
end

function accept()

   reward = 3e6
   stage = 0

   --Initialization of dead pirate memory
   gawdead = false
   kerdead1 = false
   kerdead2 = false
   goddead = false

   --set the names of the pirates (and make sure they aren't duplicates)
   gawname = pilotname.pirate()
   kername1 = fmt.f( _("{name} III"), {name=pilotname.pirate()} )
   kername2 = fmt.f( _("{name} Jr." ), {name=pilotname.pirate()} )
   godname = fmt.f( _("{name} II"), {name=pilotname.pirate()} )

   if tk.yesno(_("The mission"), _([["Hello again. As you know, I've agreed with the FLF on a contract that will extend our sales of ships to them substantially. Of course, this deal must remain a secret, which is why it is being done through a false black market dealer.
    "However, we have reason to suspect that a few key influential pirates may have their eyes on the FLF as possible buyers of the Skull and Bones pirate ships. We don't think the FLF will have any interest in those ships, but the pirates' ambitions could give them motivation to attack our false dealer's trade posts, destroying our deal with the FLF anyway. We of course can't have that.
    "So what we want you to do, quite simply, is to eliminate these pirates. It's not that many of them; there are four pirates we need eliminated, and thankfully, they're all spread out. That being said, some of them do have quite big ships, so you will have to make sure you can handle that. Are you willing to do this job for us?"]])) then
      misn.accept()
      tk.msg(_("Very good"), fmt.f(_([["So, here are the details we have gathered about these pirates:
    "{gawname} should be in {gawsys}, flying a Gawain. He seems to be on a holiday, so he probably isn't able to fight back and will just run away.
    "{kername1} should be in {kersys1}, flying a Kestrel. We believe he has some escorts.
    "{kername2} should be in {kersys2}, also flying a Kestrel. He has escorts, too, according to our records.
    "And finally, {godname} is in {godsys}. He stole and beefed up a Goddard recently, so make sure you're prepared for that. He also has escorts, according to our records.
    "And that's about it! Come back for your fee when you have finished."]]), {gawname=gawname, gawsys=gawsys, kername1=kername1, kersys1=kersys1, kername2=kername2, kersys2=kersys2, godname=godname, godsys=godsys}))

      misn.setTitle(_("The Last Detail"))
      misn.setReward(fmt.credits(reward))
      misn.setDesc(_("Nexus Shipyards has tasked you with killing four pirates."))
      misn.osdCreate(osd_title, {
         _("Kill the four pirates"),
         fmt.f(_("Report back to {pnt} in {sys}"), {pnt=paypla, sys=paysys}),
      })
      misn.osdActive(1)

      gawmarker = misn.markerAdd(gawsys, "low")
      kermarker1 = misn.markerAdd(kersys1, "high")
      kermarker2 = misn.markerAdd(kersys2, "high")
      godmarker = misn.markerAdd(godsys, "high")

      hook.enter("enter")
      hook.land("land")

   else
      tk.msg(_("Sorry, not interested"), _([["I'm sorry to hear that. Don't hesitate to come back if you change your mind."]]))
      misn.finish(false)
   end
end

function land ()
   --Job is done
   if stage == 1 and planet.cur() == planet.get("Darkshed") then
      tk.msg(_("That was impressive"), _([[Smith awaits your arrival at the spaceport. When you exit your ship, he smiles and walks up to you. "Good job," he says. "Our deal is secure, thanks to you. Here is your pay and something extra for your hard work. Thank you for all your help!"
    He hands you a credit chip and what appears to be a Nexus Shipyards commemorative sandwich holder.]]))
      pir.reputationNormalMission(rnd.rnd(2,3))
      player.pay(reward)
      player.outfitAdd("Sandwich Holder")
      shark.addLog( _([[You eliminated some pirates that were about to get in the way of Nexus Shipyards' business.]]) )
      misn.finish(true)
   end
end

function enter ()
   --Arrived in system
   if system.cur() == gawsys and gawdead == false then  --The Gawain
      local fenemy = shark.pirateFaction()

      -- Choose a random point in the system for him to stay around
      local sysrad = rnd.rnd() * system.cur():radius()
      local angle = rnd.rnd() * 360
      local pos = vec2.newP(sysrad, angle)

      baddie = pilot.add( "Gawain", fenemy, nil, gawname, {ai="dummy"} )
      baddie:setHostile()
      baddie:setHilight()
      baddie:control()
      baddie:moveto(pos)

      --The pirate becomes nice defensive outfits
      baddie:outfitRm("all")
      baddie:outfitRm("cores")
      baddie:cargoRm( "all" )

      baddie:outfitAdd("Nexus Light Stealth Plating")
      baddie:outfitAdd("Milspec Orion 2301 Core System")
      baddie:outfitAdd("Tricon Zephyr Engine")

      baddie:outfitAdd("Shield Capacitor I")
      baddie:outfitAdd("Plasteel Plating")
      baddie:outfitAdd("Milspec Impacto-Plastic Coating")
      baddie:outfitAdd("Laser Cannon MK1",2)

      baddie:setHealth(100,100)
      baddie:setEnergy(100)

      hook.pilot(baddie, "idle", "idle", pos)
      hook.pilot(baddie, "attacked", "attacked")
      hook.pilot(baddie, "death", "gawain_dead")

   elseif system.cur() == kersys1 and kerdead1 == false then  --The Kestrel
      pilot.clear()
      pilot.toggleSpawn(false)
      local fenemy = shark.pirateFaction()

      baddie = pilot.add( "Pirate Kestrel", fenemy, vec2.new(0,0), nil, {ai="pirate_norun"} )
      local enemies = {
         pilot.add( "Pirate Ancestor", fenemy, vec2.new(100,0) ),
         pilot.add( "Hyena", fenemy, vec2.new(0,100), _("Pirate Hyena") ),
      }
      for k,p in ipairs(enemies) do
         p:setLeader( baddie )
      end

      baddie:rename(kername1)
      baddie:setHilight()
      baddie:setHostile()

      hook.pilot( baddie, "death", "kestrel_dead1")

   elseif system.cur() == kersys2 and kerdead2 == false then  --The Kestrel
      pilot.clear()
      pilot.toggleSpawn(false)
      local fenemy = shark.pirateFaction()

      baddie = pilot.add( "Pirate Kestrel", fenemy, vec2.new(0,0), nil, {ai="pirate_norun"} )
      local enemies = {
         pilot.add( "Pirate Ancestor", fenemy, vec2.new(100,0) ),
         pilot.add( "Pirate Shark", fenemy, vec2.new(0,100) ),
         pilot.add( "Hyena", fenemy, vec2.new(100,100), _("Pirate Hyena") ),
      }
      for k,p in ipairs(enemies) do
         p:setLeader( baddie )
      end


      baddie:rename(kername2)
      baddie:setHilight()
      baddie:setHostile()

      hook.pilot( baddie, "death", "kestrel_dead2")

   elseif system.cur() == godsys and goddead == false then  --The Goddard
      pilot.clear()
      pilot.toggleSpawn(false)
      local fenemy = shark.pirateFaction()

      baddie = pilot.add( "Goddard", fenemy, vec2.new(0,0), godname, {ai="pirate_norun"} ) --Faction's ships come with upgraded weaponry

      local enemies = {
         pilot.add( "Pirate Ancestor", fenemy, vec2.new(100,0) ),
         pilot.add( "Hyena", fenemy, vec2.new(0,100), _("Pirate Hyena") ),
      }
      for k,p in ipairs(enemies) do
         p:setLeader( baddie )
      end

      baddie:setHilight()
      baddie:setHostile()

      hook.pilot( baddie, "death", "goddard_dead")

   end
end

function idle(_pilot, pos)  --the Gawain is flying around a random point
   baddie:moveto(pos + vec2.new( 800,  800), false, false)
   baddie:moveto(pos + vec2.new(-800,  800), false, false)
   baddie:moveto(pos + vec2.new(-800, -800), false, false)
   baddie:moveto(pos + vec2.new( 800, -800), false, false)
end

function attacked()  --the Gawain is going away
   if baddie:exists() then
      baddie:taskClear()
      baddie:runaway(player.pilot(), true)
   end
end

function gawain_dead()
   misn.markerRm(gawmarker)
   gawdead = true
   hook.timer(3.0,"generic_dead")
end

function kestrel_dead1()
   misn.markerRm(kermarker1)
   kerdead1 = true
   hook.timer(3.0,"generic_dead")
end

function kestrel_dead2()
   misn.markerRm(kermarker2)
   kerdead2 = true
   hook.timer(3.0,"generic_dead")
end

function goddard_dead()
   misn.markerRm(godmarker)
   goddead = true
   hook.timer(3.0,"generic_dead")
end

function generic_dead()
   --Are there still other pirates to kill ?
   if gawdead == true and kerdead1 == true and kerdead2 == true and goddead == true then
      tk.msg(_("Mission accomplished"), fmt.f(_("You have killed the four pirates. Now to return to {sys} and collect your payment..."), {sys=paysys}))
      stage = 1
      misn.osdActive(2)
      marker2 = misn.markerAdd(paysys, "low")
   end
end
