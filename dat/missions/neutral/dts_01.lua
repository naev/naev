--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Defend the System 2">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>4</priority>
   <chance>3</chance>
   <done>Defend the System 1</done>
   <location>None</location>
   <faction>Dvaered</faction>
   <faction>Frontier</faction>
   <faction>Goddard</faction>
   <faction>Independent</faction>
   <faction>Soromid</faction>
  </avail>
 </mission>
 --]]
--[[

   MISSION: Defend the System 2
   DESCRIPTION: A mission to defend the system against swarm of pirate ships.
                This will be the second in a planned series of random encounters.
                After the third mission, perhaps there'll be a regular diet of similar missions
                Perhaps the random missions will eventually lead on to a plot line relating to the pirates.

      Notable events:

         * Stage one: From the bar, the player learns of a pirate fleet attacking the system and joins a defense force.
         * Stage two: The volunteer force attacks the pirates.
         * Stage three: When a sufficient number have been killed, the pirates retreat.
         * Stage four: The portmaster welcomes the fleet back and thanks them with money.
         * Stage five: In the bar afterward, another pilot wonders why the pirates behaved unusually.

TO DO
Make comm chatter appear during the battle
Add some consequences if the player aborts the mission
]]--

require "scripts/numstring"

-- This section stores the strings (text) for the mission.

-- Mission details
misn_title = _("Defend the System")
misn_reward = _("%s and the pleasure of serving the Empire.")
misn_desc = _("Defend the system against a pirate fleet.")

-- Stage one: in the bar you hear a fleet of Pirates have invaded the system.
title = {}
text = {}
title[1] = _("In the bar")
text[1] = _([[The barman has just asked you for your order when the portmaster bursts though the door, out of breath. "Pirates, all over the system!  The navy's on maneuvers. Quickly, we need to organize a defense."
    All the pilots in the room scramble to their feet. "How many are there?" someone asks. "How long have they been in system?" another calls out.
    Into the confusion steps a steely-haired, upright, uniformed figure. Her stripes mark her as a navy Commodore.
    "I'm with the navy and I will organize the defense," her voice cuts through the commotion. "Who here is a pilot?  We must strike back quickly. I will arrange a reward for everyone who volunteers. We'll need as many pilots as possible. Follow me."]])
title[11] = _("Volunteers")
text[11] = _([["Take as many out of the fight early as you can," advises the Commodore before you board your ships. "If you can't chase them off, you might at least improve the odds. Good luck."]])

-- Stage two: Vicious comm chatter
comm = {}
comm[1] = _("Eat vacuum, scum!")
comm[2] = _("Die, pirate, die.")
comm[3] = _("Eat cannon fire")
comm[4] = _("Thieving parasites")
comm[5] = _("I've got one on me!")

-- Stage three: Victorious comm chatter
comm[6] = _("We've got them on the run!")
comm[7] = _("Well done, pilots. Return to port.")

-- Stage four: the commander is getting instructions when you land
title[2] = _("On the way in")
text[2] = _([[As you taxi in to land, you can make out the tiny figure of the Commodore saluting a small group of individuals to the side of the landing pads. After you and your fellow volunteers alight, she greets you with the portmaster by her side.]])

-- Stage five: the commander welcomes you back
title[3] = _("Thank you")
text[3] = _([["That was good flying," the Commodore says with a tight smile. "Thank you all for your help. This gentleman has arranged a transfer of forty thousand credits to each of you. You can be proud of what you've done today."]])

-- Other text for the mission
comm[8] = _("You fled from the battle. The Empire won't forget.")
comm[9] = _("Comm Trader>You're a coward, %s. You better hope I never see you again.")
comm[10] = _("Comm Trader>You're running away now, %s? The fight's finished, you know...")
title[4] = _("Good job!")
text[4] = _([[The debris from the battle disappears behind you in a blur of light. A moment after you emerge from hyperspace, a Imperial ship jumps in behind you and hails you.
    "Please hold course and confirm your identity, %s."  You send your license code and wait for a moment. "OK, that's fine. We're just making sure no pirates escaped. You were part of the battle, weren't you?  Surprised you didn't return for the bounty, pilot. Listen, I appreciate what you did back there. I have family on %s. When I'm not flying overhead, it's good to know there are good Samaritans like you who will step up. Thanks."
]])
title[5] = _("Left behind")
text[5] = _([[The Commodore turns and walks off. Eight men and women follow her, but you stay put.
    A man in a jumpsuit at the next table nods at you. "What, they expect me to do their dirty work for them?" he shakes his head. "It's going to be a hot ride out of the system though, with all that going on upstairs."]])
bounce_title = _("Not done yet.")
bounce_text = _("The system isn't safe yet. Get back out there!")
noReward = _("No reward for you.")
noDesc = _("Watch others defend the system.")
noTitle = _("Observe the action.")


