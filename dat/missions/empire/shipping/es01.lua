--[[

   Empire Shipping Dangerous Cargo Delivery

   Author: bobbens
      minor edits by Infiltrator

]]--

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   -- Mission details
   bar_desc = "You see Commander Soldner who is expecting you."
   misn_title = "Empire Shipping Delivery"
   misn_reward = "%d credits"
   misn_desc = {}
   misn_desc[1] = "Pick up a package at %s in the %s system."
   misn_desc[2] = "Deliver the package to %s in the %s system." 
   misn_desc[3] = "Return to %s in the %s system."
   -- Fancy text messages
   title = {}
   title[1] = "Commander Soldner"
   title[2] = "Loading Cargo"
   title[3] = "Cargo Delivery"
   title[4] = "Mission Success"
   text = {}
   text[1] = [[You approach Commander Soldner, who seems to be waiting for you.
"Hello, ready for your next mission?"]]
   text[2] = [[Commander Soldner begins, "We have an important package that we must take from %s in the %s system to %s in the %s system. We have reason to believe that it is also wanted by external forces."
"The plan is to send an advance convoy with guards to make the run in an attempt to confuse possible enemies. You will then go in and do the actual delivery by yourself. This way we shouldn't arouse suspicion. You are to report here when you finish delivery and you'll be paid %d credits."]]
   text[3] = [["Avoid hostility at all costs. The package must arrive at its destination. Since you are undercover, Empire ships won't assist you if you come under fire, so stay sharp. Good luck."]]
   text[4] = [[The packages labelled "Food" are loaded discretely onto your ship. Now to deliver them to %s in the %s system.]]
   text[5] = [[Workers quickly unload the package as mysteriously as it was loaded. You notice that one of them gives you a note. Looks like you'll have to go to %s in the %s system to report to Commander Soldner.]]
   text[6] = [[You arrive at %s and report to Commander Soldner. He greets you and starts talking. "I heard you encountered resistance. At least you managed to deliver the package. Great work there. I've managed to get you cleared for the Heavy Weapon License. You'll still have to pay the fee for getting it, though.
"If you're interested in more work, meet me in the bar in a bit. I've got some paperwork I need to finish first."]]
   -- Errors
   errtitle = {}
   errtitle[1] = "Need More Space"
   err = {}
   err[1] = "You do not have enough space to load the packages. You need to make room for %d more tons."
end


function create ()
   -- Note: this mission does not make any system claims.
   misn.setNPC( "Soldner", "soldner" )
   misn.setDesc( bar_desc )
end

function accept ()

   -- See if accept mission
   if not tk.yesno( title[1], text[1] ) then
      misn.finish()
   end

   misn.accept()

   -- target destination
   pickup,pickupsys = planet.get( "Selphod" )
   dest,destsys = planet.get( "Cerberus" )
   ret,retsys = planet.get( "Polaris Prime" )
   misn_marker = misn.markerAdd( pickupsys, "low" )

   -- Mission details
   misn_stage = 0
   reward = 50000
   misn.setTitle(misn_title)
   misn.setReward( string.format(misn_reward, reward) )
   misn.setDesc( string.format(misn_desc[1], pickup:name(), pickupsys:name()))

   -- Flavour text and mini-briefing
   tk.msg( title[1], string.format( text[2], pickup:name(), pickupsys:name(),
         dest:name(), destsys:name(), reward ))

   -- Set up the goal
   tk.msg( title[1], text[3] )

   -- Set hooks
   hook.land("land")
   hook.enter("enter")
end


function land ()
   landed = planet.get()

   if landed == pickup and misn_stage == 0 then

      -- Make sure player has room.
      if pilot.cargoFree(player.pilot()) < 3 then
         tk.msg( errtitle[1], string.format( err[1], 3 - pilot.cargoFree(playerpilot()) ) )
         return
      end

      -- Update mission
      package = misn.cargoAdd("Packages", 3)
      misn_stage = 1
      jumped = 0
      misn.setDesc( string.format(misn_desc[2], dest:name(), destsys:name()))
      misn.markerMove( misn_marker, destsys )

      -- Load message
      tk.msg( title[2], string.format( text[4], dest:name(), destsys:name()) )

   elseif landed == dest and misn_stage == 1 then
      if misn.cargoRm(package) then

         -- Update mission
         misn_stage = 2
         misn.setDesc( string.format(misn_desc[3], ret:name(), retsys:name()))
         misn.markerMove( misn_marker, retsys )

         -- Some text
         tk.msg( title[3], string.format(text[5], ret:name(), retsys:name()) )

      end
   elseif landed == ret and misn_stage == 2 then

      -- Rewards
      player.pay(reward)
      player.modFaction("Empire",5);

      -- Flavour text
      tk.msg(title[4], string.format(text[6], ret:name()) )

      -- The goods
      diff.apply("heavy_weapons_license")

      misn.finish(true)
   end
end


function enter ()
   sys = system.cur()

   if misn_stage == 1 then

      -- Mercenaries appear after a couple of jumps
      jumped = jumped + 1
      if jumped <= 3 then
         return
      end

      -- Get player position
      enter_vect = player.pos()

      -- Calculate where the enemies will be
      r = rnd.rnd(0,4)
      -- Next to player (always if landed)
      if enter_vect:dist() < 1000 or r < 2 then
         a = rnd.rnd() * 2 * math.pi
         d = rnd.rnd( 400, 1000 )
         enter_vect:add( math.cos(a) * d, math.sin(a) * d )
         enemies()
      -- Enter after player
      else
         t = hook.timer(rnd.int( 2000, 5000 ) , "enemies")
      end
   end
end


function enemies ()
   -- Choose mercenaries
   merc = {}
   if rnd.rnd() < 0.3 then table.insert( merc, "Mercenary Pacifier" ) end
   if rnd.rnd() < 0.7 then table.insert( merc, "Mercenary Ancestor" ) end
   if rnd.rnd() < 0.9 then table.insert( merc, "Mercenary Vendetta" ) end

   -- Add mercenaries
   for k,v in ipairs(merc) do
      -- Move position a bit
      a = rnd.rnd() * 2 * math.pi
      d = rnd.rnd( 50, 75 )
      enter_vect:add( math.cos(a) * d, math.sin(a) * d )
      -- Add pilots
      p = pilot.add( v, "mercenary", enter_vect )
      -- Set hostile
      for k,v in ipairs(p) do
         v:setHostile()
      end
   end
end


