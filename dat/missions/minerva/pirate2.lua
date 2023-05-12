--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Minerva Pirates 2">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Bar</location>
 <spob>Minerva Station</spob>
 <done>Minerva Pirates 1</done>
 <notes>
  <campaign>Minerva</campaign>
 </notes>
</mission>
--]]

--[[
-- Destroy a suspicious Za'lek drone.
--]]
local minerva = require "common.minerva"
local vn = require 'vn'
local fmt = require "format"
local lmisn = require "lmisn"

-- Mission constants:
local reward_amount = minerva.rewards.pirate2
local mainsys = system.get("Limbo")
local jumpinsys = system.get("Pultatis")
local drone1pos = vec2.new(  -2000, -15000 )
local drone2pos = vec2.new( -10000,   6000 )

-- Mission states:
--  nil: mission not accepted yet
--    0: Go kill drone
--    1: Get back to Minerva Station
mem.misn_state = nil
local badweaps, drone1, drone2 -- Non-persistent state

function create ()
   if not misn.claim( mainsys ) then
      misn.finish( false )
   end
   misn.setNPC( minerva.pirate.name, minerva.pirate.portrait, minerva.pirate.description )

   local desc = _("Someone wants you to incapacitate a suspicious Za'lek drone with only Dvaered weapons. Usable weapons are:")
   for k,v in ipairs(outfit.getAll()) do
      if v:tags().dvaered then
         desc = desc.._("\n* ")..v:name()
      end
   end

   misn.setDesc( desc )
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
   vn.na(_("You approach the sketchy individual who seems to be calling your attention once again."))
   pir(_([["It seems like our last job worked better than we thought. The Dvaered are all riled up, just as planned. However, there is still a lot left to do."]]))
   pir(_([["I have another job if you are interested. It should be simpler than last time, only this time we target the Za'lek instead of the Dvaered to try to… improve the situation."
She smiles at you.]]))
   vn.menu( {
      {_("Accept the job"), "accept"},
      {_("Kindly decline"), "decline"},
   } )

   vn.label("decline")
   vn.na(_("You decline her offer and take your leave."))
   vn.done()

   vn.label("accept")
   vn.func( function () mem.misn_state=0 end )
   pir(_([["Glad to have you onboard again! So the idea is very simple, we've seen that the Za'lek are setting up some reconnaissance stuff around the system. Nothing very conspicuous, but there are some scout drones here and there. I want you to take them out."]]))
   pir(_([["Just destroying them won't cut it though. We need to make it look like the Dvaered did it this time. However, that should be simple. Make sure to use Dvaered weapons to take the drones out. You know, Mace Launchers, Vulcan Guns, Shredders, Mass Drivers, Railguns and such. Make sure to not use any non-Dvaered weapons!"]]))
   pir(_([["There should be two drones out there. I have given you the locations where they were last seen. Try to take out the drones as fast as possible and get back here in one piece."]]))
   vn.run()

   -- If not accepted, mem.misn_state will still be nil
   if mem.misn_state==nil then
      return
   end

   misn.accept()
   misn.osdCreate( p_("OSD Title", "Za'lek Scout Drone"),
         {_("Destroy the scout drone with Dvaered weapons"),
          _("Go back to Minerva Station") } )
   misn.osdActive(1)

   mem.misnmarker = misn.markerAdd( system.cur() )

   minerva.log.pirate(_("You accept another job from the sketchy individual to destroy some Za'lek scout drones around Minerva Station with Dvaered weapons only to make it seem like the Dvaered are targeting Za'lek drones.") )

   hook.enter("enter")
   hook.load("land")
   hook.land("land")
end


function land ()
   if mem.misn_state==1 and spob.cur() == spob.get("Minerva Station") then
      vn.clear()
      vn.scene()
      local pir = vn.newCharacter( minerva.vn_pirate() )
      vn.music( minerva.loops.pirate )
      vn.transition()
      vn.na(_("After you land on Minerva Station you are once again greeted by the sketchy character that gave you the job to clear the Za'lek drones."))
      pir(_([["Excellent piloting there. We didn't think that the Za'lek would catch on so fast and send in reinforcements, however, it all worked out in the end."]]))
      pir(_([["This should bring suspicions to a new high between Dvaered and Za'lek. There have already been 20 casualties from fighting this period! At this rate they will basically solve themselves. However, that might be a bit slow, so let us try to accelerate the process a bit more."
She winks at you.]]))
      pir(_([["I've wired you some credits for your efforts. Meet me up at the bar for another potential job."]]))
      vn.na(fmt.reward(reward_amount))
      vn.func( function ()
         player.pay( reward_amount )
      end )
      vn.sfxVictory()
      vn.run()

      minerva.log.pirate(_("You succeeded in destroying Za'lek drones and making it seem like the Dvaered were involved.") )

      misn.finish(true)
   end
