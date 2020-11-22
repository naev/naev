--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Operation Cold Metal">
  <flags>
   <unique />
  </flags>
  <avail>
   <priority>2</priority>
   <cond>faction.playerStanding("Empire") &gt; 5 and var.peek("collective_fail") ~= true</cond>
   <done>Operation Black Trinity</done>
   <chance>100</chance>
   <location>Bar</location>
   <planet>Omega Station</planet>
  </avail>
  <notes>
   <provides name="The Collective is dead and no one will miss them"/>
   <campaign>Collective</campaign>
  </notes>
 </mission>
 --]]
--[[

   Operation Cold Metal

   Author: bobbens
      minor edits by Infiltrator

   Seventh and final mission in the Collective Campaign

]]--

require "proximity.lua"
require "fleethelper.lua"
require "missions/empire/common.lua"

bar_desc = _("You see Commodore Keer at a table with a couple of other pilots. She motions for you to sit down with them.")
misn_title = _("Operation Cold Metal")
misn_reward = _("Fame and Glory")
misn_desc = {}
misn_desc[1] = _("Neutralize enemy forces in %s")
misn_desc[2] = _("Destroy the Starfire and hostiles in %s")
misn_desc[3] = _("Return to %s in the %s system")
title = {}
title[1] = _("Bar")
title[2] = _("Operation Cold Metal")
title[3] = _("Mission Success")
coward_title = _("Cowardly Behavior")
text = {}
text[1] = _([[You join Commodore Keer at her table.
    She begins, "We're going to finally attack the Collective. We've gotten the Emperor himself to bless the mission and send some of his better pilots. Would you be interested in joining the destruction of the Collective?"]])
text[2] = _([["The Operation has been dubbed 'Cold Metal'. We're going to mount an all-out offensive in C-00. The systems up to %s are already secure and under our control, all we need to do now is to take the final stronghold. Should we encounter the Starfire at any stage our goal will be to destroy it and head back. The Imperial fleet will join you when you get there. See you in combat, pilot."]])
text[3] = _([[As you do your approach to land on %s you notice big banners placed on the exterior of the station. They seem to be in celebration of the final defeat of the Collective. When you do land you are saluted by the welcoming committee in charge of saluting all the returning pilots.
    You notice Commodore Keer. Upon greeting her, she says, "You did a good job out there. No need to worry about the Collective anymore. Without Welsh, the Collective won't stand a chance, since they aren't truly autonomous. Right now we have some ships cleaning up the last of the Collective; shouldn't take too long to be back to normal."]])
text[4] = _([[She continues. "As a symbol of appreciation, you should find a deposit of 5,000,000 credits in your account. There will be a celebration later today in the officer's room if you want to join in."
    And so ends the Collective threat...
    You don't remember much of the after party, but you wake up groggily in your ship clutching an Empire officer's boot.]])
coward_text = _([[You receive a message signed by Commodore Keer:
    "There is no room for cowards in the Empire's fleet."
    The signature does seem valid.]])
    
start_comm = _("To all pilots, this is mission control! We are ready to begin our attack! Engage at will!")
   
osd_msg = {}
osd_msg[1] = _("Fly to %s via %s and meet up with the Imperial fleet")
osd_msg[2] = _("Defeat the Starfire")
osd_msg2alt = _("Defeat the Starfire and the Trinity")
osd_msg[3] = _("Report back")
osd_msg["__save"] = true

log_text_succeed = _([[You helped the Empire to finally destroy the Collective once and for all. The Collective is now no more.]])
log_text_fail = _([[You abandoned your mission to help the Empire destroy the Collective. Commander Keer transmitted a message: "There is no room for cowards in the Empire's fleet."]])


function create ()
    missys = {system.get("C-59"), system.get("C-28"), system.get("C-00")}
    if not misn.claim(missys) then
        misn.finish( false )
    end

   misn.setNPC( _("Keer"), "empire/unique/keer" )
   misn.setDesc( bar_desc )
end


-- Creates the mission
function accept ()

   -- Intro text
   if not tk.yesno( title[1], text[1] ) then
      misn.finish()
   end

   -- Accept the mission
   misn.accept()

   -- Mission data
   misn_stage = 0
   misn_base, misn_base_sys = planet.get("Omega Station")
   misn_target_sys1 = system.get("C-59")
   misn_target_sys2 = system.get("C-28")
   misn_final_sys = system.get("C-00")
   misn_marker = misn.markerAdd( misn_target_sys2, "low" )
   misn_marker = misn.markerAdd( misn_final_sys, "high" )

   -- Mission details
   misn.setTitle(misn_title)
   misn.setReward( misn_reward )
   misn.setDesc( string.format(misn_desc[1], _(misn_target_sys1:name()) ))
   osd_msg[1] = osd_msg[1]:format(_(misn_final_sys:name()), _(misn_target_sys2:name()))
    if var.peek("trinity") then
        osd_msg[2] = osd_msg2alt
    end
   misn.osdCreate(misn_title, osd_msg)

   tk.msg( title[2], string.format( text[2],
         _(misn_target_sys1:name()), _(misn_final_sys:name()) ) )

   hook.jumpout("jumpout")
   hook.enter("jumpin")
   hook.land("land")
end


function jumpout ()
   last_sys = system.cur()
end