-- Create the mission on the current planet, and present the first Bar text.
function create()

   this_planet, this_system = planet.cur()
   if ( this_system:presences()["Pirate"]
         or this_system:presences()["Collective"]
         or this_system:presences()["FLF"] ) then
      misn.finish(false)
   end

   missys = {this_system}
   if not misn.claim(missys) then
      misn.finish(false)
   end

   planet_name = this_planet:name()
   system_name = this_system:name()
   if tk.yesno( title[1], text[1] ) then
      misn.accept()
      var.push( "dts_firstSystem", "planet_name")
      tk.msg( title[11], text[11])
      reward = 40000
      misn.setReward( string.format( misn_reward, creditstring(reward)) )
      misn.setDesc( misn_desc)
      misn.setTitle( misn_title)
      misn.markerAdd( this_system, "low" )
      defender = true

      -- hook an abstract deciding function to player entering a system
      hook.enter( "enter_system")

      -- hook warm reception to player landing
      hook.land( "celebrate_victory")
   else
      -- If player didn't accept the mission, the battle's still on, but player has no stake.
      misn.accept()
      var.push( "dts_firstSystem", "planet_name")
      tk.msg( title[5], text[5])
      misn.setReward( noReward)
      misn.setDesc( noDesc)
      misn.setTitle( noTitle)
      defender = false

      -- hook an abstract deciding function to player entering a system when not part of defense
      hook.enter( "enter_system")
   end
end

-- Decides what to do when player either takes off starting planet or jumps into another system
function enter_system()

      if this_system == system.cur() and defender == true then
         defend_system()
      elseif victory == true and defender == true then
         hook.timer(1000, "ship_enters")
      elseif defender == true then
         player.msg( comm[8])
         faction.modPlayerSingle( "Empire", -3)
         misn.finish( true)
      elseif this_system == system.cur() and been_here_before ~= true then
         been_here_before = true
         defend_system()
      else
         misn.finish( true)
      end
end

-- There's a battle to defend the system
function defend_system()

  -- Makes the system empty except for the two fleets. No help coming.
      pilot.clear ()
      pilot.toggleSpawn( false )

  -- Set up distances
      angle = rnd.rnd() * 2 * math.pi
      if defender == true then
         raider_position  = vec2.new( 400*math.cos(angle), 400*math.sin(angle) )
         defense_position = vec2.new( 0, 0 )
      else
         raider_position  = vec2.new( 800*math.cos(angle), 800*math.sin(angle) )
         defense_position = vec2.new( 400*math.cos(angle), 400*math.sin(angle) )
      end

  -- Create a fleet of raiding pirates
      raider_fleet = pilot.add( "DTS Raiders", raider_position, "def" )
      for k,v in ipairs( raider_fleet) do
         v:setHostile()
      end

  -- And a fleet of defending independents
      defense_fleet = pilot.add( "DTS Defense Fleet", defense_position, "def" )
      for k,v in ipairs( defense_fleet) do
         v:setFriendly()
      end

  --[[ Set conditions for the end of the Battle:
    hook fleet departure to disabling or killing ships]]
      casualties = 0
      for k, v in ipairs( raider_fleet) do
         hook.pilot (v, "death", "add_cas_and_check")
         hook.pilot (v, "disable", "add_cas_and_check")
      end

      if defender == false then
         misn.finish( true)
      end

end

-- Record each raider death and make the raiders flee after too many casualties
function add_cas_and_check()

      casualties = casualties + 1
      if casualties > 9 then

         raiders_left = pilot.get( { faction.get("Raider") } )
         for k, v in ipairs( raiders_left ) do
            v:changeAI("flee")
         end
         if victory ~= true then  -- A few seconds after victory, the system is back under control
            victory = true
            player.msg( comm[6])
            hook.timer(8000, "victorious")
         end
      end

end

-- When the raiders are on the run then the Empire takes over
function victorious()

   -- Call ships to base
      player.msg( comm[7])
   -- Get a position near the player for late Empire re-enforcements
      starting_vect = player.pos()
      a = rnd.rnd() * 2 * math.pi
      d = rnd.rnd( 100, 200 )
      empire_vect = starting_vect:add( math.cos(a) * d, math.sin(a) * d )
      pilot.add( "Empire Med Attack", empire_vect, "def" )

end

-- The player lands to a warm welcome (if the job is done).
function celebrate_victory()

      if victory == true then
         tk.msg( title[2], text[2] )
         player.pay( reward)
         faction.modPlayerSingle( "Empire", 3)
         tk.msg( title[3], text[3] )
         misn.finish( true)
      else
         tk.msg( bounce_title, bounce_text)   -- If any pirates still alive, send player back out.
         player.takeoff()
      end

end

-- A fellow warrior says hello in passing if player jumps out of the system without landing
function ship_enters()
      enter_vect = player.pos()
      pilot.add("Empire Pacifier", enter_vect:add( 10, 10), "def")
      hook.timer(1000, "congratulations")
end
function congratulations()
      tk.msg( title[4], string.format( text[4], player.ship(), planet_name))
      misn.finish( true)

end

function abort()

      if victory ~= true then
         faction.modPlayerSingle( "Empire", -10)
         faction.modPlayerSingle( "Trader", -10)
         player.msg( string.format( comm[9], player.name()))
      else
         player.msg( string.format( comm[10], player.name()))
      end

end
