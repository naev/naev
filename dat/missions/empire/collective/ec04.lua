--[[

   Collective Extraction

   Author: bobbens
      minor edits by Infiltrator

   Fifth mission in the collective mini campaign.

   You must go rescue a team of commandos after being attacked by collective
    forces on a surveillance mission.

]]--

include "scripts/nextjump.lua"
include "scripts/proximity.lua"

lang = naev.lang()
if lang == "es" then
   -- not translated atm
else -- default english
   misn_title = "Collective Extraction"
   misn_reward = "None"
   misn_desc = {}
   misn_desc[1] = "Check for survivors on %s in %s."
   misn_desc[2] = "Travel back to %s in %s."
   title = {}
   title[1] = "Collective Extraction"
   title[2] = "Planet %s"
   title[3] = "Mission Accomplished"
   text = {}
   text[1] = [[    As soon as you exit the landing pad you see Lt. Commander Dimitri waiting for you. He seems a bit more nervous then usual.
    "The commando team has sent us an SOS. They were discovered by the Collective, and now they're under heavy fire. We need you to go and get them out of there. Would you be willing to embark on another dangerous mission?"]]
   text[2] = [[    "We'll send extra forces to %s to try to give you a chance to break through the blockade. You'll have to land on %s and extract our team. Be very careful. This is going to be no walk in the park."]]
   text[3] = [[    The atmosphere once again starts giving your shields a workout as you land. You spend a while flying low until your sensors pick up a reading of possible life forms. The silhouette of the transport ship is barely visible. As you fly closer, it becomes apparent that you arrived too late. Everyone is already dead. You see if you can salvage the readings from their equipment, but it seems like it's completely toasted.]]
   text[4] = [[    You notice you won't have enough fuel to get back so you salvage some from the wrecked transport ship. Stealing from the dead isn't pleasant business, but if it gets you out alive, you figure it's good enough.]]
   text[5] = [[    You spend a while searching until you find a datapad on one of the corpses. Ignoring the stench of burnt flesh you grab it, just as you hear the sirens go off in your ship. Enemy reinforcements! Time to hit the afterburner.
   You've got one, right?]]
   text[6] = [[    Lt. Commander Dimitri's face cannot hide his sadness as he sees you approach with no commando members.
    "No survivors, eh? I had that gut feeling. At least you were able to salvage something? Good, at least it'll mean they didn't die in vain. Meet me in the bar in a while. We're going to try to process this datapad. It'll hopefully have the final results."]]

    escort_msg1 = "Okay, %s, we'll flank the Collective force around the planet and try to draw their fire. You punch right through and land on that planet!"
    escort_msg2 = "There's too many of them! Fall back! Everyone to the jump point!"
    land_msg = "You can't land now! Get to the jump point!"
    markername = "Empire flanking maneuver"

    osd_msg = {}
    osd_msg[1] = "Fly to %s"
    osd_msg[2] = "Land on %s"
    osd_msg[3] = "Return to %s"
    osd_msg["__save"] = true 
end


function create ()
   misn_target, misn_target_sys = planet.get("Eiroik")

   local missys = {misn_target}
   if not misn.claim(missys) then
      abort()
   end

   -- Intro text
   if tk.yesno( title[1], string.format(text[1], misn_target:name()) ) then
      misn.accept()

      misn_stage = 0
      misn_base, misn_base_sys = planet.get("Omega Station")
      misn_marker = misn.markerAdd( misn_target_sys, "low" )

      -- Mission details
      misn.setTitle(misn_title)
      misn.setReward( misn_reward )
      misn.setDesc( string.format(misn_desc[1], misn_target:name(), misn_target_sys:name() ))
      tk.msg( title[1], string.format(text[2], misn_target_sys:name(), misn_target:name()) )
      osd_msg[1] = osd_msg[1]:format(misn_target_sys:name())
      osd_msg[2] = osd_msg[2]:format(misn_target:name())
      osd_msg[3] = osd_msg[3]:format(misn_base:name())
      misn.osdCreate(misn_title, osd_msg)

      hook.enter("enter")
      hook.land("land")
   end
end

-- Handles the Collective encounters.
function enter()
    player.allowSave() -- This mission disables saving, which is dangerous. Should be turned back on ASAP.
    if system.cur() == misn_target_sys and misn_stage == 0 then
        -- Case jumped in before landing
        pilot.clear()
        pilot.toggleSpawn(false)

        local fleetpos1 = vec2.new(20500, 2300)
        local fleetpos2 = vec2.new(20500, 1700)
        local waypoint1 = vec2.new(7500, 9500)
        local waypoint2 = vec2.new(7500, -5500)
        local waypoint12 = vec2.new(1500, 3000)
        local waypoint22 = vec2.new(1500, -500)

        fleet1 = pilot.add("Empire Flanking Fleet", "empire_norun", fleetpos1)
        fleet2 = pilot.add("Empire Flanking Fleet", "empire_norun", fleetpos2)
        empireAttack(fleet1)
        empireAttack(fleet2)

        fleet1[1]:comm(escort_msg1:format(player.name()))
        fleet1[1]:taskClear()
        fleet1[1]:goto(waypoint1, false, false)
        fleet1[1]:goto(waypoint12, false, false)
        fleet2[1]:taskClear()
        fleet2[1]:goto(waypoint2, false, false)
        fleet2[1]:goto(waypoint22, false, false)
        hook.pilot(fleet1[1], "idle", "idle")
        hook.pilot(fleet2[1], "idle", "idle")

        system.mrkAdd(markername, waypoint1)
        system.mrkAdd(markername, waypoint2)

        swarm1 = pilot.add("Collective Lge Swarm", nil, misn_target:pos())
        swarm2 = pilot.add("Collective Lge Swarm", nil, misn_target:pos())
        for _, j in ipairs(swarm2) do
            swarm1[#swarm1 + 1] = j -- Combine the swarms into one swarm, for convenience.
        end
        for _, j in ipairs(swarm1) do
            if j:exists() then
                j:control()
                j:setVisplayer()
            end
        end

        hook.timer(500, "proximity", {location = misn_target:pos(), radius = 3000, funcname = "idle"})

        misn_stage = 1
        misn.osdActive(2)
    elseif system.cur() == misn_target_sys and misn_stage == 2 then
        -- Case taken off from the planet
        pilot.clear()
        pilot.toggleSpawn(false)

        local pv = pilot.player():pos()

        fleet1 = pilot.add("Empire Flanking Fleet", nil, pv + vec2.new(-150, 500))
        fleet2 = pilot.add("Empire Flanking Fleet", nil, pv + vec2.new(-150, -500))
        empireRetreat(fleet1)
        empireRetreat(fleet2)
        fleet1[1]:comm(escort_msg2)

        -- TODO: Use heavier Collective ships here
        swarm1 = pilot.add("Collective Lge Swarm", nil, pv + vec2.new(-3000, 500))
        swarm2 = pilot.add("Collective Lge Swarm", nil, pv + vec2.new(-3000, 0))
        swarm3 = pilot.add("Collective Lge Swarm", nil, pv + vec2.new(-3000, -500))
        for i, _ in ipairs(swarm1) do -- Let's be lazy.
            swarm1[i]:setVisplayer()
            swarm2[i]:setVisplayer()
            swarm3[i]:setVisplayer()
        end

        player.allowLand(false, land_msg)
        misn.osdActive(3)
    elseif misn_stage == 1 then
        -- Case jumped back out without landing
        misn_stage = 0
        misn.osdActive(1)
    elseif misn_stage == 2 then
        -- Case jumped out after landing
        misn_stage = 3
    end
end

-- Preps the Empire ships for attack.
function empireAttack(fleet)
    for _, j in ipairs(fleet) do
        if j:exists() then
            j:control()
            j:setVisplayer()
            j:follow(fleet[1])
        end
    end
end

-- Makes the Empire ships run away.
function empireRetreat(fleet)
    for _, j in ipairs(fleet) do
        if j:exists() then
            j:control()
            j:setVisplayer()
            j:hyperspace(getNextSystem(system.cur(), misn_base_sys))
        end
    end
end

-- Triggered when either Empire fleet is in attack range.
function idle()
    for _, j in ipairs(swarm1) do
        if j:exists() then
            j:control(false)
        end
    end
    for _, j in ipairs(fleet1) do
        if j:exists() then
            j:control(false)
        end
    end
    for _, j in ipairs(fleet2) do
        if j:exists() then
            j:control(false)
        end
    end
end

-- Handles arrival back to base
function land ()
   -- Just landing
   if misn_stage == 1 and planet.cur() == misn_target then
      player.allowSave(false) -- This prevents the player from starting on Eiroik if he dies after taking off.
      player.takeoff()

      -- Some flavour text
      title[2] = title[2]:format(misn_target:name())
      tk.msg( title[2], text[3] )

      -- Add fuel if needed
      if player.fuel() < 200 then
         player.refuel(200)
         tk.msg( title[2], text[4] )
      end

      tk.msg( title[2], text[5] )

      -- Add goods
      misn_cargo = misn.cargoAdd( "Datapad", 0 )
      misn_stage = 2

   elseif misn_stage == 3 and planet.cur() == misn_base then

      tk.msg( title[3], text[6] )
      misn.cargoRm( misn_cargo )
      var.pop("emp_commando")

      -- Rewards
      faction.modPlayerSingle("Empire",5)

      misn.finish(true)
   end
end