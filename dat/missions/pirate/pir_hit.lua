--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Hit">
 <priority>3</priority>
 <chance>1260</chance>
 <location>Computer</location>
 <cond>
   local pir = require "common.pirate"
   -- Lower probability on non-pirate places
   if not pir.factionIsPirate( spob.cur():faction() ) and rnd.rnd() &lt; 0.5 then
      return false
   end
   return true
 </cond>
 <faction>Wild Ones</faction>
 <faction>Black Lotus</faction>
 <faction>Raven Clan</faction>
 <faction>Dreamer Clan</faction>
 <faction>Pirate</faction>
 <faction>Free Trader</faction>
 <faction>FLF</faction>
 <done>Pirate Hit Intro</done>
 <tags>
  <tag>pir_cap_ch01_med</tag>
 </tags>
</mission>
--]]
--[[
   Pirate Hit

   Unlike Pirate bounties, this mission is kill-only. A nice side-effect
   of that is that you can plunder your target before killing them if
   you manage to board them. >:)
--]]
local pir = require "common.pirate"
local fmt = require "format"
local pilotname = require "pilotname"
local lmisn = require "lmisn"
local bounty = require "common.bounty"
local bhelp = require "events.priority_bounty.helpers"

-- Mission details
local misn_title = {}
misn_title[1] = _("Quick Assassination Job in {sys}")
misn_title[2] = _("Small Assassination Job in {sys}")
misn_title[3] = _("Moderate Assassination Job in {sys}")
misn_title[4] = _("Big Assassination Job in {sys}")
misn_title[5] = _("Dangerous Assassination Job in {sys}")
misn_title[6] = _("Highly Dangerous Assassination Job in {sys}")

