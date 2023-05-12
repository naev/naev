--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Sirius Pilgrimage Transport">
 <priority>3</priority>
 <chance>266</chance>
 <cond>
   if faction.playerStanding("Sirius") &lt; 0 then
      return false
   end
   return require("misn_test").computer()
 </cond>
 <location>Computer</location>
 <faction>Sirius</faction>
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
local car = require "common.cargo"
local fmt = require "format"
local srs = require "common.sirius"
local vntk = require "vntk"


local dest_planet, dest_sys = spob.getS("Mutris")

-- passenger rank
local prank = {}
prank[0] = _("Shaira")
prank[1] = _("Fyrra")
prank[2] = _("Serra")

local ferrytime = {}
ferrytime[0] = _("Economy") -- Note: indexed from 0, to match mission tiers.
ferrytime[1] = _("Priority")
ferrytime[2] = _("Express")

-- Outcomes for each of the 4 options above:
-- We pick random number (1-4), then use this as our index into nc_probs[rank]
local nc_probs = {}
nc_probs[0] = {0, 0, 0, 1}
nc_probs[1] = {0, 1, 1, 2}
nc_probs[2] = {2, 3, 3, 3}

--=Landing=--
local ferry_land_p1 = {}
ferry_land_p1[0] = _("The Sirian Shaira")
ferry_land_p1[1] = _("The Sirian Fyrra")
ferry_land_p1[2] = _("The Sirian Serra")

local ferry_land_p2 = {}
ferry_land_p2[0] = _("{passenger} thanks you profusely for your generosity, and carefully counts out your fare.")
ferry_land_p2[1] = _("{passenger} bows briefly in gratitude, and silently places the agreed-upon fare in your hand.")
ferry_land_p2[2] = _("{passenger} crisply counts out your credits, and nods a momentary farewell.")

local ferry_land_p3 = {}
ferry_land_p3[0] = _("{passenger}, on seeing the time, looks at you with veiled hurt and disappointment, but carefully counts out their full fare of {credits}.")
ferry_land_p3[1] = _("{passenger} counts out {credits} with pursed lips, and walks off before you have time to say anything.")
ferry_land_p3[2] = _("{passenger} tersely expresses their displeasure with the late arrival, and snaps {credits} down on the seat, with a look suggesting they hardly think you deserve that much.")

-- Customization of car.calculateRoute in common.cargo
local function ferry_calculateRoute (dplanet, dsys)
   mem.origin_p, mem.origin_s = spob.cur()
   local routesys = mem.origin_s
   local routepos = mem.origin_p:pos()

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
   local numjumps   = mem.origin_s:jumpDist(dsys)
   local traveldist = car.calculateDistance(routesys, routepos, dsys, dplanet)
   return dplanet, dsys, numjumps, traveldist, speed, rank --cargo, tier
end


