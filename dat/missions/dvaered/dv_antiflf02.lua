--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Lure out the FLF">
 <unique />
 <priority>2</priority>
 <chance>10</chance>
 <location>Bar</location>
 <cond>var.peek("flfbase_intro") == 1</cond>
 <faction>Dvaered</faction>
 <notes>
  <done_misn name="Take the Dvaered crew home"/>
  <done_misn name="Deal with the FLF agent">If you deliver Gregar to the Dvaered</done_misn>
  <provides name="The Dvaered know where Sindbad is"/>
  <campaign>Doom the FLF</campaign>
 </notes>
</mission>
--]]
--[[
   This is the second mission in the anti-FLF Dvaered campaign. The player is part of a Dvaered plot to smoke out the FLF base.
   stack variable flfbase_intro:
        1 - The player has turned in the FLF agent or rescued the Dvaered crew. Conditional for dv_antiflf02
        2 - The player has rescued the FLF agent. Conditional for flf_pre02
        3 - The player has found the FLF base for the Dvaered, or has betrayed the FLF after rescuing the agent. Conditional for dv_antiflf03
--]]

local fleet = require "fleet"
local fmt = require "format"
local portrait = require "portrait"
local dv = require "common.dvaered"
local ai_setup = require "ai.core.setup"

local fleetDV, fleetFLF -- Non-persistent state
local spawnFLF -- Forward-declared functions

function create()
    local missys = {system.get(var.peek("flfbase_sysname"))}
    if not misn.claim(missys) then
        abort()
    end

    misn.setNPC(_("Dvaered liaison"), portrait.getMaleMil("Dvaered"), _("This must be the Dvaered liaison you heard about. Allegedly, he may have a job for you that involves fighting the Frontier Liberation Front."))
end

function accept()
    if tk.yesno(_("House Dvaered needs YOU"), _([[You join the Dvaered official at his table. He greets you in a cordial fashion, at least by Dvaered standards. You explain to him that you wish to know more about the anti-FLF operation that House Dvaered is supposedly working on.
    "Ah! It's good to see such righteous enthusiasm among our citizenry. Indeed, our forces have been preparing to deal a significant blow to the FLF terrorists. I can't disclose the details, of course - this is a military operation. However, if you have a combat-capable ship and enough sense of duty to assist us, there is an opportunity to serve alongside the real Dvaered warriors. How about it? Can we count on your support?"]])) then
        mem.destsys = system.get(var.peek("flfbase_sysname"))
        tk.msg(_("A clever ruse"), _([["Splendid. You have the right mindset, citizen. If only all were like you! But that is neither here nor there. Come, I will take you to the local command bunker. The details of this operation will be explained to you there."
    True to his word, the Dvaered liaison escorts you out of the spaceport bar, and within the hour you find yourself deep inside a highly secured Dvaered military complex. You are ushered into a room, in which you find a large table and several important-looking military men. At the head of the table sits a man whose name tag identifies him as Colonel Urnus. Evidently he's the man in charge.
    A Dvaered soldier instructs you to take a seat.]]))
        tk.msg(_("A clever ruse"), fmt.f(_([[As you take the last empty seat at the table, Colonel Urnus starts the meeting. "Welcome, citizen. You know why you are here and you know what this meeting is about, so let me get right to the point. We have reason to believe the FLF terrorists are operating from a secret base of operations. We know this base is located somewhere in the nebula, and we have recently uncovered intel that indicates the base is likely to be in the {sys} system."
    One of the walls lights up, showing part of the galaxy map. The {sys} system is coloured red, and it's pulsating gently.
    "Of course, we have conducted patrols in this system, but so far without result. Our sensors are severely impaired by the nebula in this system, so the chances of us finding the terrorist hive by our own devices are slim. Fortunately, our top strategists have come up with a ruse, one that will make the FLF come to us."
    "We will use a civilian ship - your ship, naturally - as a decoy," Urnus continues. The image on the wall zooms up to the {sys} system, and a white blip representing your ship appears near the jump point. "Your ship will be equipped with an IFF transponder in use by the FLF, so to anyone who isn't looking too closely you will appear as an FLF ship. Of course, this alone is not enough. The FLF will assume you know where their base is, since you look like one of them. It is important to note that the transponder will work best in the Nebula, which we believe is where the base should be located."
    The image on the wall updates again, this time showing several House Dvaered crests near your ship.
    "Some time after you enter the system, several of our military assets will jump in and open fire. To the FLF, it will look like one of their own has come under attack! Since their base is nearby, they will undoubtedly send reinforcements to help their 'comrade' out of a tight situation."]]), {sys=mem.destsys}))
        tk.msg(_("A clever ruse"), _([["As soon as the FLF ships join the battle, you and the Dvaered ships will disengage and target the FLF instead. Your mission is to render at least one of their ships incapable of fighting, and board it. You can then access the ship's computer and download the flight log, which should contain the location of the FLF base. Take this information to a Dvaered base, and your mission will be complete."
    The image on the wall updates one last time, simulating the battle as described by Colonel Urnus. Several FLF logos appear, which are promptly surrounded by the Dvaered ones. Then the logos turn gray, indicating that they've been disabled.
    "Let me make one thing clear, citizen. You are allowed, even expected, to fire on the Dvaered ships that are firing on you. However, you must make it look you're on the losing side, or the FLF will not come to your aid! So, do NOT disable or destroy any Dvaered ships, and make sure your own armour takes a bit of a beating. This is vital to the success of the mission. Do not fail."
    Colonel Urnus seems to have concluded his explanation, so you, having spotted the obvious flaw in the Dvaereds' plan, pop the question of what happens if the FLF never show up.
    "Well," the Colonel muses, "That will mean our intel was probably wrong. But don't worry, citizen, we'll get those terrorists eventually! Now, time is of the essence, so get to your ship and follow your orders. Dismissed!"]]))

        misn.accept()
        misn.osdCreate(_("Lure out the FLF"), {
            fmt.f(_("Fly to the {sys} system"), {sys=mem.destsys}),
            _("Wait for the Dvaered military to jump in and attack you"),
            _("Fight the Dvaered until the FLF step in"),
            _("Disable and board at least one FLF ship"),
            _("Return to any Dvaered world"),
        })
        misn.setDesc(_("You have been recruited to act as a red herring in a military operation of Dvaered design. Your chief purpose is to goad the FLF into showing themselves, then disable and board one of their ships. You will fail this mission if you disable or destroy any Dvaered ship, or if you leave the system before the operation is complete."))
        misn.setTitle(_("Lure out the FLF"))
        misn.setReward(_("Serving alongside real Dvaered warlords"))
        mem.marker = misn.markerAdd( mem.destsys, "low" )

        mem.missionstarted = false
        mem.DVdisablefail = true
        mem.logsfound = false
        mem.flfdead = 0

        local c = commodity.new( N_("FLF IFF Transponder"), N_("A transponder that will make your ship appear as FLF in the nebula.") )
        misn.cargoAdd(c, 0)

        hook.jumpout("jumpout")
        hook.enter("enter")
        hook.land("land")
    else
        tk.msg(_("House Dvaered is out of luck"), _([["I see. In that case, I'm going to have to ask you to leave. My job is to recruit a civilian, but you're clearly not the pilot I'm looking for. You may excuse yourself, citizen."]]))
        return
    end
