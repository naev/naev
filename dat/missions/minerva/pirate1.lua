--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 1">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Minerva Station</planet>
  <cond>var.peek("minerva_altercation_probability")~=nil</cond>
 </avail>
 <notes>
  <campaign>Minerva</campaign>
  <requires name="Minerva Altercation 1" />
 </notes>
</mission>
--]]

--[[
-- Beat up Dvaered thugs making them think the za'leks did it
--]]
local minerva = require "minerva"
local portrait = require 'portrait'
local vn = require 'vn'
require 'numstring'

logidstr = minerva.log.pirate.idstr

misn_title = _("Dvaered Thugs")
misn_reward = _("Cold hard credits")
misn_desc = _("Someone wants you to mess around with Dvaered thugs.")
time_needed = 15 -- in seconds
reward_amount = 300000 -- 300k

mainsys = "Limbo"
runawaysys = "Pultatis"
-- Mission states:
--  nil: mission not accepted yet
--    0: Go pick up Drone
--    1: Trigger Thugs
--    2: Survive
--    3: Run away
--    4: Get back to Minerva STation
misn_state = nil


function create ()
   if not misn.claim( system.get(mainsys) ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.pirate.name, minerva.pirate.portrait )
   misn.setDesc( minerva.pirate.description )
   misn.setReward( misn_reward )
   misn.setTitle( misn_title )
end


