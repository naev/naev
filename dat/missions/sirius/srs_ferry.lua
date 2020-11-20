--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Sirius Pilgrimage Transport">
  <avail>
   <priority>3</priority>
   <chance>266</chance>
   <cond>faction.playerStanding("Sirius") &gt; 5</cond>
   <location>Computer</location>
   <faction>Sirius</faction>
  </avail>
  <notes>
   <tier>1</tier>
  </notes>
 </mission>
 --]]
--[[
-- This mission involves ferrying Sirian pilgrims to Mutris ... with complications
-- Higher-class citizens will pay more, but be more picky about their accommodations
--   (they will want to arrive in style in a Sirian ship)
-- Lower-class citizens will be more flexible, even willing to be dropped off nearby
--   if the player doesn't have clearance to land on Mutris
--
-- Est. reward, from Eiderdown (4 jumps):
--  Shaira:  50K - 100K (18 in 21)
--  Fyrra:  150K - 275K  (2 in 21)  (discounted for alternate dest or ship)
--  Serra:  450K - 800K  (1 in 21)  (discounted for alternate dest or ship)
--
-- Est. reward from Rhu (12 jumps):
--  Shaira: 150K - 300K
--  Fyrra:  400K - 775K
--  Serra:  1.1M - 2.2M
--
-- Standing bonus:  (numjumps-4)/2, + up to (1+rank) randomly
--   0-3 at Eiderdown,  2-5 at distance 8,  4-7 at Rhu
--
-- All missions can be made in a Fidelity with an afterburner
--
--]]

require "cargo_common.lua"
require "numstring.lua"

misn_title = _("SR: %s pilgrimage transport for %s-class citizen")
misn_desc = _("%s in the %s system requests transport to %s.")

dest_planet_name = "Mutris"
dest_sys_name = "Aesir"

-- passenger rank
prank = {}
prank[0] = _("Shaira")
prank[1] = _("Fyrra")
prank[2] = _("Serra")

ferrytime = {}
ferrytime[0] = _("Economy") -- Note: indexed from 0, to match mission tiers.
ferrytime[1] = _("Priority")
ferrytime[2] = _("Express")

title_p1 = _("%s space transport to %s for %s-class citizen")

-- Note: please leave the trailing space on the line below! Needed to make the newline show up.
title_p2 = [[ 
Jumps: %d
Travel distance: %d
Time limit: %s]]

slow = {}
slow[1] = _("Too slow")
slow[2] = [[The passenger requests arrival within %s, but it will take at least %s for your ship to reach %s, missing the deadline.

Accept the mission anyway?]]

--=Politics=--
no_clearace_t = _("Deficient clearance")

no_clearance_p1 = _("The passenger looks at your credentials and remarks, \"Someone of your standing will not be allowed to set foot on the holy ground. ")
no_clearance_p2 = {}
no_clearance_p2[0] = _("However, if you can take me as far as %s, I will be satisfied with that.\"")
no_clearance_p2[1] = _("However, I suppose if you can take me to %s, I can find another pilot for the remainder of the flight. But if that is the case, I wouldn't want to pay more than %s.\"")
no_clearance_p2[2] = _("However, if you're on the way to Aesir, I could be willing to pay %s for transportation to %s.\"")
no_clearance_p2[3] = _("Apparently you are not fit to be the pilot for my pilgrimage. It is the will of Sirichana to teach me patience...\"")

-- Outcomes for each of the 4 options above:
-- We pick random number (1-4), then use this as our index into nc_probs[rank]
nc_probs = {}
nc_probs[0] = {0, 0, 0, 1}
nc_probs[1] = {0, 1, 1, 2}
nc_probs[2] = {2, 3, 3, 3}

altplanet = {}
altplanet[0] = "Urail"
altplanet[1] = "Gayathi"

-- If you don't have a Sirian ship
no_ship_t = _("Transportation details")

