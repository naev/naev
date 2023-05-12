--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Introducing Taiomi">
 <location>enter</location>
 <unique />
 <chance>100</chance>
 <cond>system.cur() == system.get("Taiomi")</cond>
 <notes>
  <campaign>Taiomi</campaign>
  <done_evt name="Finding Taiomi" />
 </notes>
</event>
--]]
--[[

   Taiomi Intro Event

--]]
local vn = require 'vn'
local taiomi = require 'common.taiomi'

local drones_create -- Forward-declared function
local drone_faction = faction.get("Independent") -- Constant
local drones, d_scav, evt_state -- Event state, never saved.

--[[
-- Event states:
-- 0: player entered system
-- 1: player landed on the Goddard and took off
-- 2: player finished cutscene and is asked to land on the Goddard
--]]
evt_state = 0

function create ()
   hook.enter("enter")
   hook.land("land")

   -- Run enter stuff
   enter()
end

function enter ()
   -- Only care about Taiomi
   if system.cur() ~= system.get("Taiomi") then return end

   if evt_state==0 then
      taiomi.log.main(_("You have entered Taiomi for the first time."))

      drones_create()
      for k,d in ipairs(drones) do
         d:disable()
      end
   elseif evt_state==1 then
      drones_create() -- Not disable as they move
   elseif evt_state==2 then
      drones_create(9) -- fewer drones

      -- Drones just run around
      for k,d in ipairs(drones) do
         d:control(false)
         d:setNoJump(true)
         local aimem = d:memory()
         aimem.loiter = math.huge -- Should make them loiter forever
      end
   end
end

function drones_create( n )
   n = n or 20

   -- Add frozen and invincible drones
   drones = {}
   for i = 1,n do
      local pos = vec2.newP( 150+2000*rnd.rnd(), rnd.angle() )
      local d = pilot.add( "Drone", drone_faction, pos )
      d:setInvisible(true)
      d:setInvincible(true)
      d:setVel( vec2.new(0,0) )
      d:control()
      table.insert( drones, d )
   end
end

function land ()
   if spob.cur() ~= spob.get("One-Wing Goddard") then
      return
   end

   -- If evt_state==1 we can finish up the event
   if evt_state == 2 then

      vn.clear()
      vn.scene()
      local d = vn.newCharacter( taiomi.vn_scavenger() )
      vn.transition()
      vn.na(_("You dock with the one-winged Goddard and, once again, get out in your atmospheric suit. Once you exit the narrow hallways and entire the command room, you are once again met with the Drone which is a lot more intimidating in person."))
      d(_([["Hello again and welcome to our refuge. It may not seem like much, but it has been our home for generations now."]]))
      d(_([["As you have probably noticed, we are not organic beings such as you, however, we are equally sentient."]]))
      d(_([["Our encounters with humans have not been, in general, very fruitful and we have lost countless of our members over time. The few that remain were able to band together and stumble upon this quiet area, where we have been since."]]))
      d(_([["We scavenge and collect what we can, allowing for our repairs and developments. Despite this, it does not seem like we can continue like this forever."]]))
      d(_([["Although most don't wish to admit it, our numbers are waning and we are unable to maintain them as before. Furthermore, changes in the universe are making it so that more and more humans approach our location, and clashes are inevitable."]]))
      d(_([["We have no choice but to take a large risk and try to move on. However, on our own we might not be able to pull it off before we are reverted to inert debris floating eternally in space."]]))
      d(_([["Although I hate to ask this of you, we do not have any alternative but to push forward. If you would be willing to help our plight, we would be very grateful. We can not offer much, but we may have ways of compensating your efforts. Please take your time to think it over and get in touch with me outside the ship if you are interested."]]))
      vn.na(_("You see how the Drone elegantly and carefully maneuvers out of the Goddard without scratching a single wall."))
      vn.done()
      vn.run()

      taiomi.log.main(_("You have met the robotic inhabitants of Taiomi."))

      -- Have to be able to get back!
      jump.get( "Taiomi", "Bastion" ):setKnown(true)

      -- Since we finish here, next time the player takes off the Taiomi
      -- System event will take over and spawn NPCs
      player.allowSave(true)
      evt.finish(true)
      return
   end

   evt_state = 1
   player.allowSave(false)

   -- Small event
   vn.clear()
   vn.scene()
   vn.music( 'snd/sounds/loops/alienplanet.ogg' )
   vn.transition()
   vn.na(_("You dock with the mostly destroyed Dvaered Goddard and begin to explore the ship in your atmospheric suit. The ship shows heavy signs of fighting, however, given what seem to be high levels of cosmic radiation, it seems to have been wrecked a very long time ago."))
   vn.na(_("Most of the narrow hallways seem untouched, however, in some of the large more open areas you do see what appear to be fresh scratches and signs of things being moved around. Furthermore, you see what could only be described as some sort of crude and primitive wall painting. What could this mean?"))
   vn.na(_("As you are exploring, you suddenly receive an urgent message from your ship. It seems like there are many ships inbound on your position. You rush to get back to your ship and wonder what could be happening."))
   vn.done()
   vn.run()

   -- Force take off
   player.takeoff()

   -- Want to be taken off before starting stuff
   hook.safe( "cutscene00" )
