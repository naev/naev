--[[

   Empire Shipping Prisoner Exchange

   Author: bobbens
      minor edits by Infiltrator

]]--

include "dat/scripts/numstring.lua"

bar_desc = _("You see an Empire Commander. He seems to have noticed you.")
misn_title = _("Prisoner Exchange")
misn_reward = _("%s credits")
misn_desc = {}
misn_desc[1] = _("Go to %s in the %s system to exchange prisoners with the FLF.")
misn_desc[2] = _("Return to %s in the %s system to report what happened.")
title = {}
title[1] = _("Spaceport Bar")
title[2] = _("Prisoner Exchange")
title[3] = _("Mission Report")
text = {}
text[1] = _([[You approach the Empire Commander.
    "Hello, you must be %s. I've heard about you. I'm Commander Soldner. We've got some harder missions for someone like you in the Empire Shipping division. There would be some real danger involved in these missions, unlike the ones you've recently completed for the division. Would you be up for the challenge?"]])
text[2] = _([["We've got a prisoner exchange set up with the FLF to take place on %s in the %s system. They want a more neutral pilot to do the exchange. You would have to go to %s with some FLF prisoners aboard your ship and exchange them for some of our own. You won't have visible escorts but we will have your movements watched by ships in nearby sectors."
    "Once you get the men they captured back, bring them over to %s in %s for debriefing. You'll be compensated for your troubles. Good luck."]])
text[3] = _([[The Prisoners are loaded onto your ship along with a few marines to ensure nothing untoward happens.]])
text[4] = _([[As you land, you notice the starport has been emptied. You also notice explosives rigged on some of the columns. This doesn't look good. The marines tell you to sit still while they go out to try to complete the prisoner exchange.
    From the cockpit you see how the marines lead the prisoners in front of them with guns to their backs. You see figures step out of the shadows with weapons too; most likely the FLF.]])
text[5] = _([[All of a sudden a siren blares and you hear shooting break out. You quickly start your engines and prepare for take off. Shots ring out all over the landing bay and you can see a couple of corpses as you leave the starport. You remember the explosives just as loud explosions go off behind you. This doesn't look good at all.
    You start your climb out of the atmosphere and notice how you're picking up many FLF and Dvaered ships. Looks like you're going to have quite a run to get the hell out of here. It didn't go as you expected.]])
text[6] = _([[After you leave your ship in the starport, you meet up with Commander Soldner. From the look on his face, it seems like he already knows what happened.
    "It was all the Dvaered's fault. They just came in out of nowhere and started shooting. What a horrible mess. We're already working on sorting out the blame."
    He sighs. "We had good men there. And we certainly didn't want you to start with a mess like this, but if you're interested in another, meet me up in the bar in a while. We get no rest around here. The payment has already been transfered to your bank account."]])


function create ()
   -- Note: this mission does not make any system claims.
   -- Target destination
   dest,destsys = planet.getLandable( faction.get("Frontier") )
   ret,retsys   = planet.getLandable( "Halir" )
   if dest == nil or ret == nil then
      misn.finish(false)
   end

   -- Spaceport bar stuff
   misn.setNPC( _("Commander"), "empire/unique/soldner" )
   misn.setDesc( bar_desc )
end


function accept ()

   -- Intro text
   if not tk.yesno( title[1], string.format( text[1], player.name() ) ) then
      misn.finish()
   end

   -- Accept mission
   misn.accept()

   -- target destination
   misn_marker = misn.markerAdd( destsys, "low" )

   -- Mission details
   misn_stage = 0
   reward = 500000
   misn.setTitle(misn_title)
   misn.setReward( string.format(misn_reward, numstring(reward)) )
   misn.setDesc( string.format(misn_desc[1], dest:name(), destsys:name()))

   -- Flavour text and mini-briefing
   tk.msg( title[2], string.format( text[2], dest:name(), destsys:name(),
         dest:name(), ret:name(), retsys:name() ))
   misn.osdCreate(title[2], {misn_desc[1]:format(dest:name(),destsys:name())})
   -- Set up the goal
   prisoners = misn.cargoAdd("Prisoners", 0)
   tk.msg( title[2], text[3] )

   -- Set hooks
   hook.land("land")
   hook.enter("enter")
end


function land ()
   landed = planet.cur()
   if landed == dest and misn_stage == 0 then
      if misn.cargoRm(prisoners) then
         -- Go on to next stage
         misn_stage = 1

         -- Some text
         tk.msg(title[2], text[4] )
         tk.msg(title[2], text[5] )
         misn.markerMove( misn_marker, retsys )
         misn.setDesc( string.format(misn_desc[2], ret:name(), retsys:name()))
         misn.osdCreate(title[2], {misn_desc[2]:format(ret:name(),retsys:name())})

         -- Prevent players from saving on the destination planet
         player.allowSave(false)

         -- We'll take off right away again
         player.takeoff()

         -- Saving should be disabled for as short a time as possible
         player.allowSave()
      end
   elseif landed == ret and misn_stage == 1 then

      -- Rewards
      player.pay(reward)
      faction.modPlayerSingle("Empire",5);

      -- Flavour text
      tk.msg(title[3], text[6] )

      misn.finish(true)
   end
end


function enter ()
   sys = system.cur()
   if misn_stage == 1 and sys == destsys then

      -- Get a random position near the player
      ang = rnd.rnd(0, 360)
      enter_vect = player.pos() + vec2.newP( rnd.rnd(1500, 2000), ang )

      -- Create some pilots to go after the player
      p = pilot.add( "FLF Sml Force", nil, enter_vect )
      -- Set hostile
      for k,v in ipairs(p) do
         v:setHostile()
      end

      -- Get a far away position for fighting to happen
      local battle_pos = player.pos() +
            vec2.newP( rnd.rnd(4000, 5000), ang + 180 )

      -- We'll put the FLF first
      enter_vect = battle_pos + vec2.newP( rnd.rnd(700, 1000), rnd.rnd(0, 360) )
      pilot.add( "FLF Med Force", nil, enter_vect )

      -- Now the Dvaered
      enter_vect = battle_pos + vec2.newP( rnd.rnd(200, 300), rnd.rnd(0, 360) )
      pilot.add( "Dvaered Med Force", nil, enter_vect )

      -- Player should not be able to reland
      player.allowLand(false,_("The docking stabilizers have been damaged by weapons fire!"))
   end
end