no_ship_p1 = _("As you arrive at the hangar, the Sirian looks at your ship")

no_ship_p2 = {}
no_ship_p2[0] = _("%s, and you catch a hint of disappointment on their face, before they notice you and quickly hide it.")
no_ship_p2[1] = _("%s and remarks, \"Oh, you didn't tell me your ship is not from our native Sirian shipyards. Since that is the case, I would prefer to wait for another pilot. A pilgrimage is a sacred matter, and the vessel should be likewise.\"\nThe Sirian looks like they might be open to negotiating, however. Would you offer to fly the mission for %s?")
no_ship_p2[2] = _("%s and remarks, \"What? This is to be the ship for my pilgrimage? This is unacceptable - such a crude ship must not be allowed to touch the sacred soil of Mutris. I will wait for a pilot who can ferry me in a true Sirian vessel.\"")

no_ship_t3a = _("Offer accepted")
no_ship_p3a = _("\"Very well. For a price that reasonable, I will adjust my expectations.\"")

no_ship_t3b = _("Offer denied")
no_ship_p3b = _("\"I'm sorry. Your price is reasonable, but piety is of greater value.\"")

-- If you change ships mid-journey
change_ship_t = _("Altering the deal")
change_ship = {}
change_ship[1] = _("On landing, the passenger gives you a brief glare. \"I had paid for transportation in a Sirian ship,\" they remark. \"This alternate arrangement is quite disappointing.\" They hand you %s, but it's definitely less than you were expecting.")
change_ship[2] = _("Since you were unexpectedly able to procure a Sirian ship for the journey, you find a few extra credits tucked in with the fare!")

--=Landing=--

ferry_land_title = _("Successful arrival!")
ferry_land_late = _("Late arrival")

ferry_land_p1 = {}
ferry_land_p1[0] = _("The Sirian Shaira")
ferry_land_p1[1] = _("The Sirian Fyrra")
ferry_land_p1[2] = _("The Sirian Serra")

ferry_land_p2 = {}
ferry_land_p2[0] = _("%s thanks you profusely for your generosity, and carefully counts out your fare.")
ferry_land_p2[1] = _("%s bows briefly in gratitude, and silently places the agreed-upon fare in your hand.")
ferry_land_p2[2] = _("%s crisply counts out your credits, and nods a momentary farewell.")

ferry_land_p3 = {}
ferry_land_p3[0] = _("%s, on seeing the time, looks at you with veiled hurt and disappointment, but carefully counts out their full fare of %s.")
ferry_land_p3[1] = _("%s counts out %s with pursed lips, and walks off before you have time to say anything.")
ferry_land_p3[2] = _("%s tersely expresses their displeasure with the late arrival, and snaps %s down on the seat, with a look suggesting they hardly think you deserve that much.")

accept_title = _("Mission Accepted")

timeup_1 = _("You've missed the scheduled arrival time! But better late than never...")
timeup_2 = _("You're far too late ... best to drop your passengers off on the nearest planet before tempers run any higher.")

fail_t = _("Passenger transport failure")
toolate_msg = _("Well, you arrived, but with this late of an arrival, you can't hope for any payment.")
wrongplanet_msg = _("You drop the upset pilgrim off at the nearest spaceport.")

osd_title = _("Pilgrimage transport")
osd_msg = {}
osd_msg[1] = _("Fly to %s in the %s system before %s")
osd_msg[2] = _("You have %s remaining")
osd_msg1 = _("Fly to %s in the %s system before %s")
osd_msg2 = _("You have %s remaining")

abort_t = _("Passenger transport aborted")
abort_p = _("Informing the pilgrim that their flight to %s has been canceled, you promise to drop them off at the nearest planet.")


