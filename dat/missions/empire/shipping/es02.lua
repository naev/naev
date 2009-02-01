--[[

   Empire VIP Rescue

   Rescue a VIP stranded on a disabled ship in a system while FLF and Dvaered
    are fighting.

   Stages:

      0) Go to sector.
      1) Board ship and rescue VIP.
      2) Rescued VIP, returning to base.
      3) VIP died, mission failure.
]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   -- Mission details
   misn_title = "Empire Shipping Delivery"
   misn_reward = "%d credits"
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
"Hello again %s.  Still interested in doing another mission?  This one will be more dangerous."]]
   text[2] = [[Commander Soldner continues, "We've had reports that a transport vessel was under attack while transporting a VIP.  It managed to escape but the engine ended up giving out in the %s system.  The ship is now disabled and we need someone to board the ship and rescue the VIP.  There's been many FLF ships detected near the sector and we've managed to get a Dvaered escort for you."
"You're going to have to fly to the %s system, board the transport ship to rescue the VIP and fly back.  There's most likely going to be combat, that's where the Dvaereds will come in.  Their mission will be to attempt to distract and neutralize all possible hostiles.  You must not let the transport ship get destroyed before you rescue the VIP."]]
   text[3] = [["Be careful with the Dvaered, they can be a bit blunt and might accidentally destroy the transport ship.  If mission goes well you'll be paid %d credits when you return with the VIP.  Good luck pilot."]]
   text[4] = [[The ship's hatch opens and immediately an unconscious VIP is brought aboard by his bodyguard.  Looks like there is no one else aboard.]]
   text[5] = [[You land at the starport.  It looks like the VIP has already recovered.  He thanks your profusely before heading off.  You proceed to pay Commander Soldner a visit.  He seems to be happy.
"It seems like you managed to pull it off.  I had my doubts at first, but you've proven to be a very good pilot.  I'm putting the good word with the Dvaered for you.  Hopefully they'll start offering you work also.  We have no more missions for you now, but check in periodically in case we have a job for you."]]
   msg = {}
   msg[1] = "MISSION FAILED: VIP is dead."
   msg[2] = "MISSION FAILED: You left the VIP abandoned."
end


function create ()

   -- Intro text
   if tk.yesno( title[1], string.format( text[1], player.name() ) )
      then
      misn.accept()

      -- target destination
      destsys = space.getSystem( "Slaccid" )
      ret,retsys = space.getPlanet( "Polaris Prime" )
      misn.setMarker(destsys)

      -- Mission details
      misn_stage = 0
      reward = 75000
      misn.setTitle(misn_title)
      misn.setReward( string.format(misn_reward, reward) )
      misn.setDesc( string.format(misn_desc[1], destsys:name() ))

      -- Flavour text and mini-briefing
      tk.msg( title[1], string.format( text[2], destsys:name(), destsys:name() ) )
      tk.msg( title[1], string.format( text[3], reward ) )

      -- Set hooks
      hook.land("land")
      hook.enter("enter")
   end
end


function land ()
   landed = space.getPlanet()

   if landed == ret then
      -- Successfully rescued the VIP
      if misn_stage == 2 then

         -- VIP gets off
         player.rmCargo(vip)

         -- Rewards
         player.pay(reward)
         player.modFaction("Empire",5);

         -- Flavour text
         tk.msg( title[3], text[5] )

         misn.finish(true)

      -- Mister VIP is dead
      elseif misn_stage == 3 then

         -- What a disgrace you are, etc...
         tk.msg( title[4], text[6] )

         misn.finish(true)
      end
   end
end


function enter ()
   sys = space.getSystem()

   if misn_stage == 0 and sys == destsys then

      -- Put the VIP at the far end of the player
      enter_vect = player.pos()
      x,y = enter_vect:get()
      d = 1200
      enter_vect:set( d * -x / math.abs(x), d * -y / math.abs(y) )
      p = pilot.add( "Trader Gawain", "dummy", enter_vect )
      for k,v in ipairs(p) do
         v:warp( enter_vect ) -- Will clear velocity
         v:disable()
         v:setFaction( "Empire" )
         hook.pilot( v, "board", "board" )
         hook.pilot( v, "death", "death" )
      end

      -- We'll toss all other ships in the middle
      -- FLF first
      a = rnd.rnd() * 2 * math.pi
      d = rnd.rnd( 0, 700 )
      enter_vect:set( math.cos(a) * d, math.sin(a) * d )
      p = pilot.add( "FLF Lge Force", "def", enter_vect )
      for k,v in ipairs(p) do
         v:setHostile()
      end
      -- Now Dvaered
      a = rnd.rnd() * 2 * math.pi
      d = rnd.rnd( 0, 700 )
      enter_vect:set( math.cos(a) * d, math.sin(a) * d )
      p = pilot.add( "Dvaered Med Force", "def", enter_vect )
      for k,v in ipairs(p) do
         v:setFriendly()
      end

      -- Add more ships on a timer to make this messy
      enter_vect = player.pos()
      misn.timerStart( "delay_flf", rnd.rnd( 3000, 5000 ) )

      -- Pass to next stage
      misn_stage = 1

   -- Can't run away from combat
   elseif misn_stage == 1 then

      -- Notify of mission failure
      player.msg( msg[2] )
      --[[
      misn_stage = 3
      misn.setMarker(retsys)
      misn.setDesc( string.format(misn_desc[2], ret:name(), retsys:name() ))
      ]]--
      misn.finish(false)

   end
end


function delay_flf ()

   -- More ships to pressue player from behind
   p = pilot.add( "FLF Med Force", "def", enter_vect )
   for k,v in ipairs(p) do
      v:setHostile()
   end
end


function board ()
   -- VIP boards
   vip = player.addCargo( "VIP", 0 )
   tk.msg( title[2], text[4] )

   -- Update mission details
   misn_stage = 2
   misn.setMarker(retsys)
   misn.setDesc( string.format(misn_desc[2], ret:name(), retsys:name() ))
end


function death ()
   -- Notify of death
   player.msg( msg[1] )

   -- Update mission details
   misn_stage = 3
   --[[
   misn.setMarker(retsys)
   misn.setDesc( string.format(misn_desc[2], ret:name(), retsys:name() ))
   ]]--
   misn.finish(false)
end