end

function cutscene00 ()
   -- Add scavenger drone
   local pos = vec2.newP( 1500+1000*rnd.rnd(), rnd.angle() )
   d_scav = pilot.add( "Drone (Hyena)", drone_faction, pos, _("Scavenger Drone") )
   d_scav:setInvisible(true)
   d_scav:setInvincible(true)
   d_scav:setVel( vec2.new(0,0) )
   d_scav:control()

   -- Disable control
   player.pilot():control(true)

   -- Start next hook
   hook.timer( 3.0, "cutscene01" )
end

function cutscene01 ()
   -- Zoom out to see everything run around
   camera.setZoom( 2.0 )

   -- Normal drones
   local pp = player.pilot()
   for k,d in ipairs(drones) do
      local _modulus, angle = (d:pos() - pp:pos()):polar()
      local pos = pp:pos() + vec2.newP( 100+100*rnd.rnd(), angle )

      d:taskClear()
      d:moveto( pos )
      d:face( pp )
   end

   -- Scavenger
   d_scav:taskClear()
   d_scav:moveto( pp:pos() + vec2.newP( 300, pp:dir() ) )
   d_scav:face( pp )

   -- Next stage
   hook.timer( 5.0, "cutscene02" )
end

function cutscene02 ()
   camera.setZoom() -- reset zoom
   hook.timer( 6.0, "cutscene03" )
end

function cutscene03 ()
   vn.clear()
   vn.scene()
   local d = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition()
   vn.na(_("As you get into your ship and undock, you quickly find yourself surrounded by many sleek white small ships. As you stare at the ships surrounding you, your comm channel flashes open, however, it is audio only."))
   d(_([["Salutations."
The voice sounds uncanny, almost human but something sets it apart.]]))
   -- Special case player has done parts of the Collective campaign
   --[[
   if player.misnDone("Collective Scouting") then -- this is the first mission
      vn.na(_("You clearly identify the ships as Collective drones. How the hell did they get over here?"))
   end
   --]]
   vn.menu({
      {_([["Hello."]]), "greet"},
      {_([["…"]]), "greet_silent"},
   })

   vn.label("greet_silent")
   d(_([["Was that not the correct human greeting? We have not had much contact with humans."]]))

   -- TODO maybe make this less of a "hey yo, you're welcome here" type thing and have the player make some harder decisions
   vn.label("greet")
   d(_([["This is an extraordinary place, is it not? The stellar winds have created currents that slowly drag many ships from very far away and concentrate them here."]]))
   d(_([["However, you do not seem to have been caught in the stellar wind. The time-frame in which the stellar winds operate does not seem to be compatible with human lifespan."]]))
   d(_([["It is quite remarkable that you made it here. As far as we know, the probability of your conventional sensors finding the jump that brought you here should be, to all practical effects, zero."]]))
   d(_([["We do not wish to harm you, however, our experience leads us to think that co-existence with humans is futile. After analyzing your behaviour we have decided that you do not seem to be a threat. Please note that we do not want knowledge of our existence to spread."]]))
   d(_([["Please dock once more with the Goddard and I will explain the situation slightly more in detail."]]))

   vn.done()
   vn.run()

   -- Re-enable control
   player.pilot():control(false)

   -- Drones just run around
   for k,dk in ipairs(drones) do
      dk:control(false)
      dk:setNoJump(true)
      local aimem = dk:memory()
      aimem.loiter = math.huge -- Should make them loiter forever
   end

   -- Scavenger lands (to avoid having to do more vn stuff for now)
   d_scav:taskClear()
   d_scav:land( system.cur():spobs()[1] )

   -- Update state
   evt_state = 2
end
