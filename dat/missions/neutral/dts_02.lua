--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Defend the System 3">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>3</chance>
  <done>Defend the System 2</done>
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

   MISSION: Defend the System 3
   DESCRIPTION: A mission to defend the system against swarm of pirate ships.
                This will be the third in a series of random encounters.
                After the mission, perhaps there'll be a regular diet of similar missions
                Perhaps the random missions will eventually lead on to a plot line relating to the pirates.

      Notable events:

         * Stage one: From the bar, the player learns of a pirate fleet attacking the system and joins a defense force.
         * Stage two: The volunteer force attacks the pirates.
         * Stage three: When a sufficient number have been killed, the pirates retreat.
         * Stage four: The portmaster welcomes the fleet back and thanks them with money.
         * Stage five: In the bar afterward, another pilot wonders why the pirates behaved unusually.

TO DO
Make comm chatter appear during the battle

NOTE
Because of bad planning, this mission is badly organized.
Anyone looking for a model of good mission-making should look elsewhere! -- the author
]]--

require "scripts/numstring.lua"

-- This section stores the strings (text) for the mission.

-- Mission details
misn_title = _("Defend the System")
misn_reward = _("%s and the pleasure of serving the Empire.")
misn_desc = _("Defend the system against a pirate fleet.")

-- Stage one: in the bar you hear a fleet of Pirates have invaded the system.
title = {}
text = {}
title[1] = _("In the bar")
text[1] = _([[The bar rests in the dull clink of glasses and the scattered murmur of conversation when the door bursts open. An older couple stumbles in, faces gaping, eyes staring. They take a few steps before the woman sinks to her knees and bursts into tears.
    "Our son... his ship was supposed to land a hectosecond ago," her partner says mechanically. "But pirates, suddenly everywhere-" he swallows. "-they didn't make it."  His wife throws her head back and wails.
    Two young men rise abruptly from a table in the back of the room and come stiffly forward. One goes to the grieving couple while the other turns address the room.
    "These raiders must be stopped. We are cadets at the Imperial Flight School. If you feel the injustice of this family's loss, will you fly with us to avenge their son's death?"]])
title[11] = _("Volunteers")
text[11] = _([["These terrorists cannot sustain many losses," one of the young men explains as you and a group of other volunteers prepare for takeoff, "and they have no organization. We can destroy them if you team up and focus your fire on one ship at a time."]])

-- Stage two: comm chatter, now no longer used except to initialize the table
comm = {}

-- Stage three: Victorious comm chatter
comm[6] = _("We've got them on the run!")
comm[61] = _("Broadcast %s> The raiders are retreating!")
comm[7] = _("Good flying, volunteers. The governor is waiting for us back in port.")
comm[71] = _("Comm %s> Good flying, volunteers. The governor is waiting for you back in port.")

-- Stage four: the governor greets you and the cadets in front of a crowd
title[2] = _("A public occasion")
text[2] = _([[Night is falling by the time you land back on %s. Looking solemn in front of a gathering crowd and news recorders, a large man with a fleshy face comes forward to greet the survivors of the fight. A flock of men and women follow him.
    When he shakes your hand, the Governor looks keenly at you at smiles, "Very well done."
    After meeting each surviving pilot, the tall man stands still for aide to attach an amplifier to his lapel. Then he turns his face to the news-casters and the crowd.]])

-- Stage five: the commander welcomes you back
title[3] = _("The Governor's speech")
text[3] = _([[
"Even here on %s, even in the protective embrace of civilization, we face many dangers. The ties that bind us through space to other worlds are fragile. When criminals attack these precious connections, they trouble the very foundations of our peace. How glad we are now for the security of the Empire whose young navy cadets led a team of independent pilots to defend us today."  The Governor turns to the pair of officers-in-training. "In the name of the Emperor, I have the privilege of decorating these two young heroes with the %s Silver Heart. I hope they and their volunteers will not be too proud to also accept a generous purse, along with the gratitude of all our people. Please join me in applauding their bravery."
    The public ceremony lasts only a few hectoseconds. Afterwards, as interviewers draw the young navy officers aside and the crowd disperses, you catch sight of the elderly couple from the bar holding each other and looking up into the darkening sky.]])

-- Other text for the mission
comm[8] = _("You fled from the battle. The Empire won't forget.")
comm[9] = _("Comm Lancelot> You're a coward, %s. You better hope I never see you again.")
comm[10] = _("Comm Lancelot> You're running away now, %s? The fight's finished, you know...")
title[4] = _("Good job!")
text[4] = _([[A freighter hails you as you jump into the system.
    "Thank you for responding, %s. Are you coming in from %s?  I have a delivery I need to get to %s and I can't wait much longer. Is the system safe now?"
    You relate the outcome of the space battle.
    "Oh, that's good news! You know, these raids are getting worse all the time. I wish the Empire would do something about it. Anyway, thank you for the information. Safe travels."]])
