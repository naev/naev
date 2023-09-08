--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Bounty">
 <priority>4</priority>
 <cond>
   return require("misn_test").mercenary()
 </cond>
 <chance>660</chance>
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

   Bounty mission to take out a pirate.
   Can either capture or dead/alive mission.

   Can work with any faction.

--]]
local pir = require "common.pirate"
local fmt = require "format"
local pilotname = require "pilotname"
local lmisn = require "lmisn"
local bounty = require "common.bounty"

-- Case target can be dead or alive
local misn_title = {
   _("Tiny Dead or Alive Bounty in {sys}"),
   _("Small Dead or Alive Bounty in {sys}"),
   _("Moderate Dead or Alive Bounty in {sys}"),
   _("High Dead or Alive Bounty in {sys}"),
   _("Dangerous Dead or Alive Bounty in {sys}"),
}
local misn_desc = _([[The pirate known as {pirname} was recently seen in the {sys} system. {fct} authorities want this pirate dead or alive{reason}. {pirname} is believed to be flying a {shipclass}-class ship. The pirate may disappear if you take too long to reach the {sys} system.

#nTarget:#0 {pirname} ({shipclass}-class ship)
#nWanted:#0 Dead or Alive
#nLast Seen:#0 {sys} system]])

-- In the case the player has to capture the target alive
local misn_title_alive = {
   _("Tiny Alive Bounty in {sys}"),
   _("Small Alive Bounty in {sys}"),
   _("Moderate Alive Bounty in {sys}"),
   _("High Alive Bounty in {sys}"),
   _("Dangerous Alive Bounty in {sys}"),
}
local misn_desc_alive = _([[The pirate known as {pirname} was recently seen in the {sys} system. {fct} authorities want this pirate alive{reason}. {pirname} is believed to be flying a {shipclass}-class ship. The pirate may disappear if you take too long to reach the {sys} system.

#nTarget:#0 {pirname} ({shipclass}-class ship)
#nWanted:#0 Alive
#nLast Seen:#0 {sys} system]])

local reason_list = {
   ["Independent"] = {
      _([[ for attempting to undermine the integry of {spb}]]),
      _([[ for endangering the peace of {spb}]]),
      _([[ for trafficking of weapons]]),
      _([[ for arms dealing]]),
      _([[ for armed robbery]]),
      _([[ for hijacking]]),
      _([[ for illegal waste disposal]]),
      _([[ for attempted sabotage]]),
   },
   ["Empire"] = {
      _([[ for bureaucratic infringements]]),
      _([[ for submitting erroneous paperwork]]),
      _([[ for failing to file taxes in a duly manner]]),
      _([[ for insulting the Emperor's pet iguana]]),
      _([[ for not declaring bribes in their tax statements]]),
      _([[ for spreading lies about the Emperor]]),
      _([[ for filing bureaucratic documents in the incorrect order]]),
      _([[ for attempting to sell new clothes to the Emperor]]), -- reference to the Emperor's new clothes
      _([[ for filing excessive tax deductions]]),
   },
   ["Za'lek"] = {
      _([[ for operating an illegal scientific paper mill]]),
      _([[ for creating a large scale botnet]]),
      _([[ for forging the signature of a principal investigator]]),
      _([[ for fraudulent use of research funds]]),
      _([[ for failing to comply with research ethics course]]),
      _([[ for not adding the principal investigator as a co-author on a scientific manuscript]]),
      _([[ for sending spam e-mails]]),
      _([[ for repeatedly crashing a supercomputer cluster with spaghetti code]]),
      _([[ for software piracy]]),
      _([[ for fake NP=P proofs]]),
      _([[ for disbelief of science]]),
      _([[ for not respecting the scientific method]]),
   },
   ["Dvaered"] = {
      _([[ for running away from a duel to the death]]),
      _([[ for cheating in Mace Rocket Ballet]]),
      _([[ for disrespecting their local Warlord]]),
      _([[ for insufficient violence]]),
      _([[ for cowardly behaviour]]),
      _([[ for slapping a commanding officer with a sausage]]),
      _([[ for insubordination]]),
   },
   ["Soromid"] = {
      _([[ for dishonouring their tribe]]),
      _([[ for insulting their ancestors]]),
      _([[ for contempt by purposely using incorrect pronouns]]),
      _([[ for abusing bioships]]),
      _([[ for dangerous and volatile biological modifications]]),
      _([[ for developing dangerous bioweapons]]),
      _([[ for disrespecting Soromid history]]),
      _([[ for insulting biological modifications]]),
      _([[ for denouncing the theory of evolution]]),
   },
   ["Sirius"] = {
      _([[ for blasphemy]]),
      _([[ for disrespecting the Serra echelon]]),
      _([[ for abuse of psychic powers]]),
      _([[ for attempting to sabotage an obelisk]]),
      _([[ for trying to breakdance in a meditation chamber]]),
      _([[ for trafficking of pilgrims]]),
      _([[ for posing as a member of another echelon]]),
      _([[ for disrupting the harmony of House Sirius]]),
   },
}
reason_list["Goddard"] = reason_list["Independent"] -- TODO someday

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

   -- See if alive only
   local title, desc = misn_title, misn_desc
   local osd_objective
   local alive_only = (rnd.rnd() > 0.5)
   if alive_only then
      reward = reward * 1.5
      reputation = reputation * 1.5
      title = misn_title_alive
      desc = misn_desc_alive
      osd_objective = _("Capture {plt}")
   end

   -- Reason for the bounty
   local reason = ""
   local reasons = reason_list[ payingfaction:name() ]
   if reasons then
      reason = fmt.f( reasons[ rnd.rnd(1,#reasons) ], {spb=spob.cur()} )
   end

   -- Faction prefix
   local prefix = ""
   if not payingfaction:static() then
      prefix = require("common.prefix").prefix(payingfaction)
   end

   -- Set mission details
   misn.setTitle( prefix..fmt.f(title[mem.level], {sys=missys}) )
   local mdesc = fmt.f( desc,
      {pirname=pname, sys=missys, fct=payingfaction, shipclass=_(ship.get(pship):classDisplay()), reason=reason })
   if not payingfaction:static() then
      mdesc = mdesc.."\n"..fmt.f(_([[#nReputation Gained:#0 {fct}]]),
         {fct=payingfaction})
   end
   misn.setDesc( mdesc )
   misn.setReward( reward )

   bounty.init( missys, pname, pship, nil, reward, {
      payingfaction     = payingfaction,
      reputation        = reputation,
      targetfactionfunc = "get_faction", -- have to pass by name
      alive_only        = alive_only,
      osd_objective     = osd_objective,
   } )
end

function accept ()
   misn.accept()
   bounty.accept()
end