function ferry_calculateRoute (dplanet, dsys)
    origin_p, origin_s = planet.cur()
    local routesys = origin_s
    local routepos = origin_p:pos()

    -- Select mission tier.
    -- determines class of Sirian citizen
    local rank = rnd.rnd(0, 20)
    if rank < 18 then
        rank = 0
    elseif rank < 20 then
        rank = 1
    else
        rank = 2
    end

    local speed = rnd.rnd(0,2)  -- how long you have; priority of the ticket

    local destplanet = planet.get(dplanet)
    local destsys = system.get(dsys)

    -- We have a destination, now we need to calculate how far away it is by simulating the journey there.
    -- Assume shortest route with no interruptions.
    -- This is used to calculate the reward.

    local numjumps   = origin_s:jumpDist(destsys)
    local traveldist = cargo_calculateDistance(routesys, routepos, destsys, destplanet)

    -- Return lots of stuff
    return destplanet, destsys, numjumps, traveldist, speed, rank --cargo, tier
end


-- Create the mission
function create()
    -- RULES:
    -- You have to be flying a Sirian ship to land on Mutris, and have standing > 75, but you get much more money
    --   faction.get('Sirius'):playerStanding() > 75
    --   player.pilot():ship():baseType() in (...)
    -- Otherwise, you can drop the person off at Urail or Gayathi (if they're OK with that) and get less pay
    --   Lower-class citizens are more likely to be OK with this

    -- Dest planet will be Mutris, dest system is Aesir, unless things change in the game
    if not system.get(dest_sys_name):known() then
       misn.finish(false)
    end

    origin_p, origin_s = planet.cur()
    if origin_s == system.get(dest_sys_name) then
       misn.finish(false)
    end

    -- Calculate the route, distance, jumps, time limit, and priority
    destplanet, destsys, numjumps, traveldist, print_speed, rank = ferry_calculateRoute(dest_planet_name, dest_sys_name)

    if numjumps < 2 then  -- don't show mission on really close systems; they don't need you for short hops
       misn.finish(false)
    end

    -- Calculate time limit. Depends on priority and rank.
    -- The second time limit is for the reduced reward.
    speed = print_speed + rank - 1    -- higher-ranked citizens want faster transport
    stuperpx   = 0.2 - 0.02 * speed
    stuperjump = 11000 - 200 * speed
    stupertakeoff = 12000 - 75 * speed
    allowance  = traveldist * stuperpx + numjumps * stuperjump + stupertakeoff + 240 * numjumps

    -- Allow extra time for refuelling stops.
    local jumpsperstop = 3 + rank
    if numjumps > jumpsperstop then
        allowance = allowance + math.floor((numjumps-1) / jumpsperstop) * stuperjump
    end

    timelimit  = time.get() + time.create(0, 0, allowance)
    timelimit2 = time.get() + time.create(0, 0, allowance * 1.3)

    -- Choose mission reward. This depends on the priority and the passenger rank.
    finished_mod = 2.0 -- Modifier that should tend towards 1.0 as Naev is finished as a game
    jumpreward = 10000
    distreward = 0.18
    reward     = 1.4^(speed + rank) * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma()) / (2-rank/2.0)

    -- Set some mission constants.
    distbonus_maxjumps = 12 -- This is the maximum distance for reputation bonus calculations. Distances beyond this don't add to the bonus.
    distbonus_minjumps = 5 -- This is the minimum distance needed to get a reputation bonus. Distances less than this don't incur a bonus.

    misn.markerAdd(destsys, "computer")
    misn.setTitle( string.format(misn_title, ferrytime[print_speed], prank[rank]) )
    misn.setDesc(title_p1:format( ferrytime[print_speed], destplanet:name(), prank[rank]) .. title_p2:format(numjumps, traveldist, (timelimit - time.get()):str()))
    misn.setReward(creditstring(reward))

    -- Set up passenger details so player cannot keep trying to get a better outcome
    destpicky = rnd.rnd(1,4)
    shippicky = rank*2 + rnd.rnd(-1,1)

end

