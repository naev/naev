--[[

   Empire Shipping Prisoner Exchange

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_title = "Prisoner Exchange"
   misn_reward = "%d credits"
   misn_desc = {}
   misn_desc[1] = "Go to %s in the %s system to exchange prisoners with the FLF."
   misn_desc[2] = "Return to %s in the %s system to report what happened."
   title = {}
   title[1] = "Spaceport Bar"
   title[2] = "Prisoner Exchange"
   title[3] = "Mission Report"
   text = {}
   text[1] = [[You are approached by an Empire Commander.
"Hello, you must be %s.  I've heard about you.  I'm Commander Soldner.  We've got some harder missions for someone like you in the Empire Shipping division.  There would be real danger involved in these missions unlike the ones you've been doing recently for the division.  Would you be up for the challenge?"]]
   text[2] = [["We've got a prisoner exchange set up with the FLF to take place on %s in the %s system.  They want a more neutral pilot to do the exchange.  You would have to go to %s with some FLF prisoners aboard your ship and exchange them for some of our own.  You won't have visible escorts but we will have your movements watched by ships in nearby sectors."
"Once we get the men they captured back bring them over to %s in %s for debriefing. You'll be compensated for your troubles.  Good luck."]]
   text[3] = [[The Prisoners are loaded on your ship along with a few marines to ensure nothing happens.]]
   text[4] = [[As you land you notice the starport has been emptied.  You notice some explosives rigged on some of the columns.  This doesn't look good.  The marines tell you to sit still and go out to try to do the prisoner exchange.
From the cockpit you see how the marines lead the prisoners in front of them with their guns to their back.  You notice that some people step out of the shadows with weapons too, most likely the FLF.]]
   text[5] = [[You suddenly hear a blaring siren and hear shooting.  You quickly start your engines and prepare for take off.  You see shots all over and a couple of prisoner corpses as you leave the starport.  As you remember the explosives you hear loud detonations behind you.  This doesn't look to good.
You start your climb out of the atmosphere and notice how you're picking up many FLF and Dvaered ships.  Looks like you're going to have quite a run to get the hell out of here.  It didn't go as you expected.]]
   text[6] = [[After you leave your ship in the starport you meet up with Commander Soldner.  From the look on his face it seems like he already knows about what happened.
"It was all the  Dvaered's fault.  They just came in out of no where and started shooting.  What a horrible mess.  We're already working on sorting out the blame, we had good men there."
He sighs, "Didn't want you to start with a mess like this, but if you're interested in another meet me up in the bar in a while.  We get no rest here.  We already transfered the payment to your bank account."]]
end


function create ()

   -- Intro text
   if tk.yesno( title[1], string.format( text[1], player.name() ) )
      then
      misn.accept()

      -- target destination
      dest,destsys = planet.get( faction.get("Frontier") )
      ret,retsys = planet.get( "Polaris Prime" )
      misn.setMarker(destsys)

      -- Mission details
      misn_stage = 0
      reward = 50000
      misn.setTitle(misn_title)
      misn.setReward( string.format(misn_reward, reward) )
      misn.setDesc( string.format(misn_desc[1], dest:name(), destsys:name()))

      -- Flavour text and mini-briefing
      tk.msg( title[2], string.format( text[2], dest:name(), destsys:name(),
            dest:name(), ret:name(), retsys:name() ))

      -- Set up the goal
      prisoners = misn.addCargo("Prisoners", 0)
      tk.msg( title[2], text[3] )

      -- Set hooks
      hook.land("land")
      hook.enter("enter")
   end
end


function land ()
   landed = planet.get()
   if landed == dest and misn_stage == 0 then
      if misn.rmCargo(prisoners) then
         -- Go on to next stage
         misn_stage = 1

         -- Some text
         tk.msg(title[2], text[4] )
         tk.msg(title[2], text[5] )
         misn.setMarker(retsys)
         misn.setDesc( string.format(misn_desc[2], ret:name(), retsys:name()))

         -- We'll take off right away again
         misn.takeoff()
      end
   elseif landed == ret and misn_stage == 1 then

      -- Rewards
      player.pay(reward)
      player.modFaction("Empire",5);

      -- Flavour text
      tk.msg(title[3], text[6] )

      misn.finish(true)
   end
end


function enter ()
   sys = system.get()
   if misn_stage == 1 and sys == destsys then

      -- Get a position near the player
      enter_vect = player.pos()
      a = rnd.rnd() * 2 * math.pi
      d = rnd.rnd( 100, 200 )
      enter_vect:add( math.cos(a) * d, math.sin(a) * d )

      -- Create some pilots to go after the player
      p = pilot.add( "FLF Sml Force", "def", enter_vect )
      -- Set hostile
      for k,v in ipairs(p) do
         v:setHostile()
      end

      -- Get a far away position for fighting to happen
      -- We'll put the FLF first
      a = rnd.rnd() * 2 * math.pi
      d = rnd.rnd( 700, 1000 )
      enter_vect:set( math.cos(a) * d, math.sin(a) * d )
      pilot.add( "FLF Med Force", "def", enter_vect )
      -- Now the Dvaered
      a = rnd.rnd() * 2 * math.pi
      d = rnd.rnd( 200, 300 )
      enter_vect:add( math.cos(a) * d, math.sin(a) * d )
      pilot.add( "Dvaered Med Force", "def", enter_vect )
   end
end