function accept ()
   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( minerva.pirate.name, {image=minerva.pirate.image} )
   vn.fadein()
   vn.label( "leave" )
   vn.na(_("You approach the sketch individual who seems to be calling your attention."))
   pir(_([["What do you think about them Za'leks and Dvaereds? Quite a work, eh? Always getting into fights with each other and creating trouble for those who just want to enjoy life. Such a bother."]]))
   pir(_([["Say what, I know you're a pretty decent pilot. Would you be interested in a somewhat non-standard job? Nothing very out of the ordinary, just want to ruff up some feathers."]]))
   vn.menu( {
      {_("Accept the job"), "accept"},
      {_("Kindly decline"), "decline"},
   } )
   
   vn.label("decline")
   vn.na(_("You decline their offer and take your leave."))
   vn.fadeout()
   vn.done()

   vn.label("accept")
   vn.func( function ()
      misn_state=0
   end )
   pir(_([["Excellent. As you probably know, the Za'lek and Dvaered want to take control of this station, hence the large amount of military crawling over the station. This leads to inevitable disagreements, quarrels, brawls, altercations, fights, you name it. Instead of trying to take care of it directly, we can sort of encourage them to take care of each other and problem solved, no?]]))
   pir(_([["I know what you're thinking, that's a great idea right? So it's very simple. Some Dvaered thugs are station around the system, they are not the cleverest of folks, so I want you to provoke them. Just rough them up a little and get out of there."]]))
   pir(_([["Sounds naïve, yes? Well, I've managed to get a Za'lek drone shell, all it has is the engine and some basic following software, but no weapons nor gear. If you drag it along while harassing the thugs, they should probably think that it has some sort of Za'lek involvement. They're not the smartest fellows in the world if you catch my drift."]]))
   pir(_([["To make sure they are all riled up, spend 15 seconds harassing them near their original location. Make sure to harass them, and not kill them. We want them to inform the other Dvaereds about this. Afterwards, run away in one piece if possible.."]]))
   pir(string.format(_([["I've sent you the coordinates of both the Za'lek drone and the Dvaered thugs. I'll pay you well if you manage to pull this off. Oh and one thing, when getting away jump to the %s system to make it look even more like the Za'lek did it,"
They beam a smile at you.]]),_(runawaysys)))
   vn.fadeout()
   vn.run()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      misn.finish(false)
      return
   end

   shiplog.createLog( logidstr, minerva.log.pirate.logname, minerva.log.pirate.logtype, true )
   shiplog.appendLog( logidstr, _("You accepted a job from a shady individual to harass Dvaered thugs and make it seem like the Za'lek were involved.") )

   misn.accept()
   misn.setDesc( misn_desc )
   osd = misn.osdCreate( _("Thug Decoy"),
         {_("Get the drone to follow you"),
          _("Harass the thugs"),
          string.format(_("Jump to %s"),_(runawaysys)),
          _("Go back to Minerva Station") } )
   misn.osdActive(1)

   hook.enter("enter")
   hook.load("land")
   hook.land("land")
end


function land ()
   if misn_state==4 then
      vn.clear()
      vn.scene()
      local pir = vn.newCharacter( minerva.pirate.name, {image=minerva.pirate.image} )
      vn.fadein()
      vn.na(_("After you land on Minerva Station you are once again greeted by the shady character that gave you the job dealing with the Dvaered thugs."))
      pir(_([["I hear it went rather well. This should cause more tension between the Za'lek and the Dvaered so we can get them off this station. However, this is only the beginning."]]))
      pir(_([["If you are interested, I may have another job for you which I believe you are more than capable of handling. Meet me up at the bar if you are interested. I have also transferred"]]))
      vn.na(string.format(_("You have received #g%s."), creditstring(reward_amount)))
      vn.func( function ()
         player.pay( reward_amount )
      end )
      vn.sfxVictory()
      vn.fadeout()
      vn.run()

      shiplog.appendLog( logidstr, _("You succeeded in making the Dvaered think that the Za'lek were harassing their thugs near Minerva station.") )
      misn.finish(true)
   end
end


function enter ()
   if misn_state==1 or misn_state==2 then
      player.msg(_("#rMISSION FAILED! You were supposed to harass the thugs with the drone."))
      misn.finish(false)
   end

   if misn_state==3 then
      if system.cur()==system.get(runawaysys) then
         misn_state=4
         misn.osdActive(4)
      else
         player.msg(string.format(_("#rMISSION FAILED! You were supposed to jump to the %s system!"),_(runawaysys)))
         misn.finish(false)
      end
   end

   if system.cur()==system.get(mainsys) and misn_state==0 then
      pilot.clear()
      pilot.toggleSpawn(false)
      thugpos = vec2.new( 6000, -4000 )
      dronepos = vec2.new( -12000, -12000 )

      fthugs = faction.dynAdd( "Dvaered", "Dvaered Thugs" )

      local pos = thugpos
      boss = pilot.addRaw( "Dvaered Vigilance", fthugs, pos, "dvaered" )
      boss:control()
      boss:brake()
      hook.pilot( boss, "attacked", "thugs_attacked" )
      hook.pilot( boss, "death", "thugs_dead" )
      thugs = { boss }
      for i = 1,3 do
         pos = thugpos + vec2.newP( rnd.rnd(50,150), rnd.rnd(1,360) )
         local p = pilot.addRaw( "Dvaered Vendetta", fthugs, pos, "dvaered" )
         p:setLeader( boss )
         hook.pilot( p, "attacked", "thugs_attacked" )
         hook.pilot( p, "death", "thugs_dead" )
         table.insert( thugs, p )
      end

      fdrone = faction.dynAdd( "Independent", "Drone" )
      drone = pilot.addRaw( "Za'lek Light Drone", fdrone, dronepos, "zalek" )
      drone:control()
      drone:brake()

      dronemarker = system.mrkAdd( _("Za'lek Drone"), drone:pos() )
      thugsmarker = system.mrkAdd( _("Dvaered Thugs"), boss:pos() )

      hook.timer( 500, "heartbeat" )
   end
end


function heartbeat ()
   local pp = player.pilot()
   local dist =  pp:pos():dist( drone:pos() ) 
   if dist < 1000 then
      player.msg(_("#gThe drone begins to follow you."))
      misn_state=1 
      misn.osdActive(2)
      drone:taskClear()
      drone:follow(pp)
      system.mrkRm( dronemarker )
      return
   end
   hook.timer( 500, "heartbeat" )
end


function thugs_attacked ()
   if misn_state==0 then
      player.msg(_("#rMISSION FAILED! You were supposed to harass the thugs with the drone."))
      misn.finish(false)
   elseif misn_state==1 then
      faction.dynEnemy( fdrone, fthugs ) -- Make enemies
      boss:broadcast(_("I'm going to wash my ship's hull with your blood, Za'lek Scum!"))
      boss:control(false)
      misn_state=2
      for k,v in ipairs(thugs) do
         v:setHostile()
      end
      total_harassment = 0
      hook.timer( 1000, "harassed" )
   end
end


function thugs_dead ()
   player.msg(_("#rMISSION FAILED! You were supposed to harass the thugs, not kill them!"))
   misn.finish(false)
end


function harassed ()
   local pp = player.pilot()
   local dist = pp:pos():dist( thugpos )
   if dist > 5000 then
      player.msg(_("#rMISSION FAILED! You moved too far away from the harassment location!"))
      misn.finish(false)
   end
   total_harassment = total_harassment + 1
   if total_harassment > time_needed then
      player.msg(_("#gThe thugs seem to be sufficiently riled up."))
      misn_state=3
      misn.osdActive(3)
      system.mrkRm( thugsmarker )
      return
   end
   hook.timer( 1000, "harassed" )
end



