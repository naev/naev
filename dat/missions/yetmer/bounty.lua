--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Yetmer Bounty">
 <priority>4</priority>
 <cond>
   return require("misn_test").mercenary()
 </cond>
 <chance>260</chance>
 <location>Computer</location>
 <faction>Yetmer</faction>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   Bounty mission to take out a pirate.

--]]
local fmt = require "format"
local pilotname = require "pilotname"
local pilotai = require "pilotai"
local lmisn = require "lmisn"
local bounty = require "common.bounty"

-- Case target can be dead or alive
local misn_title = {
   _("Eliminate O'rez Traitor in {sys}"),
}
local misn_desc = _([[An O'rez traitor known as {name} has gained notoriety through violent acts against House Yetmer in the {sys} system. Their elimination will help the war efforts to triumph over the traitors.

#nTarget:#0 {pilotname} ({shipclass}-class ship{escorts})
#nWanted:#0 Dead
#nLast seen:#0 {sys} system
#nTime limit:#0 {deadline}
#nReputation Gained:#0 {fct}]])

local payingfaction = faction.get("Yetmer")
local targetfaction = faction.get("O'rez")
local missys = system.get("Yetmer-O'rez Highspace")
local jumpa = jump.get( missys, system.get("K'tos") )
local jumpb = jump.get( missys, system.get("Mayla") )
local cpos  = (jumpa:pos() + jumpb:pos())*0.5 * 0.7

   -- TODO update when they get their own ships
local OREZ_SHIPS = {
   ship.get("Lancelot"),
   ship.get("Admonisher"),
   ship.get("Hawking"),
}

-- Set up the ship, credits, and reputation based on the level.
local function bounty_setup ( points )
   local ships = bounty.choose_ships_from_points( OREZ_SHIPS, points )
   points = bounty.fleet_points( ships ) -- Update points

   local level
   if points <= 10 then
      level = 1
   elseif points <= 20 then
      level = 2
   elseif points <= 40 then
      level = 3
   elseif points <= 60 then
      level = 4
   else
      level = 5
   end

   local calcpoints  = points / 40
   if points > 40 then
      calcpoints = 1 + (calcpoints - 1) * 0.5
   end
   local credits     = 1e6 * calcpoints * (0.9 + 0.2 * rnd.rnd())
   local reputation  = 30  * calcpoints

   return {
      ships       = ships,
      credits     = credits,
      reputation  = reputation,
      level       = level,
   }
end

function create ()
   if not misn.claim( missys, true ) then misn.finish( false ) end

   -- Enemy details
   local pname = pilotname.generic() -- TODO something better?
   local points = 100 + rnd.rnd() * 500
   local target = bounty_setup( points )
   local title, desc = misn_title[rnd.rnd(1,#misn_title)], misn_desc

   -- Faction prefix
   local prefix = ""
   if not payingfaction:static() then
      prefix = require("common.prefix").prefix(payingfaction)
   end

   mem.level = target.level
   mem.missys = missys
   mem.deadline = time.cur() + time.new( 0, 2 * system.cur():jumpDist(mem.missys, true), rnd.rnd( 100e3, 150e3 ) )

   -- Set mission details
   local escorts = ""
   if #target.ships > 1 then
      local num = #target.ships-1
      escorts = fmt.f(n_(", with {num} escort", ", with {num} escorts", num), {
         num = num
      })
   end
   misn.setTitle( prefix..fmt.f(title, {sys=missys}) )
   local mdesc = fmt.f( desc, {
      pilotname   = pname,
      sys         = missys,
      fct         = payingfaction,
      shipclass   = _(ship.get(target.ships[1]):classDisplay()),
      deadline    = (mem.deadline-time.cur()),
      escorts     = escorts,
   })
   misn.setDesc( mdesc )
   misn.setReward( target.credits )
   misn.setDistance( lmisn.calculateDistance( system.cur(), spob.cur():pos(), missys) )

   bounty.init( missys, pname, target.ships, target.credits, {
      payingfaction     = payingfaction,
      targetfaction     = targetfaction,
      spawnfunc         = "spawn_target",
      reputation        = target.reputation,
      deadline          = mem.deadline,
   } )
end

function accept ()
   bounty.accept()
end

-- luacheck: globals spawn_target
function spawn_target( b, _params )
   -- Fuzzes the position a bit
   local function fuzz( pos )
      return (pos+vec2.newP( rnd.rnd()*500, rnd.angle() )) * (1 - 0.2*rnd.rnd())
   end

   local pos = fuzz( (jumpa:pos() + jumpb:pos())*0.5 )

   local target_ship
   local target = {}
   local fct = bounty.get_faction()
   for k,s in ipairs(b.targetship) do
      local p = pilot.add( s, fct, pos )
      p:setHostile(true)
      local aimem = p:memory()
      aimem.defensive   = true -- Always try to be defensive
      aimem.loiter      = math.huge -- Should make them loiter forever
      aimem.capturable  = true
      if not target_ship then
         target_ship = p
         p:rename( b.targetname )
         -- Make esaier to spot but not fight
         p:intrinsicSet( "ew_detected", 50 )
      else
         p:setLeader( target_ship )
      end
      table.insert( target, p )
   end

   -- Patrol
   pilotai.patrol( target_ship, {
      fuzz(jumpa:pos()),
      fuzz(jumpb:pos()),
      fuzz(cpos),
   } )
   target_ship:setNoDisable(true)

   return target_ship
end
