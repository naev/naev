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
   Can either capture or dead/alive mission.

   Can work with any faction.

--]]
local fmt = require "format"
local pilotname = require "pilotname"
local lmisn = require "lmisn"
local bounty = require "common.bounty"

-- Case target can be dead or alive
local misn_title = {
   _("Eliminate O'rez Traitor in {sys}"),
}
local misn_desc = _([[An O'rez traitor known as {name} has gained notoriety through violent acts against House Yetmer in the {sys} system. Their elimination will help the war efforts to triumph over the traitors.

#nTarget:#0 {name} ({shipclass}-class ship)
#nWanted:#0 Dead
#nLast seen:#0 {sys} system
#nTime limit:#0 {deadline}
#nReputation Gained:#0 {fct}]])

-- Set up the ship, credits, and reputation based on the level.
local function bounty_setup ()
   local pship, credits, reputation
   -- TODO update when they get their own ships
   pship = "Hawking"
   credits = 1e6 + rnd.sigma() * 100e3
   reputation = 3.5
   return pship, credits, reputation
end

function create ()
   local payingfaction = faction.get("Yetmer")
   local missys = system.get("Yetmer-O'rez Highspace")
   if not misn.claim( missys, true ) then misn.finish( false ) end

   -- Pirate details
   local pname = pilotname.generic() -- TODO something better?
   local pship, reward, reputation = bounty_setup()
   local title, desc = misn_title[rnd.rnd(1,#misn_title)], misn_desc

   -- Faction prefix
   local prefix = ""
   if not payingfaction:static() then
      prefix = require("common.prefix").prefix(payingfaction)
   end

   mem.missys = missys
   mem.deadline = time.get() + time.new( 0, 2 * system.cur():jumpDist(mem.missys, true), rnd.rnd( 100e3, 150e3 ) )

   -- Set mission details
   misn.setTitle( prefix..fmt.f(title, {sys=missys}) )
   local mdesc = fmt.f( desc,
      {name=pname, sys=missys, fct=payingfaction, shipclass=_(ship.get(pship):classDisplay()), deadline=(mem.deadline-time.get()) })
   misn.setDesc( mdesc )
   misn.setReward( reward )
   misn.setDistance( lmisn.calculateDistance( system.cur(), spob.cur():pos(), missys) )

   bounty.init( missys, pname, pship, reward, {
      targetfaction     = faction.get("O'rez"),
      payingfaction     = payingfaction,
      reputation        = reputation,
      deadline          = mem.deadline,
   } )
end

function accept ()
   bounty.accept()
end
