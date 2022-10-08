--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Defend the System 3">
 <unique />
 <priority>4</priority>
 <chance>3</chance>
 <done>Defend the System 2</done>
 <location>None</location>
 <faction>Dvaered</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Soromid</faction>
</mission>
--]]
--[[

   MISSION: Defend the System 3
   DESCRIPTION: A mission to defend the system against swarm of pirate ships.
                This will be the third in a series of random encounters.
                After the mission, perhaps there'll be a regular diet of similar missions
                Perhaps the random missions will eventually lead on to a plot line relating to the pirates.

      Notable events:

         * Stage one: From the bar, the player learns of a pirate fleet attacking the system and joins a defence force.
         * Stage two: The volunteer force attacks the pirates.
         * Stage three: When a sufficient number have been killed, the pirates retreat.
         * Stage four: The portmaster welcomes the fleet back and thanks them with money.
         * Stage five: In the bar afterward, another pilot wonders why the pirates behaved unusually.

NOTE
Because of bad planning, this mission is badly organized.
Anyone looking for a model of good mission-making should look elsewhere! -- the author
]]--
local fleet = require "fleet"
local fmt = require "format"
local pir = require "common.pirate"

local reward = 400e3

local cadet1, cadet2, defense_fleet, fraider, raider_fleet, raiders_left, second_wave -- Non-persistent state
local defend_system, second_wave_attacks -- Forward-declared functions

-- Create the mission on the current planet, and present the first Bar text.
function create()
   mem.this_planet, mem.this_system = spob.cur()
   if ( pir.systemPresence(mem.this_system) > 0
         or mem.this_system:presences()["Collective"]
         or mem.this_system:presences()["FLF"]
         or mem.this_system == system.get("Gamma Polaris")
         or mem.this_system == system.get("Doeston")
         or mem.this_system == system.get("NGC-7291") ) then
      misn.finish(false)
   end

   local missys = {mem.this_system}
   if not misn.claim(missys) then
      misn.finish(false)
   end

   if tk.yesno( _("In the bar"), _([[The dull clink of glasses and the scattered murmur of conversation drifts through the bar until the door bursts open. An older couple stumbles in, faces gaping, eyes staring. They take a few steps before the woman sinks to her knees and bursts into tears.
    "Our son... his ship was supposed to land a hectosecond ago," her partner says mechanically. "But pirates, suddenly everywhere-" he swallows. "-they didn't make it."  His wife throws her head back and wails.
    Two young men rise abruptly from a table in the back of the room and come stiffly forward. One goes to the grieving couple while the other turns address the room.
    "These raiders must be stopped. We are cadets at the Imperial Flight School. If you feel the injustice of this family's loss, will you fly with us to avenge their son's death?"]]) ) then
      misn.accept()
      tk.msg( _("Volunteers"), _([["These terrorists cannot sustain many losses," one of the young men explains as you and a group of other volunteers prepare for takeoff, "and they have no organization. We can destroy them if you team up and focus your fire on one ship at a time."]]))
      misn.setReward( fmt.f( _("{credits} and the pleasure of serving the Empire."), {credits=fmt.credits(reward)}) )
      misn.setDesc( _("Defend the system against a pirate fleet."))
      misn.setTitle( _("Defend the System"))
      misn.markerAdd( mem.this_system, "low" )
      mem.defender = true

  -- hook an abstract deciding function to player entering a system
      hook.enter( "enter_system")

  -- hook warm reception to player landing
      hook.land( "celebrate_victory")

   else
  -- If player didn't accept the mission, the battle's still on, but player has no stake.
      misn.accept()
      tk.msg( _("Not fighting"), _([[You stand by the grieving couple as the two cadets lead a group of pilots out of the bar toward the padfield.
    "Oh no!" the woman cries suddenly, looking up into her partner's face. "They're going off to fight. Those young men, they... there'll be more killing because of us."
    He nods grimly. "That's the way of things."
    For long time, the two of them sit on the floor of the bar holding each other.]]))
      misn.setReward( _("No reward for you."))
      misn.setDesc( _("Watch others defend the system."))
      misn.setTitle( _("Observe the action."))
      mem.defender = false

  -- hook an abstract deciding function to player entering a system when not part of defence
      hook.enter( "enter_system")
   end
