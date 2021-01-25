--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 2">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Bar</location>
  <planet>Minerva Station</planet>
  <done>Minerva Pirates 1</done>
 </avail>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
-- Destroy a suspicious Za'lek drone.
--]]
local minerva = require "minerva"
local portrait = require 'portrait'
local vn = require 'vn'
require 'numstring'

logidstr = minerva.log.pirate.idstr

misn_title = _("Dvaered Thugs")
misn_reward = _("Cold hard credits")
misn_desc = _("Someone wants you to incapacitate a suspicious Za'lek drone.")
reward_amount = 200000 -- 200k

mainsys = "Limbo"
jumpinsys = "Pultatis"
-- Mission states:
--  nil: mission not accepted yet
--    0: Go kill drone
--    1: Get back to Minerva STation
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
   vn.na(_("You approach the sketch individual who seems to be calling your attention once again."))
   pir(_([["It seems like our last job worked better than we thought. The Dvaered are all riled up, just as planned. However, there is still a lot left to do."]]))
   pir(_([["I have another job if you are interested, it should be simpler than last time, only this time we target the Za'lek instead of the Dvaered to try to... improve the situation."
They smiles at you.]]))
   vn.menu( {
      {_("Accept the job"), "accept"},
      {_("Kindly decline"), "decline"},
   } )
   
   vn.label("decline")
   vn.na(_("You decline their offer and take your leave."))
   vn.fadeout()
   vn.done()

   vn.label("accept")
   vn.func( function () misn_state=0 end )
   pir(_([["Glad to have you onboard again! So the idea is very simple, we've seen that the Za'lek are setting up some reconnaissance stuff around the system. Noting very conspicuous, but there are some scout drones here and there. I want you to take them out."]]))
   pir(_([["Just destroying them won't cut it though, we need to make it look like the Dvaered did it this time. However, that should be simple. Make sure to use Dvaered weapons to take the drones out. You know, Mace Launchers, Vulcan Guns, Shredders, Mass Drivers, Railguns and such. Make sure to not use any non-Dvaered weapons!"]]))
   pir(_([["There should be two drones out there. I have given you the locations where they were last seen. Try to take out the drones as fast as possible and get back here in one piece."]]))
   vn.fadeout()
   vn.run()

   -- If not accepted, misn_state will still be nil
   if misn_state==nil then
      misn.finish(false)
      return
   end

   misn.accept()
   misn.setDesc( misn_desc )
   osd = misn.osdCreate( _("Za'lek Scout Drone"),
         {_("Destroy the scout drone with Dvaered weapons"),
          _("Go back to Minerva Station") } )
   misn.osdActive(1)

   shiplog.appendLog( logidstr, _("You accept another job from the shady individual to destroy some Za'lek scout drones around Minerva Station with Dvaered weapons only to make it seem like the Dvaered are targeting Za'lek drones.") )

   hook.enter("enter")
   hook.load("land")
   hook.land("land")
end


function land ()
   if misn_state==1 then
      vn.clear()
      vn.scene()
      local pir = vn.newCharacter( minerva.pirate.name, {image=minerva.pirate.image} )
      vn.fadein()
      vn.na(_("After you land on Minerva Station you are once again greeted by the shady character that gave you the job to clear the Za'lek drones."))
      pir(_([["Excellent piloting there. We didn't think that the Za'lek would catch on so fast and send in reinforcements, however, it all worked out in the end."]]))
      pir(_([["This should make bring suspicions to a new high between Dvaered and Za'lek. There have already been 20 casualties from fighting this period! At this rate they will basically solve themselves. However, that might be a bit slow, so let us try to accelerate the process a bit more."
They wink at you.]]))
      pir(_([["I've wired you some credits for your efforts. Meet me up at the bar for a new potential job."]]))
      vn.na(string.format(_("You have received #g%s."), creditstring(reward_amount)))
      vn.func( function ()
         player.pay( reward_amount )
      end )
      vn.sfxVictory()
      vn.fadeout()
      vn.run()
   
      shiplog.appendLog( logidstr, _("You succeeded in destroying Za'lek drones and making it seem like the Dvaered were involved.") )

      misn.finish(true)
   end