end

function jumpout()
    mem.last_sys = system.cur()
end

function enter()
    if system.cur() == mem.destsys and not mem.logsfound then
        pilot.clear()
        pilot.toggleSpawn(false)
        misn.osdActive(2)
        hook.timer(15.0, "spawnDV")
    elseif mem.missionstarted then -- The player has jumped away from the mission theater, which instantly ends the mission.
        tk.msg(_("You ran away!"), _("You have left the system without first completing your mission. The operation ended in failure."))
        faction.get("Dvaered"):modPlayerSingle(-10)
        abort()
    end
end

function land()
    if mem.logsfound and spob.cur():faction() == faction.get("Dvaered") then
        tk.msg(_("X marks the spot"), _([[As soon as you land, a Dvaered military operator contacts you and requests you turn over the flight log you procured from the FLF ship, so you do. The Dvaered are then silent for some twenty hectoseconds, time you use to complete your post-landing routines. Then, you are summoned to the local Dvaered security station.
    Colonel Urnus welcomes you. "Well met, citizen. I have received word of your accomplishment in our recent operation. It seems HQ is quite pleased with the result, and they have instructed me to reward you appropriately."
    He hands you a credit chip that represents a decent sum of money, though you feel that a mere monetary reward doesn't begin to compensate for the dangerous plan the Dvaered made you part of. However, you wisely opt not to give voice to that thought.
    "In addition," Urnus resumes, "Dvaered military command has decided that you may participate in the upcoming battle against the FLF stronghold, in recognition of your courage and your skill in battle. You may contact our liaison whenever you're ready."
    That concludes the pleasantries, and you are unceremoniously deposited outside the security compound. But at least you earned some money - and a chance to see some real action.]]))
        var.push("flfbase_intro", 3)
        faction.get("Dvaered"):modPlayerSingle(5)
        player.pay(700e3)
        dv.addAntiFLFLog( _([[You aided the Dvaered in their efforts to locate a secret FLF base. You posed as an FLF pilot, luring real FLF pilots out so that one could be disabled and you could steal its flight logs. The Dvaered are now planning an assault on the terrorist base, and Colonel Urnus said that you can contact a Dvaered liaison whenever you're ready for the operation.]]) )
        misn.finish(true)
    end
end

function spawnDV()
    misn.osdActive(3)
    mem.missionstarted = true
    fleetDV = fleet.add( 3, "Dvaered Vigilance", "Dvaered", mem.last_sys, nil, {ai="dvaered_norun"} )
    -- The Dvaered ships should attack the player, so set them hostile.
    -- These are Vigilances, so we should tune them WAY down so the player doesn't insta-die.
    for i, j in ipairs(fleetDV) do
        j:setHostile()
        j:setHilight(true)
        j:setVisplayer(true)
        j:outfitRm("all")
        j:outfitAdd("Turreted Gauss Gun", 1)
        j:outfitAdd("Small Shield Booster", 1)
        ai_setup.setup(j)
        hook.pilot(j, "disable", "disableDV")
    end

    hook.timer(0.5, "pollHealth")
end

-- Polls the player's health and the Dvaereds' shields, and spawns the FLF fleet if shields and armor are below a certain value.
function pollHealth()
    mem.shieldDV = 0
    mem.parmor, mem.pshield = player.pilot():health()
    local maxshieldDV = 0
    for i, j in ipairs(fleetDV) do
        maxshieldDV = maxshieldDV + j:stats()["shield"]
        mem.armor, mem.shield = j:health()
        mem.shieldDV = mem.shieldDV + mem.shield
    end
    if mem.parmor <= 60 and mem.pshield <= 10 and mem.shieldDV <= (maxshieldDV - 50) then
        spawnFLF()
    else
        hook.timer(0.5, "pollHealth")
    end
end

-- Spawn the FLF ships, reset the Dvaered.
function spawnFLF()
    mem.DVdisablefail = false -- The player no longer fails the mission if a Dvaered ship is disabled
    misn.osdActive(4)
    for i, j in ipairs(fleetDV) do
        j:setFriendly()
        j:setHilight(false)
        j:changeAI("dvaered_norun")
        j:setInvincPlayer()
        -- Re-outfit the ships to use disable weapons. Kind of ugly, should probably be handled via AI orders in the future.
        j:outfitAdd("EMP Grenade Launcher", 3)
        ai_setup.setup(j)
    end
    local vecFLF = vec2.newP(800, rnd.angle() )
    fleetFLF = fleet.add( 4, "Vendetta", "FLF", player.pos() + vecFLF, nil, {ai="flf_norun"} )
    mem.flfactive = #fleetFLF
    fleetDV[1]:comm(_("Here come the FLF! All units, switch to EMPs and disable the terrorist ships!"))

    for i, j in ipairs(fleetFLF) do
        j:setHilight(true)
        j:setVisplayer()
        hook.pilot(j, "disable", "disableFLF")
        hook.pilot(j, "death", "deathFLF")
        hook.pilot(j, "board", "boardFLF")
    end
end

function disableDV()
    if mem.DVdisablefail then -- Only true as long as the FLF aren't there yet
        tk.msg(_("You fought too hard!"), _("You have disabled a Dvaered ship, thereby violating your orders. The operation is canceled thanks to you. The Dvaered are less than pleased."))
        faction.get("Dvaered"):modPlayerSingle(-10)
        abort()
    end
end

function disableFLF()
    mem.flfactive = mem.flfactive - 1
    -- Persuade the Dvaered to stop shooting at disabled FLF
    for i, j in ipairs(fleetDV) do
        if j:exists() then
            if mem.flfactive == 0 and not mem.messaged then
               fleetDV[i]:comm(_("All targets neutralized. Download the flight log and let's get out of here!"))
               mem.messaged = true
            end
            j:changeAI("dvaered_norun")
        end
    end
end

function deathFLF()
    mem.flfdead = mem.flfdead + 1
    if mem.flfdead == 6 and not mem.logsfound then
        tk.msg(_("All the FLF are dead!"), _("Unfortunately, all the FLF ships have been destroyed before you managed to board one of them. The operation ended in failure."))
        abort()
    end
end

function boardFLF()
    misn.osdActive(5)
    tk.msg(_("Take no prisoners - only their logs"), _([[You successfully board the FLF ship and secure its flight logs. This is what the Dvaered want - you should take it to a Dvaered planet immediately.]]))
    mem.missionstarted = false
    mem.logsfound = true
    player.unboard()
    misn.markerRm(mem.marker)
    for i, j in ipairs(fleetDV) do
        if j:exists() then
            j:changeAI("flee") -- Make them jump out (if they're not dead!)
        end
    end
    for _, j in ipairs(fleetFLF) do
        if j:exists() then
            j:setHilight(false)
            j:setVisplayer(false)
            j:setNoboard(true)
        end
    end
end

function abort()
    misn.finish(false)
end