function player_has_sirian_ship()
    local playership = player.pilot():ship():baseType()
    local sirianships = {"Fidelity", "Shaman", "Preacher", "Divinity", "Dogma"}
    local has_sirian_ship = false
    for _,v in pairs(sirianships) do
        if playership == v then
            has_sirian_ship = true
            break
        end
    end
    return has_sirian_ship
end

-- Mission is accepted
function accept()
    local playerbest = cargoGetTransit( timelimit, numjumps, traveldist )
    if timelimit < playerbest then
        if not tk.yesno( slow[1], slow[2]:format( (timelimit - time.get()):str(), (playerbest - time.get()):str(), destplanet:name()) ) then
            misn.finish()
        end
    end

    --if faction.get('Sirius'):playerStanding() <= 75 then
    local can_land, can_bribe = destplanet:canLand()  -- Player with rank < 75 will not be allowed to land on Mutris
    if not can_land then
        -- Decide if the passenger will be OK with being dropped off at Urail or Gayathi, and if reward is reduced
        -- Then ask player if they're OK with that

        local counter = 0
        local altplanets = {}
        for key,dplanet in ipairs( destsys:planets() ) do
            if dplanet:canLand() then
                counter = counter + 1
                altplanets[counter] = dplanet
            end
        end

        if counter == 0 then
            -- Something has changed with the system map, and this mission is no longer valid
            print( _("Error: no landable planets in the Aesir system. This mission is broken.\n") )
            misn.finish()
        end

        altdest = rnd.rnd(1,counter)

        local ok = false
        --local picky = rnd.rnd(1,4)  -- initialized in the create function
        local outcome = nc_probs[rank][destpicky]

        if outcome == 3 then
            -- Rank 2 will demand to be delivered to Sirius
            tk.msg(no_clearace_t, no_clearance_p1 .. no_clearance_p2[outcome])
        elseif outcome == 2 then
            -- Rank 1 will accept an alternate destination, but cut your fare
            reward = reward / 2
            ok = tk.yesno(no_clearace_t, no_clearance_p1 .. no_clearance_p2[outcome]:format(creditstring(reward), altplanets[altdest]:name()) )
        elseif outcome == 1 then
            -- OK with alternate destination, with smaller fare cut
            reward = reward * 0.6666
            ok = tk.yesno(no_clearace_t, no_clearance_p1 .. no_clearance_p2[outcome]:format(altplanets[altdest]:name(), creditstring(reward)) )
        else
            -- Rank 0 will take whatever they can get
            ok = tk.yesno(no_clearace_t, no_clearance_p1 .. no_clearance_p2[outcome]:format(altplanets[altdest]:name()) )
        end

        if not ok then
            misn.finish()
        end

        destplanet = altplanets[altdest]
        misn.setDesc(title_p1:format( ferrytime[print_speed], destplanet:name(), prank[rank]) .. title_p2:format(numjumps, traveldist, (timelimit - time.get()):str()))
        --wants_sirian = false    -- Don't care what kind of ship you're flying
    end

    -- Sirians prefer to make their pilgrimage in a Sirian ship
    if not player_has_sirian_ship() then
        local picky = shippicky -- initialized in the create function

        if not can_land then picky = picky - 1 end  -- less picky about ship when going to alternate destination

        if picky > 2 then
            -- Demands to be delivered in a Sirian ship
            tk.msg(no_ship_t, no_ship_p2[2]:format( no_ship_p1 ))
            misn.finish()
        elseif picky > 0 then
            -- Could be persuaded, for a discount
            reward = reward*0.6666
            if not tk.yesno(no_ship_t, no_ship_p2[1]:format(no_ship_p1, creditstring(reward))) then
                misn.finish() -- Player won't offer a discount
            end
            if picky > 1 then
                tk.msg(no_ship_t3b, no_ship_p3b)
                misn.finish() -- Would not be persuaded by a discount
            else
                tk.msg(no_ship_t3a, no_ship_p3a)  -- discount is ok
            end
        elseif picky <= 0 then
            tk.msg(no_ship_t, no_ship_p2[0]:format(no_ship_p1)) -- ok with the arrangements
        end

        wants_sirian = false  -- Will not expect to arrive in a Sirian ship
    else
        wants_sirian = true   -- Will expect to arrive in a Sirian ship
    end

    misn.accept()
    intime = true
    overtime = false
    misn.cargoAdd("Pilgrims", 0)  -- We'll assume you can hold as many pilgrims as you want?
    osd_msg[1] = osd_msg1:format(destplanet:name(), destsys:name(), timelimit:str())
    osd_msg[2] = osd_msg2:format((timelimit - time.get()):str())
    misn.osdCreate(osd_title, osd_msg)
    hook.land("land")
    hook.date(time.create(0, 0, 100), "tick") -- 100STU per tick