local misn_desc = _([[A meddlesome {fct} pilot known as {plt} was recently seen in the {sys} system. Local crime lords want this pilot dead{reason}. {plt} is known to be flying a {shipclass}-class ship. The pilot may disappear if you take too long to reach the {sys} system.{msg}

#nTarget:#0 {plt} ({shipclass}-class ship{escorts})
#nWanted:#0 Dead
#nLast seen:#0 {sys} system
#nTime limit:#0 {deadline}]])

-- Reasons
local reason_list = {
   _(" for excessive amounts of pineapple on pizza"),
   _(" for messing with 'the business'"),
   _(" for cozying up to the wrong pirate clans"),
   _(" for increasing their bribe fees"),
   _(" for being too nosy"),
   _(" for winning in blackjack"),
   _(" for being a beacon of order and peace"),
   _(" for thwarting piracy"),
   _(" for not respecting the pirate code"),
   _(" for throwing out pineapple pizza"),
   _(" for being a tool of bourgeoisie oppression"),
   _(" for putting fellow pirates in prison"),
   _(" for being an asshole"),
   _(" for taking a Pirate Lord's landing spot"),
   _(" for abandoning a pet cat"),
   _(" for using pirates as scapegoats in insurance fraud and not giving a cut of the share"),
   _(" for disrespecting poor people"),
   _(" for fomenting class warfare"),
   _(" for not respecting human freedom"),
   _(" for just being a piece of shit human"),
   _(" for spitting sunflower seeds on the space station floor"),
   _(" for not properly recycling their garbage"),
   _(" for hosting a lame party"),
   _(" for raising tariffs on cheese imports"),
   _(" for making fun of the space pirate shanty"),
}

-- Set up the ship, credits, and reputation based on the level.
local function bounty_setup( targetfct, points )
   local ships, ships_escort
   if targetfct == faction.get("Empire") then
      ships = bhelp.ships.empire
   elseif targetfct == faction.get("Dvaered") then
      ships = bhelp.ships.dvaered
   elseif targetfct == faction.get("Soromid") then
      ships = bhelp.ships.soromid
   elseif mem.target_faction == faction.get("Frontier") then
      ships = bhelp.ships.frontier
   elseif mem.target_faction == faction.get("Sirius") then
      ships = bhelp.ships.sirius
   elseif mem.target_faction == faction.get("Za'lek") then
      ships = bhelp.ships.zalek
   elseif mem.target_faction == faction.get("Trader") then
      ships = bhelp.ships.trader
      ships_escort = bhelp.ships.mercenary
   elseif mem.target_faction == faction.get("Traders Society") then
      ships = bhelp.ships.trader
   elseif mem.target_faction == faction.get("Independent") then
      ships = bhelp.ships.mercenary
   else
      error(fmt.f("pir_hit: unknown faction {fct}", {fct=targetfct}))
   end

   local pilots = bounty.choose_ships_from_points( ships, points )
   -- If we use separate escorts, make sure they get added after the main pilot
   if ships_escort then
      local capship = pilots[1]
      pilots = bhelp.choose_ships_from_points_and_capship( capship, ships_escort, points-capship:points() )
      table.insert( pilots, 1, capship )
   end
   points = bounty.fleet_points( pilots )

   local level
   if points <= 50 then
      level = 1
   elseif points <= 100 then
      level = 2
   elseif points <= 200 then
      level = 3
   elseif points <= 300 then
      level = 4
   else
      level = 5
   end

   local calcpoints  = points / 200
   if points > 200 then
      calcpoints = 1 + (calcpoints - 1) * 0.5
   end
   local credits     = 1e6 * calcpoints * (0.9 + 0.2 * rnd.rnd())
   local reputation  = 30  * calcpoints

   -- Get reason
   local reason = reason_list[ rnd.rnd(#reason_list) ]

   local escorts = ""
   if #pilots > 1 then
      local num = #pilots-1
      escorts = fmt.f(n_(", with {num} escort", ", with {num} escorts", num), {
         num = num
      })
   end

   return {
      level       = level,
      name        = pilotname.generic(),
      ships       = pilots,
      reward      = credits,
      reputation  = reputation,
      reason      = reason,
      points      = points,
      escorts     = escorts,
   }
end

function create ()
   -- Determine paying faction probabilistic
   mem.paying_faction = pir.systemClanP( system.cur() )
   local faction_text = pir.reputationMessage( mem.paying_faction )

   local TARGET_FACTIONS = {
      "Independent",
      "Dvaered",
      "Empire",
      "Frontier",
      "Independent",
      "Sirius",
      "Soromid",
      "Trader",
      "Traders Society",
      "Za'lek",
   }

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 6,
      function(s)
         for i, j in ipairs(TARGET_FACTIONS) do
            local p = s:presences()[j]
            if p ~= nil and p > 0 then
               return true
            end
         end
         return false
      end, nil, true )

   if #systems == 0 then
      -- No enemy presence nearby
      misn.finish( false )
   end

   mem.missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( mem.missys, true ) then misn.finish( false ) end

   mem.target_faction = nil
   local presences = mem.missys:presences()
   while mem.target_faction == nil and #TARGET_FACTIONS > 0 do
      local i = rnd.rnd(  #TARGET_FACTIONS )
      local p = presences[ TARGET_FACTIONS[i] ]
      if p ~= nil and p > 0 then
         mem.target_faction = TARGET_FACTIONS[i]
      else
         for j = i, #TARGET_FACTIONS do
            TARGET_FACTIONS[j] = TARGET_FACTIONS[j + 1]
         end
      end
   end

   if mem.target_faction == nil then
      -- Should not happen, but putting this here just in case.
      misn.finish( false )
   end
   mem.target_faction = faction.get(mem.target_faction)

   mem.deadline = time.new( 0, 2 * system.cur():jumpDist(mem.missys, true), rnd.rnd( 100e3, 150e3 ) )

   mem.name = pilotname.generic()
   local var = var.peek("pirate_bounty_points") or 0
   local points = 30 + (200 + math.min(500, 0.5*var)) * rnd.rnd()
   local target = bounty_setup( mem.target_faction, points )

   -- Set mission details
   local title = fmt.f( misn_title[target.level], {
      sys = mem.missys,
   } )
   -- Faction prefix
   if not mem.paying_faction:static() then
      local prefix = require("common.prefix").prefix(mem.paying_faction)
      title = prefix..title
   end

   local mdesc = fmt.f( misn_desc, {
      fct   = mem.target_faction,
      plt   = mem.name,
      sys   = mem.missys,
      shipclass = _(ship.get(target.ships[1]):classDisplay()),
      msg   = faction_text,
      reason= target.reason,
      deadline = mem.deadline,
      escorts = target.escorts,
   } )
   if pir.factionIsClan(mem.paying_faction) then
      mdesc = mdesc.."\n"..fmt.f(_([[#nReputation Gained:#0 {fct}]]),
         {fct=mem.paying_faction})
   end
   if not pir.factionIsPirate( spob.cur():faction() ) then
      -- We're not on a pirate stronghold, so include a clear warning that the
      -- mission is in fact illegal.
      mdesc = mdesc .. "\n\n" .. _("#rWARNING:#0 This mission is illegal and will get you in trouble with the authorities!")
      misn.setIllegal(true)
   end
   misn.setDesc( mdesc )

   misn.setTitle( title )
   misn.setDesc( mdesc )
   misn.setReward( target.reward )
   misn.setDistance( lmisn.calculateDistance( system.cur(), spob.cur():pos(), mem.missys) )
   mem.deadline = time.cur() + mem.deadline

   bounty.init( mem.missys, target.name, target.ships, target.reward, {
      trackingvar       = { "pirate_bounty_points", target.points },
      payingfaction     = mem.paying_faction,
      reputation        = target.reputation,
      targetfaction     = mem.target_faction,
      alive_only        = false,
      deadline          = mem.deadline,
      deathfunc         = "finish",
      boardfunc         = "finish",
   } )
end

-- Succeed the mission
-- luacheck: globals finish
function finish( b )
   lmisn.sfxMoney()
   player.msg( "#g"..fmt.f(_("MISSION SUCCESS! Pay of {credits} has been transferred into your account."),{
      credits = fmt.credits(b.reward)
   }).."#0" )
   player.pay( b.reward )
   if b.reputation then
      b.payingfaction:hit( b.reputation )
      pir.reputationNormalMission( b.reputation )
   end
   if b.trackingvar then
      local v = var.peek( b.trackingvar[1] ) or 0
      var.push( b.trackingvar[1], v+b.trackingvar[2] )
   end
   misn.finish( true )
end

function accept ()
   bounty.accept()
end