end


function dvaered_weapons( p )
   local function inlist( val, list )
      for k,v in ipairs(list) do
         if v==val then
            return true
         end
      end
      return false
   end
   local allowed = {
      "Unicorp Mace Launcher",
      "Vulcan Gun",
      "Shredder",
      "Mass Driver MK1",
      "Mass Driver MK2",
      "Mass Driver MK3",
      "Turreted Gauss Gun",
      "Turreted Vulcan Gun",
      "Railgun Turret",
      "Repeating Railgun",
   }
   local weapons = p:outfits( "weapon" )
   local baditems = {}
   if #weapons==0 then
      return false, baditems -- Need weapons
   end
   for k,o in ipairs(weapons) do
      if not inlist( o:nameRaw(), allowed ) then
         table.insert( baditems, o:name() )
      end
   end
   return #baditems==0, baditems
end


function drone_create( pos )
   local d = pilot.addRaw( "Za'lek Scout Drone", fzalek, pos )
   d:control()
   d:brake()
   hook.pilot( d, "death", "drone_death" )
   hook.pilot( d, "attacked", "drone_attacked" )
   hook.pilot( d, "jump", "drone_ranaway" )
   hook.pilot( d, "land", "drone_ranaway" )
   return d
end


function enter ()
   if misn_state==0 and system.cur()==system.get(mainsys) then
      weap_ok, badweaps = dvaered_weapons( player.pilot() )
      if not weap_ok then
         player.msg(string.format(_("#oNon-Dvaered equipped weapons detected: %s"), table.concat(badweaps,_(", "))))
      end

      pilot.clear()
      pilot.toggleSpawn(false)
      drone1pos = vec2.new(  -2000, -15000 )
      drone2pos = vec2.new( -10000,   6000 )

      fzalek = faction.dynAdd( "Za'lek", "zalek_thugs", "Za'lek", "zalek" )

      drone1 = drone_create( drone1pos )
      drone1marker = system.mrkAdd( _("Za'lek Drone"), drone1:pos() )

      drones_killed = 0
   end
end


function drone_death ()
   if not dvaered_weapons( player.pilot() ) then
      player.msg(_("#rMISSION FAILED! You were supposed to kill the drones with Dvaered-only weapons!"))
   end
   drones_killed = drones_killed+1
   if drones_killed==1 then
      system.mrkRm( drone1marker )
      drone2 = drone_create( drone2pos )
      drone2marker = system.mrkAdd( _("Za'lek Drone"), drone2:pos() )
      player.msg(_("You detected another Za'lek drone in the system!"))
      zalek_inbound = false
      hook.timer( 500, "heartbeat" )
   elseif drones_killed==2 then
      system.mrkRm( drone2marker )
      misn_state = 1
      misn.osdActive(2)
      pilot.toggleSpawn(true)
   end
end
function drone_attacked( p )
   p:control(false)
   p:setHostile()
end
function drone_ranaway ()
   player.msg(_("#rMISSION FAILED! You let a drone get away!"))
   misn.finish(false)
end


function heartbeat ()
   local pp = player.pilot()
   local dist = pp:pos():dist( drone2pos )
   if dist < 5000 then
      local msg = _("Your sensors are detecting a Za'lek fleet inbound!")
      player.msg("#o"..msg)
      player.autonavAbort(msg)
      hook.timer( 5000, "reinforcements_jumpin" )
      return
   end
   hook.timer( 500, "heartbeat" )
end


function reinforcements_jumpin ()
   local ships = {
      "Za'lek Light Drone",
      "Za'lek Light Drone",
      "Za'lek Heavy Drone",
      "Za'lek Demon",
   }
   for k,s in ipairs(ships) do
      local p = pilot.addRaw( s, fzalek, system.get(jumpinsys) )
      if drone2:exists() then
         p:setLeader( drone2 )
      end
   end
   player.msg(_("#oZa'lek reinforcements have arrived!"))
end