end

-- Land hook
function land()
    if planet.cur() == destplanet then

        -- Check if we're still flying a Sirian ship
        has_sirian_ship = player_has_sirian_ship()

        change = 0
        if wants_sirian and not has_sirian_ship then
            change = 1  -- Bad: they wanted a Sirian ship and you switched on them
            reward = reward / (rank+1.5)
            tk.msg( change_ship_t, change_ship[change]:format( creditstring(reward) ) )
            player.pay(reward)
            misn.finish(true)
        elseif not wants_sirian and has_sirian_ship then
            change = 2  -- Good: they weren't expecting a Sirian ship, but they got one anyway
        end

        if intime then
            local distbonus = math.max(math.min(numjumps,distbonus_maxjumps)-distbonus_minjumps+1, 0) / 2  -- ranges from 0 (<distbonus_minjumps jumps) to 4 (>=distbonus_maxjumps jumps)
            faction.modPlayerSingle("Sirius", rnd.rnd(distbonus, distbonus+rank+1))

            tk.msg(ferry_land_title, ferry_land_p2[rank]:format( ferry_land_p1[rank]) )
        elseif overtime then
            tk.msg(fail_t, toolate_msg)
            misn.finish(false)
        else
            -- You were late
            reward = reward / (rank + 1)
            tk.msg(ferry_land_late, ferry_land_p3[rank]:format( ferry_land_p1[rank], creditstring(reward)))
        end

        if change == 2 then
            faction.modPlayerSingle("Sirius", 1)  -- A little bonus for doing something nice
            tk.msg(change_ship_t, change_ship[change])  -- Pay them a bonus for using a Sirian ship
            reward = reward * 1.25
        end

        player.pay(reward)
        misn.finish(true)
    elseif timelimit2 <= time.get() then
        -- if we missed the second deadline, drop the person off at the planet.
        tk.msg(fail_t, wrongplanet_msg)
        misn.finish(false)
    end
end

-- Date hook
function tick()
    if timelimit >= time.get() then
        -- Case still in time
        osd_msg[1] = osd_msg1:format(destplanet:name(), destsys:name(), timelimit:str())
        osd_msg[2] = osd_msg2:format((timelimit - time.get()):str())
        misn.osdCreate(osd_title, osd_msg)
    elseif timelimit2 <= time.get() and not overtime then
        -- Case missed second deadline
        player.msg(timeup_2)
        misn.osdCreate( _("Pilgrim drop-off"), 
            {_("Drop off the pilgrims at the nearest planet")})
        overtime = true
    elseif intime then
        -- Case missed first deadline
        player.msg(timeup_1)
        osd_msg[1] = osd_msg[1]:format(destplanet:name(), destsys:name(), timelimit:str())
        osd_msg[2] = timeup_1--:format(destsys:name())
        misn.osdCreate(osd_title, osd_msg)
        intime = false
    end
end

function abort()
    tk.msg(abort_t, abort_p:format(destplanet:name()))
end