-- Create the mission
function create()
   -- RULES:
   -- You have to be flying a Sirian ship to land on Mutris, and have standing > 75, but you get much more money
   --   faction.get('Sirius'):playerStanding() > 75
   --   player.pilot():ship():baseType() in (...)
   -- Otherwise, you can drop the person off at Urail or Gayathi (if they're OK with that) and get less pay
   --   Lower-class citizens are more likely to be OK with this

   mem.origin_p, mem.origin_s = spob.cur()
   if mem.origin_s == dest_sys then
      misn.finish(false)
   end

   -- Calculate the route, distance, jumps, time limit, and priority
   mem.destplanet, mem.destsys, mem.numjumps, mem.traveldist, mem.print_speed, mem.rank = ferry_calculateRoute(dest_planet, dest_sys)

   if mem.numjumps < 2 then  -- don't show mission on really close systems; they don't need you for short hops
      misn.finish(false)
   end

   -- Calculate time limit. Depends on priority and rank.
   -- The second time limit is for the reduced reward.
   local speed = mem.print_speed + mem.rank - 1    -- higher-ranked citizens want faster transport
   local stuperpx   = 0.2 - 0.02 * speed
   local stuperjump = 11000 - 200 * speed
   local stupertakeoff = 12000 - 75 * speed
   local allowance  = mem.traveldist * stuperpx + mem.numjumps * stuperjump + stupertakeoff + 240 * mem.numjumps

   -- Allow extra time for refuelling stops.
   local jumpsperstop = 3 + mem.rank
   if mem.numjumps > jumpsperstop then
      allowance = allowance + math.floor((mem.numjumps-1) / jumpsperstop) * stuperjump
   end

   mem.timelimit  = time.get() + time.new(0, 0, allowance)
   mem.timelimit2 = time.get() + time.new(0, 0, allowance * 1.3)

   -- Choose mission reward. This depends on the priority and the passenger rank.
   local jumpreward = 15e3
   local distreward = 0.25
   mem.reward     = 1.4^(speed + mem.rank) * (mem.numjumps * jumpreward + mem.traveldist * distreward) * (1. + 0.05*rnd.twosigma()) / (2-mem.rank/2.0)

   -- Set some mission constants.
   mem.distbonus_maxjumps = 12 -- This is the maximum distance for reputation bonus calculations. Distances beyond this don't add to the bonus.
   mem.distbonus_minjumps = 5 -- This is the minimum distance needed to get a reputation bonus. Distances less than this don't incur a bonus.

   misn.markerAdd(mem.destplanet, "computer")
   misn.setTitle( fmt.f(srs.prefix.._("{tier} pilgrimage transport for {rank}-class citizen"), {tier=ferrytime[mem.print_speed], rank=prank[mem.rank]}) )
   car.setDesc( fmt.f(_("{tier} space transport to {pnt} for {rank}-class citizen"), {tier=ferrytime[mem.print_speed], pnt=mem.destplanet, rank=prank[mem.rank]}), nil, nil, mem.destplanet, mem.timelimit )
   misn.setReward(mem.reward)

   -- Set up passenger details so player cannot keep trying to get a better outcome
   mem.destpicky = rnd.rnd(1,4)
   mem.shippicky = mem.rank*2 + rnd.rnd(-1,1)

end

local function player_has_sirian_ship()
   return player.pilot():ship():tags().sirius
end

-- Mission is accepted
function accept()
   local playerbest = car.getTransit( mem.numjumps, mem.traveldist )
   if mem.timelimit < playerbest then
      if not vntk.yesno( _("Too slow"), fmt.f(_([[The passenger requests arrival within {time_limit}, but it will take at least {time} for your ship to reach {pnt}, missing the deadline.

Accept the mission anyway?]]), {time_limit=(mem.timelimit - time.get()), time=(playerbest - time.get()), pnt=mem.destplanet}) ) then
         return
      end
   end

   --if faction.get('Sirius'):playerStanding() <= 75 then
   local can_land = mem.destplanet:canLand()  -- Player with mem.rank < 75 will not be allowed to land on Mutris
   if not can_land then
      -- Decide if the passenger will be OK with being dropped off at Urail or Gayathi, and if mem.reward is reduced
      -- Then ask player if they're OK with that
      local counter = 0
      local altplanets = {}
      for key,dplanet in ipairs( mem.destsys:spobs() ) do
         if dplanet:canLand() then
            counter = counter + 1
            altplanets[counter] = dplanet
         end
      end

      if counter == 0 then
         -- Something has changed with the system map, and this mission is no longer valid
         print(_("Error: no landable planets in the Aesir system. This mission is broken."))
         return
      end

      mem.altdest = rnd.rnd(1,counter)

      local ok = false
      local can_downgrade = true
      local no_clearance_text
      --local picky = rnd.rnd(1,4)  -- initialized in the create function
      local outcome = nc_probs[mem.rank][mem.destpicky]

      if outcome == 3 then
         -- Rank 2 will demand to be delivered to Sirius
         can_downgrade = false
         no_clearance_text = _([[Apparently you are not fit to be the pilot for my pilgrimage. It is the will of Sirichana to teach me patience…"]])
      elseif outcome == 2 then
         -- Rank 1 will accept an alternate destination, but cut your fare
         mem.reward = mem.reward / 2
         no_clearance_text = fmt.f(_([[However, if you're on the way to Aesir, I could be willing to pay {credits} for transportation to {pnt}."]]),
            {credits=fmt.credits(mem.reward), pnt=altplanets[mem.altdest]})
      elseif outcome == 1 then
         -- OK with alternate destination, with smaller fare cut
         mem.reward = mem.reward * 0.6666
         no_clearance_text = fmt.f(_([[However, I suppose if you can take me to {pnt}, I can find another pilot for the remainder of the flight. But if that is the case, I wouldn't want to pay more than {credits}."]]),
            {pnt=altplanets[mem.altdest], credits=fmt.credits(mem.reward)})
      else
         -- Rank 0 will take whatever they can get
         no_clearance_text = fmt.f(_([[However, if you can take me as far as {pnt}, I will be satisfied with that."]]), {pnt=altplanets[mem.altdest]})
      end

      local no_clearance_p1 = _([[The passenger looks at your credentials and remarks, "Someone of your standing will not be allowed to set foot on the holy ground. ]])
      if can_downgrade then
         ok = vntk.yesno(_("Deficient clearance"), no_clearance_p1 .. no_clearance_text)
      else
         vntk.msg(_("Deficient clearance"), no_clearance_p1 .. no_clearance_text)
      end
      if not ok then
         return
      end

      mem.destplanet = altplanets[mem.altdest]
      car.setDesc( fmt.f(_("{tier} space transport to {pnt} for {rank}-class citizen"), {tier=ferrytime[mem.print_speed], pnt=mem.destplanet, rank=prank[mem.rank]}), nil, nil, mem.destplanet, mem.timelimit )
      --mem.wants_sirian = false    -- Don't care what kind of ship you're flying
   end

   -- Sirians prefer to make their pilgrimage in a Sirian ship
   if not player_has_sirian_ship() then
      local picky = mem.shippicky -- initialized in the create function

      if not can_land then picky = picky - 1 end  -- less picky about ship when going to alternate destination

      if picky > 2 then
         -- Demands to be delivered in a Sirian ship
         vntk.msg(_("Transportation details"), _([[As you arrive at the hangar, the Sirian looks at your ship and remarks, "What? This is to be the ship for my pilgrimage? This is unacceptable - such a crude ship must not be allowed to touch the sacred soil of Mutris. I will wait for a pilot who can ferry me in a true Sirian vessel."]]))
         return
      elseif picky > 0 then
         -- Could be persuaded, for a discount
         mem.reward = mem.reward*0.6666
         if not vntk.yesno(_("Transportation details"), fmt.f(_([["As you arrive at the hangar, the Sirian looks at your ship and remarks, "Oh, you didn't tell me your ship is not from our native Sirian shipyards. Since that is the case, I would prefer to wait for another pilot. A pilgrimage is a sacred matter, and the vessel should be likewise."
The Sirian looks like they might be open to negotiating, however. Would you offer to fly the mission for {credits}?"]]),
            {credits=fmt.credits(mem.reward)})) then
            return -- Player won't offer a discount
         end
         if picky > 1 then
            vntk.msg(_("Offer denied"), _([["I'm sorry. Your price is reasonable, but piety is of greater value."]]))
            return -- Would not be persuaded by a discount
         else
            vntk.msg(_("Offer accepted"), _([["Very well. For a price that reasonable, I will adjust my expectations."]]))  -- discount is ok
         end
      elseif picky <= 0 then
         vntk.msg(_("Transportation details"), _("As you arrive at the hangar, the Sirian looks at your ship, and you catch a hint of disappointment on their face, before they notice you and quickly hide it.")) -- ok with the arrangements
      end

      mem.wants_sirian = false  -- Will not expect to arrive in a Sirian ship
   else
      mem.wants_sirian = true   -- Will expect to arrive in a Sirian ship
   end

   misn.accept()
   mem.intime = true
   mem.overtime = false
   local c = commodity.new( N_("Pilgrims"), N_("A bunch of giddy Sirian pilgrims.") )
   misn.cargoAdd(c, 0)  -- We'll assume you can hold as many pilgrims as you want?
   tick()
   hook.land("land")
   hook.date(time.new(0, 0, 100), "tick") -- 100STU per tick
end

-- Land hook
function land()
   if spob.cur() == mem.destplanet then
      -- Check if we're still flying a Sirian ship
      mem.has_sirian_ship = player_has_sirian_ship()

      mem.change = 0
      if mem.wants_sirian and not mem.has_sirian_ship then
         mem.change = 1  -- Bad: they wanted a Sirian ship and you switched on them
         mem.reward = mem.reward / (mem.rank+1.5)
         vntk.msg( _("Altering the deal"), fmt.f( _([["On landing, the passenger gives you a brief glare. "I had paid for transportation in a Sirian ship," they remark. "This alternate arrangement is quite disappointing." They hand you {credits}, but it's definitely less than you were expecting."]]),
            {credits=fmt.credits(mem.reward)} ) )
         player.pay(mem.reward)
         misn.finish(true)
      elseif not mem.wants_sirian and mem.has_sirian_ship then
         mem.change = 2  -- Good: they weren't expecting a Sirian ship, but they got one anyway
      end

      if mem.intime then
         local distbonus = math.max(math.min(mem.numjumps,mem.distbonus_maxjumps)-mem.distbonus_minjumps+1, 0) / 2  -- ranges from 0 (<mem.distbonus_minjumps jumps) to 4 (>=mem.distbonus_maxjumps jumps)
         faction.modPlayerSingle("Sirius", rnd.rnd(distbonus, distbonus+mem.rank+1))

         vntk.msg(_("Successful arrival!"), fmt.f( ferry_land_p2[mem.rank], {passenger=ferry_land_p1[mem.rank]} ) )
      elseif mem.overtime then
         vntk.msg(_("Passenger transport failure"), _("Well, you arrived, but with this late of an arrival, you can't hope for any payment."))
         misn.finish(false)
      else
         -- You were late
         mem.reward = mem.reward / (mem.rank + 1)
         vntk.msg(_("Late arrival"), fmt.f(ferry_land_p3[mem.rank], {passenger=ferry_land_p1[mem.rank], credits=fmt.credits(mem.reward)}))
      end

      if mem.change == 2 then
         -- A little bonus for doing something nice
         faction.modPlayerSingle("Sirius", 1)
         vntk.msg(_("Altering the deal"), _("Since you were unexpectedly able to procure a Sirian ship for the journey, you find a few extra credits tucked in with the fare!"))
         mem.reward = mem.reward * 1.25
      end

      player.pay(mem.reward)
      misn.finish(true)
   elseif mem.timelimit2 <= time.get() then
      -- if we missed the second deadline, drop the person off at the planet.
      vntk.msg(_("Passenger transport failure"), _("You drop the upset pilgrim off at the nearest spaceport."))
      misn.finish(false)
   end
end

-- Date hook
function tick()
   local osd_msg = {}
   osd_msg[1] = fmt.f(_("Fly to {pnt} in the {sys} system before {time}"), {pnt=mem.destplanet, sys=mem.destsys, time=mem.timelimit})
   if mem.timelimit >= time.get() then
      -- Case still in time
      osd_msg[2] = fmt.f(_("You have {time} remaining"), {time=(mem.timelimit - time.get())})
      misn.osdCreate(_("Pilgrimage transport"), osd_msg)
   elseif mem.timelimit2 <= time.get() and not mem.overtime then
      -- Case missed second deadline
      player.msg(_("You're far too late … best to drop your passengers off on the nearest planet before tempers run any higher."))
      misn.osdCreate( _("Pilgrim drop-off"),
         {_("Drop off the pilgrims at the nearest planet")})
      mem.overtime = true
   elseif mem.intime then
      -- Case missed first deadline
      player.msg(_("You've missed the scheduled arrival time! But better late than never…"))
      osd_msg[2] = _("You've missed the scheduled arrival time! But better late than never…")
      misn.osdCreate(_("Pilgrimage transport"), osd_msg)
      mem.intime = false
   end
end

function abort()
   vntk.msg(_("Passenger transport aborted"), fmt.f(_("Informing the pilgrim that their flight to {pnt} has been canceled, you promise to drop them off at the nearest planet."), {pnt=mem.destplanet}))
end