title[5] = _("Not fighting")
text[5] = _([[You stand by the grieving couple as the two cadets lead a group of pilots out of the bar toward the padfield.
    "Oh no!" the woman cries suddenly, looking up into her partner's face. "They're going off to fight. Those young men, they... there'll be more killing because of us."
    He nods grimly. "That's the way of things."
    For long time, the two of them sit on the floor of the bar holding each other.]])
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
         or this_system:presences()["FLF"]
         or this_system == system.get("Gamma Polaris")
         or this_system == system.get("Doeston")
         or this_system == system.get("NGC-7291") ) then
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
         pilot.add( "Trader Koala", "def", player.pos(), false)
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
      raider_fleet = pilot.add( "DTS Raiders", "def", raider_position )
      for k,v in ipairs( raider_fleet) do
         v:setHostile()
      end

  -- And a fleet of defending independents
      defense_fleet = pilot.add( "DTS Defense Fleet", "def", defense_position )
      cadet1 = pilot.add( "Empire Lancelot", "def", defense_position )[1]
      do 
         cadet1_alive = true
         hook.pilot( cadet1, "death", "cadet1_dead")
      end
      cadet2 = pilot.add( "Empire Lancelot", "def", defense_position )[1]
      do
         cadet2_alive = true
         hook.pilot( cadet2, "death", "cadet2_dead")
      end
      for k,v in ipairs( defense_fleet) do
         v:setFriendly()
      end
      cadet1:setFriendly()
      cadet2:setFriendly()

  --[[ Set conditions for the end of the Battle:
    hook fleet departure to disabling or killing ships]]
      casualties = 0
      casualty_max = (#raider_fleet / 2 + 1)
      victories = 0
      victory = false
      for k, v in ipairs( raider_fleet) do
         hook.pilot (v, "disable", "add_cas_and_check")
      end

      if defender == false then
         misn.finish( true)
      end

end

-- If they die, they can't communicate with the player.
function cadet1_dead() cadet1_alive = false end
function cadet2_dead() cadet2_alive = false end

-- Record each raider death and make the raiders flee after too many casualties
function add_cas_and_check()

      if victory then return end
      casualties = casualties + 1
      if casualties > casualty_max then

         raiders_left = pilot.get( { faction.get("Raider") } )
         for k, v in ipairs( raiders_left ) do
            v:changeAI("flee")
         end
         if victories < 1 then  -- Send in the second wave
            victories = victories + 1
            second_wave_attacks()
         else
            victory = true
            player.msg( comm[6])  -- A few seconds after victory, the system is back under control
            hook.timer(8000, "victorious")
            return
         end
      end

end

function second_wave_attacks()

      casualties = 0
      second_wave = pilot.add( "Pirate Hyena Pack", "def", player.pos(), true)
      for k, v in ipairs( second_wave) do
         v:setFaction( "Raider")
         v:setHostile()
      end
      casualty_max = math.modf( #second_wave / 2)
      for k, v in ipairs( second_wave) do
         hook.pilot (v, "disable", "add_cas_and_check")
      end

end

-- Separate mission for a mid-mission interjection <-- bad organization
function cadet_first_comm()

      if cadet1_alive then
         cadet1:comm( comm[6])
      elseif cadet2_alive then
         cadet2:comm( comm[6])
      else player.msg( string.format(comm[61], planet_name))
      end

end


-- When the raiders are on the run then the Empire takes over
function victorious()

      if cadet1_alive then
         cadet1:comm( comm[7])
      elseif cadet2_alive then
         cadet2:comm( comm[7])
      else player.msg( string.format(comm[71], planet_name))
      end

end

-- The player lands to a warm welcome (if the job is done).
function celebrate_victory()

      if victory == true then
         tk.msg( title[2], string.format( text[2], planet_name) )
         player.pay( reward)
         faction.modPlayerSingle( "Empire", 3)
         tk.msg( title[3], string.format( text[3], planet_name, planet_name) )
         misn.finish( true)
      else
         tk.msg( bounce_title, bounce_text)   -- If any pirates still alive, send player back out.
         player.takeoff()
      end

end

-- A fellow warrior says hello in passing if player jumps out of the system without landing
function ship_enters()

      enter_vect = player.pos()
      hook.timer(1000, "congratulations")
end
function congratulations()
      tk.msg( title[4], string.format( text[4], player.ship(), system_name, planet_name))
      misn.finish( true)

end

-- If the player aborts the mission, the Empire and Traders react
function abort()

      if victory == false then
         faction.modPlayerSingle( "Empire", -10)
         faction.modPlayerSingle( "Trader", -10)
         player.msg( string.format( comm[9], player.name()))
      else
         player.msg( string.format( comm[10], player.name()))
      end
      misn.finish( true)

end