-- Handles jumping to target system
function jumpin ()
    if misn_stage == 0 then
        -- Entering target system?
        if system.cur() == misn_final_sys then
            pilot.clear()
            pilot.toggleSpawn(false)

            -- Create big battle
            fleetE = {}
            fleetC = {}
            droneC = {}
            local fleetCpos = vec2.new(0, 0)
            deathsC = 0

            fleetE[#fleetE + 1] = pilot.add("Empire Peacemaker", nil, last_sys)[1]
            fleetE[#fleetE + 1] = pilot.add("Empire Hawking", nil, last_sys)[1]
            fleetE[#fleetE + 1] = pilot.add("Empire Hawking", nil, last_sys)[1]
            for i = 1, 6 do
                fleetE[#fleetE + 1] = pilot.add("Empire Pacifier", nil, last_sys)[1]
            end
            for i = 1, 15 do
                fleetE[#fleetE + 1] = pilot.add("Empire Lancelot", nil, last_sys)[1]
            end
            
            fleetC[#fleetC + 1] = pilot.add("Starfire", nil, fleetCpos)[1]
            hook.pilot(fleetC[#fleetC], "death", "col_dead")
            fleetC[#fleetC]:setNoDisable()
            fleetC[#fleetC]:setFaction( "Collective" )
            if var.peek("trinity") then
                fleetC[#fleetC + 1] = pilot.add("Trinity", nil, fleetCpos + vec2.new(300, 0))[1]
                hook.pilot(fleetC[#fleetC], "death", "col_dead")
                fleetC[#fleetC]:setNoDisable()
                fleetC[#fleetC]:setFaction( "Collective" )
            end
            droneC = {}
            for i = 1, 60 do
                local pos = fleetCpos + vec2.new(rnd.rnd(-10000, 10000), rnd.rnd(-10000, 10000))
                if i <= 10 then
                    droneC[#droneC + 1] = pilot.add("Collective Heavy Drone", nil, pos)[1]
                else
                    droneC[#droneC + 1] = pilot.add("Collective Drone", nil, pos)[1]
                end
            end
            
            for _, j in ipairs(fleetE) do
                j:changeAI("empire_idle")
                j:setVisible()
            end
            
            for _, j in ipairs(fleetC) do
                j:changeAI("collective_norun")
                j:setVisible()
                j:setHilight()
            end
            for _, j in ipairs(droneC) do
                j:changeAI("collective_norun")
                j:setVisible()
            end

            fleetE[1]:broadcast(start_comm)
            misn.osdActive(2)
            misn_stage = 1
            player.pilot():setVisible()
        elseif system.cur() == misn_target_sys1 or system.cur() == misn_target_sys2 then
            pilot.clear()
            pilot.toggleSpawn(false)
            misn.osdActive(1)
            misn_stage = 0
        else
            misn.osdActive(1)
            misn_stage = 0
        end
    elseif misn_stage == 1 and system.cur() ~= misn_final_sys then
        pilot.clear()
        pilot.toggleSpawn(false)
        misn_stage = 0
        diff.apply("collective_dead")
        hook.timer( 4000, "fail_timer" )
    end
end


function fail_timer ()
   tk.msg( coward_title, coward_text )
   emp_addCollectiveLog( log_text_fail )
   misn.finish( true )
end


function refuelBroadcast ()
   if refship:exists() then
      refship:broadcast(_("Tanker in system, contact if in need of fuel."))
      hook.timer(10000, "refuelBroadcast")
   end
end


function addRefuelShip ()
   -- Create the pilot
   refship = pilot.add( "Trader Mule", "empire_refuel", last_sys )[1]
   refship:rename(_("Fuel Tanker"))
   refship:setFaction("Empire")
   refship:setFriendly()
   refship:setVisplayer()
   refship:setHilight()
   refship:setNoJump()

   -- Maximize fuel
   refship:rmOutfit("all") -- Only will have fuel pods
   local h,m,l = refship:ship():slots()
   refship:addOutfit( "Fuel Pod", l )
   refship:setFuel( true ) -- Set fuel to max

   -- Add some escorts
   refesc = {}
   refesc[1] = pilot.add( "Empire Lancelot", "empire_idle", last_sys )[1]
   refesc[2] = pilot.add( "Empire Lancelot", "empire_idle", last_sys )[1]
   for k,v in ipairs(refesc) do
      v:setFriendly()
   end

   -- Broadcast spam
   refuelBroadcast()
end


-- Handles collective death
function col_dead(victim)
    deathsC = deathsC + 1
    if var.peek("trinity") and deathsC < 2 then
        return
    end
    misn.osdActive(3)
    addRefuelShip()
    diff.apply("collective_dead")
    misn_stage = 4
end


-- Handles arrival back to base
function land ()
   -- Final landing stage
   if misn_stage == 4 and planet.cur() == misn_base then

      tk.msg( title[3], string.format(text[3], _(misn_base:name())) )

      -- Rewards
      -- This was the last mission in the minor campaign, so bump the reputation cap.
      emp_modReputation( 10 )
      faction.modPlayerSingle("Empire",5)
      player.pay( 5000000 ) -- 5m

      tk.msg( title[3], text[4] )
      player.addOutfit("Left Boot")

      emp_addCollectiveLog( log_text_succeed )

      misn.finish(true) -- Run last
   end
end

