--[[

   Empire VIP Rescue

   Author: bobbens
      minor edits by Infiltrator
      - Mission fixed to suit big systems (Anatolis, 11/02/2011)

   Rescue a VIP stranded on a disabled ship in a system while FLF and Dvaered
    are fighting.

   Stages:

      0) Go to sector.
      1) Board ship and rescue VIP.
      2) Rescued VIP, returning to base.
      3) VIP died or jump out of system without VIP  --> mission failure.
]]--

include "dat/scripts/numstring.lua"

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   -- Mission details
   bar_desc = "Commander Soldner is waiting for you."
   misn_title = "Empire VIP Rescue"
   misn_reward = "%s credits"
   misn_desc = {}
   misn_desc[1] = "Rescue the VIP from a transport ship in the %s system."
   misn_desc[2] = "Return to %s in the %s system with the VIP."
   -- Fancy text messages
   title = {}
   title[1] = "Commander Soldner"
   title[2] = "Disabled Ship"
   title[3] = "Mission Success"
   title[4] = "Mission Failure"
   text = {}
   text[1] = [[You meet up once more with Commander Soldner at the bar.
    "Hello again, %s. Still interested in doing another mission? This one will be more dangerous."]]
   text[2] = [[Commander Soldner nods and continues, "We've had reports that a transport vessel came under attack while transporting a VIP. They managed to escape, but the engine ended up giving out in the %s system. The ship is now disabled and we need someone to board the ship and rescue the VIP. There have been many FLF ships detected near the sector, but we've managed to organise a Dvaered escort for you."
    "You're going to have to fly to the %s system, find and board the transport ship to rescue the VIP, and then fly back. The sector is most likely going to be hot. That's where your Dvaered escorts will come in. Their mission will be to distract and neutralise all possible hostiles. You must not allow the transport ship to be destroyed before you rescue the VIP. His survival is vital."]]
   text[3] = [["Be careful with the Dvaered; they can be a bit blunt, and might accidentally destroy the transport ship. If all goes well, you'll be paid %d credits when you return with the VIP. Good luck, pilot."]]
   text[4] = [[The ship's hatch opens and immediately an unconscious VIP is brought aboard by his bodyguard. Looks like there is no one else aboard.]]
   text[5] = [[You land at the starport. It looks like the VIP has already recovered. He thanks you profusely before heading off. You proceed to pay Commander Soldner a visit. He seems to be happy, for once.
    "It seems like you managed to pull it off. I had my doubts at first, but you've proven to be a very skilled pilot. We have nothing more for you now, but check in periodically in case something comes up for you."]]
   msg = {}
   msg[1] = "MISSION FAILED: VIP is dead."
   msg[2] = "MISSION FAILED: You abandoned the VIP."
end

include "dat/missions/empire/common.lua"

function create ()
   -- Target destination
   destsys     = system.get( "Slaccid" )
   ret,retsys  = planet.getLandable( "Halir" )
   if ret== nil then
      misn.finish(false)
   end

   -- Must claim system
   if not misn.claim( destsys ) then
      misn.finish(false)
   end

   -- Add NPC.
   misn.setNPC( "Soldner", "empire/unique/soldner" )
   misn.setDesc( bar_desc )
end


function accept ()

   -- Intro text
   if not tk.yesno( title[1], string.format( text[1], player.name() ) ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   -- Set marker
   misn_marker = misn.markerAdd( destsys, "low" )

   -- Mission details
   misn_stage = 0
   reward = 75000
   misn.setTitle(misn_title)
   misn.setReward( string.format(misn_reward, numstring(reward)) )
   misn.setDesc( string.format(misn_desc[1], destsys:name() ))

   -- Flavour text and mini-briefing
   tk.msg( title[1], string.format( text[2], destsys:name(), destsys:name() ) )
   tk.msg( title[1], string.format( text[3], reward ) )
   misn.osdCreate(misn_title, {misn_desc[1]:format(destsys:name())})
   -- Set hooks
   hook.land("land")
   hook.enter("enter")
   hook.jumpout("jumpout")

   -- Initiate mission variables (A.)
   prevsys = system.cur()
end


function land ()
   landed = planet.cur()

   if landed == ret then
      -- Successfully rescued the VIP
      if misn_stage == 2 then

         -- VIP gets off
         misn.cargoRm(vip)

         -- Rewards
         player.pay(reward)
         emp_modReputation( 5 ) -- Bump cap a bit
         faction.modPlayerSingle("Empire",5);
         faction.modPlayerSingle("Dvaered",5);

         -- Flavour text
         tk.msg( title[3], text[5] )

         misn.finish(true)

      -- Mister VIP is dead
      elseif misn_stage == 3 then

         -- What a disgrace you are, etc...
         tk.msg( title[4], text[6] )
         emp_modReputation( 5 ) -- Bump cap a bit

         misn.finish(true)
      end
   end
end


function enter ()
   sys = system.cur()

   if misn_stage == 0 and sys == destsys then

      -- Put the VIP a ways off of the player but near the jump.
      enter_vect = jump.pos(sys, prevsys)
      m,a = enter_vect:polar()
      enter_vect:setP( m-3000, a )
      p = pilot.add( "Trader Gawain", "dummy", enter_vect )
      for _,v in ipairs(p) do
         v:setPos( enter_vect )
         v:setVel( vec2.new( 0, 0 ) ) -- Clear velocity
         v:disable()
         v:setHilight(true)
         v:setVisplayer(true)
         v:setFaction( "Empire" )
         hook.pilot( v, "board", "board" )
         hook.pilot( v, "death", "death" )
      end

      -- FLF Spawn around the Gawain
      p = pilot.add( "FLF Med Force", nil, enter_vect )
      for k,v in ipairs(p) do
         v:setHostile()
      end
      -- To make it more interesting a vendetta will solely target the player.
      p = pilot.add( "FLF Vendetta", nil, enter_vect )[1]
      p:control()
      p:setHostile()
      p:attack( player.pilot() )
      
      -- Now Dvaered
      -- They will jump together with you in the system at the jumppoint. (A.)
      p = pilot.add( "Dvaered Med Force", nil, prevsys )
      for k,v in ipairs(p) do
         v:setFriendly()
      end

      -- Add more ships on a timer to make this messy
      hook.timer(rnd.rnd( 3000, 5000 ) , "delay_flf")

      -- Pass to next stage
      misn_stage = 1

   -- Can't run away from combat
   elseif misn_stage == 1 then

      -- Notify of mission failure
      player.msg( msg[2] )
      misn.finish(false)

   end
end

function jumpout ()
   -- Storing the system the player jumped from.
   prevsys = system.cur()
end


function delay_flf ()

   if misn_stage ~= 0 then
      return
   end

   -- More ships to pressure player from behind
   p = pilot.add( "FLF Sml Force", nil, prevsys )
   for k,v in ipairs(p) do
      v:setHostile()
   end
end


function board ()
   -- VIP boards
   vip = misn.cargoAdd( "VIP", 0 )
   tk.msg( title[2], text[4] )

   -- Update mission details
   misn_stage = 2
   misn.markerMove( misn_marker, retsys )
   misn.setDesc( string.format(misn_desc[2], ret:name(), retsys:name() ))
   misn.osdCreate(misn_title, {misn_desc[2]:format(ret:name(),retsys:name())})

   -- Force unboard
   player.unboard()
end


function death ()
   if misn_stage == 1 then
      -- Notify of death
      player.msg( msg[1] )

      -- Update mission details
      misn_stage = 3
      misn.finish(false)
   end
end

function abort ()
   -- If aborted you'll also leave the VIP to fate. (A.)
   player.msg( msg[2] )
   misn.finish(false)
end
