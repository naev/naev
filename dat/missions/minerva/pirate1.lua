--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 1">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <cond>var.peek("minerva_altercation_probability")~=nil</cond>
 <notes>
  <campaign>Minerva</campaign>
  <requires name="Minerva Altercation 1" />
 </notes>
</mission>
--]]

--[[
-- Beat up Dvaered thugs making them think the za'leks did it
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local fmt = require "format"
local lmisn = require "lmisn"

-- Mission constants:
local time_needed = 15 -- in seconds
local reward_amount = minerva.rewards.pirate1
local mainsys = system.get("Limbo")
local runawaysys = system.get("Pultatis")
local thugpos = vec2.new( 6000, -4000 )
local dronepos = vec2.new( -12000, -12000 )

-- Mission states:
--  nil: mission not accepted yet
--    0: Go pick up Drone
--    1: Trigger Thugs
--    2: Survive
--    3: Run away
--    4: Get back to Minerva STation
mem.misn_state = nil
local boss, drone, fdrone, fthugs, thugs -- Non-persistent state


function create ()
   if not misn.claim( mainsys ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )
   misn.setDesc( _("Someone wants you to mess around with Dvaered thugs.") )
   misn.setReward( _("Cold hard credits") )
   misn.setTitle( _("Dvaered Thugs") )
end


function accept ()
   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( minerva.vn_pirate() )
   vn.music( minerva.loops.pirate )
   vn.transition()
   vn.label( "leave" )
   vn.na(_("You approach the sketchy individual who seems to be calling your attention."))
   pir(_([["What do you think about them Za'leks and Dvaereds? Quite a work, eh? Always getting into fights with each other and creating trouble for those who just want to enjoy life. Such a bother."]]))
   pir(_([["Say what, I know you're a pretty decent pilot. Would you be interested in a somewhat non-standard job? Nothing very out of the ordinary, just want to ruffle some feathers."]]))
   vn.menu( {
      {_("Accept the job"), "accept"},
      {_("Kindly decline"), "decline"},
   } )

   vn.label("decline")
   vn.na(_("You decline their offer and take your leave."))
   vn.done()

   vn.label("accept")
   vn.func( function ()
      mem.misn_state=0
   end )
   pir(_([["Excellent. As you probably know, the Za'lek and Dvaered want to take control of this station, hence the large amount of military crawling all over the station. This leads to inevitable disagreements, quarrels, brawls, altercations, fights, you name it. Instead of trying to take care of it directly, we can sort of encourage them to take care of each other and problem solved, no?"]]))
   pir(_([["I know what you're thinking, that's a great idea right? So it's very simple. Some Dvaered thugs are stationed around the system, they are not the cleverest of folks, so I want you to provoke them. You know, just rough them up a little and get out of there."]]))
   pir(_([["Sounds naïve, yes? Might be. So, I've managed to get a Za'lek drone shell, all it has is the engine and some basic following software, but no weapons or gear. If you were to drag it along while harassing the thugs, they probably would think that there is some kind of Za'lek involvement. They're not the smartest fellows in the world if you catch my drift."]]))
   pir(_([["To make sure they are all riled up, I want you to spend 15 seconds harassing them near their original location. Make sure to harass them, but don't kill them! We want them to tell the other Dvaereds about this. Once the time is up, get the hell away from there, in one piece if possible."]]))
   pir(fmt.f(_([["I've sent you the coordinates of both the Za'lek drone and the Dvaered thugs. I'll pay you well if you manage to pull this off. Oh and one more thing, when getting away, make sure to jump to the {sys} system to make it look even more like the Za'lek did it."
They beam a smile at you.]]), {sys=runawaysys}))
   vn.run()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end

   minerva.log.pirate(_("You accepted a job from a sketchy individual to harass Dvaered thugs in the Limbo system and make it seem like the Za'lek were involved.") )

   misn.accept()
   misn.osdCreate( _("Thug Decoy"),
         {_("Get the drone to follow you"),
          _("Harass the thugs"),
          fmt.f(_("Jump to {sys}"), {sys=runawaysys}),
          _("Go back to Minerva Station") } )
   misn.osdActive(1)

   mem.misnmarker = misn.markerAdd( system.cur() )

   hook.enter("enter")
   hook.load("land")
   hook.land("land")
end


function land ()
   if mem.misn_state==4 and spob.cur() == spob.get("Minerva Station") then
      vn.clear()
      vn.scene()
      vn.music( minerva.loops.pirate )
      local pir = vn.newCharacter( minerva.vn_pirate() )
      vn.transition()
      vn.na(_("After you land on Minerva Station you are once again greeted by the sketchy character that gave you the job dealing with the Dvaered thugs."))
      pir(_([["I hear it went rather well. This should cause more tension between the Za'lek and the Dvaered so we can get them off this station. However, this is only the beginning."]]))
      pir(_([["If you are interested, I may have another job for you which I believe you are more than capable of handling. Meet me up at the bar if you want more information. I have also transferred a sum of credits to your account as a reward for your services."
She winks at you and walks way.]]))
      vn.na(fmt.reward(reward_amount))
      vn.func( function ()
         player.pay( reward_amount )
      end )
      vn.sfxVictory()
      vn.run()

      minerva.log.pirate(_("You succeeded in making the Dvaered think that the Za'lek were harassing their thugs near Minerva station.") )
      misn.finish(true)
   end
end


function enter ()
   if mem.misn_state==1 or mem.misn_state==2 then
      lmisn.fail(_("You were supposed to harass the thugs with the drone."))
   end

   if mem.misn_state==3 then
      if system.cur()==runawaysys then
         mem.misn_state=4
         misn.osdActive(4)
         misn.markerMove( mem.misnmarker, spob.get("Minerva Station") )
      else
         lmisn.fail(fmt.f(_("You were supposed to jump to the {sys} system!"), {sys=runawaysys}))
      end
   end

   if system.cur()==mainsys and mem.misn_state==0 then
      pilot.clear()
      pilot.toggleSpawn(false)

      fthugs = faction.dynAdd( "Dvaered", "Dvaered Thugs", _("Dvaered Thugs") )

      local pos = thugpos
      boss = pilot.add( "Dvaered Vigilance", fthugs, pos )
      boss:control()
      boss:brake()
      hook.pilot( boss, "attacked", "thugs_attacked" )
      hook.pilot( boss, "death", "thugs_dead" )
      thugs = { boss }
      for i = 1,3 do
         pos = thugpos + vec2.newP( rnd.rnd(50,150), rnd.angle() )
         local p = pilot.add( "Dvaered Vendetta", fthugs, pos )
         p:setLeader( boss )
         hook.pilot( p, "attacked", "thugs_attacked" )
         hook.pilot( p, "death", "thugs_dead" )
         table.insert( thugs, p )
      end

      fdrone = faction.dynAdd( "Independent", "Drone", _("Drone") )
      drone = pilot.add( "Za'lek Light Drone", fdrone, dronepos )
      drone:control()
      drone:brake()

      mem.dronemarker = system.markerAdd( drone:pos(), _("Za'lek Drone") )
      mem.thugsmarker = system.markerAdd( boss:pos(), _("Dvaered Thugs") )

      hook.timer( 0.5, "heartbeat" )
   end
end


function heartbeat ()
   local pp = player.pilot()
   local dist =  pp:pos():dist( drone:pos() )
   if dist < 1000 then
      player.msg(_("#gThe drone begins to follow you."), true)
      mem.misn_state=1
      misn.osdActive(2)
      drone:taskClear()
      drone:follow(pp)
      system.markerRm( mem.dronemarker )
      return
   end
   hook.timer( 0.5, "heartbeat" )
end


function thugs_attacked ()
   if mem.misn_state==0 then
      pilot.toggleSpawn(true)
      lmisn.fail(_("You were supposed to harass the thugs with the drone."))
   elseif mem.misn_state==1 then
      faction.dynEnemy( fdrone, fthugs ) -- Make enemies
      boss:broadcast(_("I'm going to wash my ship's hull with your blood, Za'lek Scum!"))
      boss:control(false)
      mem.misn_state=2
      for k,v in ipairs(thugs) do
         v:setHostile()
      end
      mem.total_harassment = 0
      hook.timer( 1.0, "harassed" )
   end
end


function thugs_dead ()
   pilot.toggleSpawn(true)
   lmisn.fail(_("You were supposed to harass the thugs, not kill them!"))
end


function harassed ()
   local pp = player.pilot()
   local dist = pp:pos():dist( thugpos )
   if dist > 5000 then
      if mem.failingdistance==nil then
         player.msg(_("#rYou are moving too far away from the harassment point!"), true)
         mem.failingdistance = 0
      end
      mem.failingdistance = mem.failingdistance + 1
      if mem.failingdistance > 6 then
         pilot.toggleSpawn(true)
         lmisn.fail(_("You moved too far away from the harassment location!"))
      end
   else
      mem.failingdistance = nil
   end
   mem.total_harassment = mem.total_harassment + 1
   if mem.total_harassment > time_needed then
      player.msg(_("#gThe thugs seem to be sufficiently riled up."))
      mem.misn_state=3
      misn.osdActive(3)
      system.markerRm( mem.thugsmarker )
      misn.markerMove( mem.misnmarker, runawaysys )
      return
   end
   hook.timer( 1.0, "harassed" )
end
