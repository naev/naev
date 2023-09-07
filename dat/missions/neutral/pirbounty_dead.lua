--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dead Or Alive Bounty">
 <priority>4</priority>
 <cond>
   return require("misn_test").mercenary()
 </cond>
 <chance>360</chance>
 <location>Computer</location>
 <faction>Dvaered</faction>
 <faction>Empire</faction>
 <faction>Frontier</faction>
 <faction>Goddard</faction>
 <faction>Independent</faction>
 <faction>Sirius</faction>
 <faction>Soromid</faction>
 <faction>Za'lek</faction>
 <notes>
  <tier>3</tier>
 </notes>
</mission>
--]]
--[[

   Dead or Alive Pirate Bounty

   Can work with any faction.

--]]
local pir = require "common.pirate"
local fmt = require "format"
local pilotname = require "pilotname"
--local vntk = require "vntk"
local lmisn = require "lmisn"
local bounty = require "common.bounty"

-- Mission details
local misn_title = {
   _("Tiny Dead or Alive Bounty in {sys}"),
   _("Small Dead or Alive Bounty in {sys}"),
   _("Moderate Dead or Alive Bounty in {sys}"),
   _("High Dead or Alive Bounty in {sys}"),
   _("Dangerous Dead or Alive Bounty in {sys}"),
}

local desc_ref = _([[The pirate known as {pirname} was recently seen in the {sys} system. {fct} authorities want this pirate dead or alive. {pirname} is believed to be flying a {shipclass}-class ship. The pirate may disappear if you take too long to reach the {sys} system.

#nTarget:#0 {pirname} ({shipclass}-class ship)
#nWanted:#0 Dead or Alive
#nLast Seen:#0 {sys} system]])

-- luacheck: globals get_faction
function get_faction ()
   return faction.dynAdd( "Pirate", "Wanted Pirate", _("Wanted Pirate"), {clear_enemies=true, clear_allies=true} )
end

-- Set up the ship, credits, and reputation based on the level.
local function bounty_setup ()
   local pship, credits, reputation
   if mem.level == 1 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Hyena"
         credits = 80e3 + rnd.sigma() * 15e3
      else
         pship = "Pirate Shark"
         credits = 100e3 + rnd.sigma() * 30e3
      end
      reputation = 0.5
   elseif mem.level == 2 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Vendetta"
      else
         pship = "Pirate Ancestor"
      end
      credits = 300e3 + rnd.sigma() * 50e3
      reputation = 1
   elseif mem.level == 3 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Admonisher"
      else
         pship = "Pirate Phalanx"
      end
      credits = 500e3 + rnd.sigma() * 80e3
      reputation = 2
   elseif mem.level == 4 then
      if rnd.rnd() < 0.5 then
         pship = "Pirate Starbridge"
      else
         pship = "Pirate Rhino"
      end
      credits = 700e3 + rnd.sigma() * 90e3
      reputation = 2.8
   elseif mem.level == 5 then
      pship = "Pirate Kestrel"
      credits = 1e6 + rnd.sigma() * 100e3
      reputation = 3.5
   end
   return pship, credits, reputation
end

function create ()
   local payingfaction = spob.cur():faction()

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 3,
      function(s)
         return pir.systemPresence( s ) > 0
      end )

   if #systems == 0 then
      -- No pirates nearby
      misn.finish( false )
   end

   local missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys, true ) then misn.finish( false ) end

   local num_pirates = pir.systemPresence( missys )
   if num_pirates <= 50 then
      mem.level = 1
   elseif num_pirates <= 100 then
      mem.level = rnd.rnd( 1, 2 )
   elseif num_pirates <= 200 then
      mem.level = rnd.rnd( 2, 3 )
   elseif num_pirates <= 300 then
      mem.level = rnd.rnd( 3, 4 )
   else
      mem.level = rnd.rnd( 4, 5 )
   end

   -- Pirate details
   local pname = pilotname.pirate()
   local pship, reward, reputation = bounty_setup()

   -- Faction prefix
   local prefix = ""
   if not payingfaction:static() then
      prefix = require("common.prefix").prefix(payingfaction)
   end

   -- Set mission details
   misn.setTitle( prefix..fmt.f(misn_title[mem.level], {sys=missys}) )
   local desc = fmt.f( desc_ref,
      {pirname=pname, sys=missys, fct=payingfaction, shipclass=_(ship.get(pship):classDisplay()) })
   if not payingfaction:static() then
      desc = desc.."\n"..fmt.f(_([[#nReputation Gained:#0 {fct}]]),
         {fct=payingfaction})
   end
   misn.setDesc( desc )
   misn.setReward( reward )

   bounty.init( missys, pname, pship, nil, reward, {
      payingfaction = payingfaction,
      reputation = reputation,
      targetfactionfunc = "get_faction", -- have to pass by name
   } )
end

function accept ()
   misn.accept()
   bounty.accept()
end