end

-- Decides what to do when player either takes off starting planet or jumps into another system
function enter_system()

      if mem.this_system == system.cur() and mem.defender == true then
         defend_system()
      elseif mem.victory == true and mem.defender == true then
         pilot.add( "Koala", "Trader", player.pos(), _("Trader Koala"), {ai="def"} )
         hook.timer(1.0, "ship_enters")
      elseif mem.defender == true then
         player.msg( _("You fled from the battle. The Empire won't forget.") )
         faction.modPlayerSingle( "Empire", -3)
         misn.finish( true)
      elseif mem.this_system == system.cur() and mem.been_here_before ~= true then
         mem.been_here_before = true
         defend_system()
      else
         misn.finish( true)
      end
end

-- There's a battle to defend the system
function defend_system()
   fraider = faction.dynAdd( "Pirate", "Raider", _("Raider") )

  -- Makes the system empty except for the two fleets. No help coming.
      pilot.clear ()
      pilot.toggleSpawn( false )

  -- Set up distances
      local angle, defense_position, raider_position
      angle = rnd.angle()
      if mem.defender == true then
         raider_position  = vec2.newP( 400, angle )
         defense_position = vec2.new( 0, 0 )
      else
         raider_position  = vec2.newP( 800, angle )
         defense_position = vec2.newP( 400, angle )
      end

  -- Create a fleet of raiding pirates
      raider_fleet = fleet.add( 18, "Pirate Hyena", fraider, raider_position, _("Raider Hyena"), {ai="def"} )
      for k,v in ipairs( raider_fleet) do
         v:setHostile()
      end

  -- And a fleet of defending independents
      local dfleet = { "Mule", "Lancelot", "Ancestor", "Gawain" }
      defense_fleet = fleet.add( 2, dfleet, "Trader", defense_position, _("Defender"), {ai="def"} )
      cadet1 = pilot.add( "Empire Lancelot", "Empire", defense_position, nil, {ai="def"} )
      do
         mem.cadet1_alive = true
         hook.pilot( cadet1, "death", "cadet1_dead")
      end
      cadet2 = pilot.add( "Empire Lancelot", "Empire", defense_position, nil, {ai="def"} )
      do
         mem.cadet2_alive = true
         hook.pilot( cadet2, "death", "cadet2_dead")
      end
      for k,v in ipairs( defense_fleet) do
         v:setFriendly()
      end
      cadet1:setFriendly()
      cadet2:setFriendly()

  --[[ Set conditions for the end of the Battle:
    hook fleet departure to disabling or killing ships]]
      mem.casualties = 0
      mem.casualty_max = (#raider_fleet / 2 + 1)
      mem.victories = 0
      mem.victory = false
      for k, v in ipairs( raider_fleet) do
         hook.pilot (v, "disable", "add_cas_and_check")
      end

      if mem.defender == false then
         misn.finish( true)
      end

end

-- If they die, they can't communicate with the player.
function cadet1_dead() mem.cadet1_alive = false end
function cadet2_dead() mem.cadet2_alive = false end

-- Record each raider death and make the raiders flee after too many casualties
function add_cas_and_check()

      if mem.victory then return end
      mem.casualties = mem.casualties + 1
      if mem.casualties > mem.casualty_max then

         raiders_left = pilot.get( { faction.get(fraider) } )
         for k, v in ipairs( raiders_left ) do
            v:changeAI("flee")
         end
         if mem.victories < 1 then  -- Send in the second wave
            mem.victories = mem.victories + 1
            second_wave_attacks()
         else
            mem.victory = true
            player.msg( _("We've got them on the run!") )  -- A few seconds after victory, the system is back under control
            hook.timer(8.0, "victorious")
            return
         end
      end

end

function second_wave_attacks()
      mem.casualties = 0
      second_wave = fleet.add( 4, "Pirate Hyena", fraider, player.pos(), nil, {ai="def"} )
      for k, v in ipairs( second_wave) do
         v:setHostile()
      end
      mem.casualty_max = math.modf( #second_wave / 2)
      for k, v in ipairs( second_wave) do
         hook.pilot (v, "disable", "add_cas_and_check")
      end
end

--[[
-- Separate mission for a mid-mission interjection <-- bad organization <-- FIXME: it's worse, nothing even refers to this.
local function cadet_first_comm()
      if mem.cadet1_alive then
         cadet1:comm( _("We've got them on the run!") )
      elseif mem.cadet2_alive then
         cadet2:comm( _("We've got them on the run!") )
      else player.msg( fmt.f(_("Broadcast {pnt}> The raiders are retreating!"), {pnt=mem.this_planet}))
      end
end
--]]


-- When the raiders are on the run then the Empire takes over
function victorious()
      if mem.cadet1_alive then
         cadet1:comm( _("Good flying, volunteers. The governor is waiting for us back in port.") )
      elseif mem.cadet2_alive then
         cadet2:comm( _("Good flying, volunteers. The governor is waiting for us back in port.") )
      else player.msg( fmt.f(_("Comm {pnt}> Good flying, volunteers. The governor is waiting for you back in port."), {pnt=mem.this_planet}))
      end
end

-- The player lands to a warm welcome (if the job is done).
function celebrate_victory()
      if mem.victory == true then
         tk.msg( _("A public occasion"), fmt.f( _([[Night is falling by the time you land back on {pnt}. Looking solemn in front of a gathering crowd and news recorders, a large man with a fleshy face comes forward to greet the survivors of the fight. A flock of men and women follow him.
    When he shakes your hand, the Governor looks keenly at you at smiles, "Very well done."
    After meeting each surviving pilot, the tall man stands still for aide to attach an microphone to his lapel. Then he turns to the news-casters and the crowd.]]), {pnt=mem.this_planet}) )
         player.pay( reward)
         faction.modPlayerSingle( "Empire", 3)
         tk.msg( _("The Governor's speech"), fmt.f( _([[
"Even here on {pnt}, even in the protective embrace of civilization, we face many dangers. The ties that bind us through space to other worlds are fragile. When criminals attack these precious connections, they threaten the very foundations of our peace. We are grateful for the security of the Empire whose young navy cadets led a team of independent pilots to defend us today."  The Governor turns to the pair of officers-in-training. "In the name of the Emperor, I have the privilege of decorating these two young heroes with the {pnt} Silver Heart. I hope they, and their volunteers, will not be too proud to also accept a generous purse, along with the gratitude of all our people. Please join me in applauding their bravery."
    The public ceremony lasts only a few hectoseconds. Afterwards, as interviewers draw the young navy officers aside and the crowd disperses, you catch sight of the elderly couple from the bar holding each other and looking up into the darkening sky.]]), {pnt=mem.this_planet} ) )
         misn.finish( true)
      else
         tk.msg( _("Not done yet."), _("The system isn't safe yet. Get back out there!"))   -- If any pirates still alive, send player back out.
         player.takeoff()
      end

end

-- A fellow warrior says hello in passing if player jumps out of the system without landing
function ship_enters()
      hook.timer(1.0, "congratulations")
end

function congratulations()
      tk.msg( _("Good job!"), fmt.f( _([[A freighter hails you as you jump into the system.
    "Thank you for responding, {ship}. Are you coming in from {sys}?  I have a delivery I need to get to {pnt} and I can't wait much longer. Is the system safe now?"
    You relate the outcome of the space battle.
    "Oh, that's good news! You know, these raids are getting worse all the time. I wish the Empire would do something about it. Anyway, thank you for the information. Safe travels."]]), {ship=player.ship(), sys=mem.this_system, pnt=mem.this_planet} ) )
      misn.finish( true)

end

-- If the player aborts the mission, the Empire and Traders react
function abort()
      if mem.victory == false then
         faction.modPlayerSingle( "Empire", -10)
         faction.modPlayerSingle( "Trader", -10)
         player.msg( fmt.f( _("Comm Lancelot> You're a coward, {player}. You better hope I never see you again."), {player=player.name()} ) )
      else
         player.msg( fmt.f( _("Comm Lancelot> You're running away now, {player}? The fight's finished, you know..."), {player=player.name()} ) )
      end
      misn.finish( true)
end