end

local function dvaered_weapons()
   local pp = player.pilot()
   local baditems = {}
   local allowed = {}
   for k,v in ipairs(outfit.getAll()) do
      if v:tags().dvaered then
         table.insert( allowed, v:nameRaw() )
      end
   end
   for k,o in ipairs(pp:outfitsList("weapon")) do
      if not inlist( allowed, o:nameRaw() ) then
         table.insert( baditems, o:name() )
      end
   end
   for i,p in ipairs(pp:followers()) do
      for k,o in ipairs(p:outfitsList("weapon")) do
         if not inlist( allowed, o:nameRaw() ) then
            table.insert( baditems, o:name()..fmt.f(_(" ({ship})"),{ship=p:name()}) )
         end
      end
   end
   return #baditems==0, baditems
end

local fzalek
local function drone_create( pos )
   local d = pilot.add( "Za'lek Scout Drone", fzalek, pos )
   d:control()
   d:brake()
   hook.pilot( d, "death", "drone_death" )
   hook.pilot( d, "attacked", "drone_attacked" )
   hook.pilot( d, "jump", "drone_ranaway" )
   hook.pilot( d, "land", "drone_ranaway" )
   return d
end


function enter ()
   if mem.misn_state==0 and system.cur()==mainsys then
      mem.weap_ok, badweaps = dvaered_weapons()
      if not mem.weap_ok then
         player.msg("#o"..fmt.f(_("Non-Dvaered equipped weapons detected: {list}"), {list=fmt.list(badweaps)}))
      end

      pilot.clear()
      pilot.toggleSpawn(false)

      -- Gets reset on enter system
      fzalek = faction.dynAdd( "Za'lek", "zalek_thugs", _("Za'lek") )

      drone1 = drone_create( drone1pos )
      mem.drone1marker = system.markerAdd( drone1:pos(), _("Za'lek Drone") )

      mem.drones_killed = 0
   end
end


function drone_death ()
   if not mem.weap_ok then
      lmisn.fail(_("You were supposed to kill the drones with Dvaered-only weapons!"))
   end
   mem.drones_killed = mem.drones_killed+1
   if mem.drones_killed==1 then
      system.markerRm( mem.drone1marker )
      drone2 = drone_create( drone2pos )
      mem.drone2marker = system.markerAdd( drone2:pos(), _("Za'lek Drone") )
      player.msg(_("You detected another Za'lek drone in the system!"))
      mem.zalek_inbound = false
      hook.timer( 0.5, "heartbeat" )
   elseif mem.drones_killed==2 then
      system.markerRm( mem.drone2marker )
      mem.misn_state = 1
      misn.osdActive(2)
      pilot.toggleSpawn(true)
      misn.markerMove( mem.misnmarker, spob.get("Minerva Station") )
   end
end
function drone_attacked( p )
   p:control(false)
   p:setHostile()
end
function drone_ranaway ()
   lmisn.fail(_("You let a drone get away!"))
end


function heartbeat ()
   local pp = player.pilot()
   local dist = pp:pos():dist( drone2pos )
   if dist < 5000 then
      local msg = _("Your sensors are detecting a Za'lek fleet inbound!")
      player.msg("#o"..msg)
      player.autonavAbort(msg)
      hook.timer( 5.0, "reinforcements_jumpin" )
      return
   end
   hook.timer( 0.5, "heartbeat" )
end


function reinforcements_jumpin ()
   local ships = {
      "Za'lek Light Drone",
      "Za'lek Light Drone",
      "Za'lek Heavy Drone",
      "Za'lek Demon",
   }
   for k,s in ipairs(ships) do
      local p = pilot.add( s, fzalek, jumpinsys )
      if drone2:exists() then
         p:setLeader( drone2 )
      end
   end
   player.msg(_("#oZa'lek reinforcements have arrived!"))
end
