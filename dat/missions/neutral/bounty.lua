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
--local var = require "shipvariants"

-- Case target can be dead or alive
local misn_title = {
   _("Tiny Dead or Alive Bounty in {sys}"),
   _("Small Dead or Alive Bounty in {sys}"),
   _("Moderate Dead or Alive Bounty in {sys}"),
   _("High Dead or Alive Bounty in {sys}"),
   _("Dangerous Dead or Alive Bounty in {sys}"),
}
local misn_desc = _([[The {pilotnoun} known as {pilotname} was recently seen in the {sys} system. {fct} authorities want {pilotname} {deadoralive}{reason}. {pilotname} is believed to be flying a {shipclass}-class ship. {pilotname} may disappear if you take too long to reach the {sys} system.

#nTarget:#0 {pilotname} ({shipclass}-class ship{escorts})
#nWanted:#0 {wanted}
#nLast seen:#0 {sys} system
#nTime limit:#0 {deadline}]])

-- In the case the player has to capture the target alive
local misn_title_alive = {
   _("Tiny Alive Bounty in {sys}"),
   _("Small Alive Bounty in {sys}"),
   _("Moderate Alive Bounty in {sys}"),
   _("High Alive Bounty in {sys}"),
   _("Dangerous Alive Bounty in {sys}"),
}

local reason_list = {
   ["Independent"] = {
      _([[ for attempting to undermine the integrity of {spb}]]),
      _([[ for endangering the peace of {spb}]]),
      _([[ for trafficking of weapons]]),
      _([[ for arms dealing]]),
      _([[ for armed robbery]]),
      _([[ for hijacking]]),
      _([[ for illegal waste disposal]]),
      _([[ for attempted sabotage]]),
      _([[ for espionage against {spb}]]),
      _([[ for inciting riots in {spb}]]),
      _([[ for smuggling contraband into {spb}]]),
      _([[ for arson in a restricted area]]),
      _([[ for unauthorized access to restricted systems]]),
      _([[ for theft of sensitive information]]),
      _([[ for vandalism of critical infrastructure]]),
      _([[ for conspiracy to commit treason]]),
      _([[ for illegal surveillance of {spb} citizens]]),
      _([[ for unauthorized use of advanced technology]]),
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
      _([[ for falsifying Imperial Decrees]]),
      _([[ for embezzling funds from the Imperial Treasury]]),
      _([[ for misrepresenting the Emperor's will to a Great House]]),
      _([[ for failing to report a minor rebellion to the Imperial Council]]),
      _([[ for accepting bribes from a Minor House]]),
      _([[ for losing critical documents in the Imperial Archives]]),
      _([[ for misallocating resources to a favoured Imperial project]]),
      _([[ for conspiring with a Great House to undermine the Emperor's authority]]),
      _([[ for submitting fraudulent expense reports to the Imperial Council]]),
      _([[ for neglecting to perform a mandatory Imperial audit]]),
      _([[ for falsifying a noble's pedigree]]),
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
      _([[ for falsifying research data to support a pet theory]]),
      _([[ for stealing intellectual property from a fellow researcher]]),
      _([[ for conducting human experimentation without approval from the ethics board]]),
      _([[ for misusing research funds to finance a personal project]]),
      _([[ for failing to properly cite sources in a scientific paper]]),
      _([[ for sabotaging a rival researcher's experiment]]),
      _([[ for creating a fake research institute to launder funds]]),
      _([[ for using Za'lek resources to develop a personal AI assistant]]),
      _([[ for neglecting to follow proper laboratory safety protocols]]),
      _([[ for attempting to patent a scientific concept that is already widely known]]),
   },
   ["Dvaered"] = {
      _([[ for running away from a duel to the death]]),
      _([[ for cheating in Mace Rocket Ballet]]),
      _([[ for disrespecting their local Warlord]]),
      _([[ for insufficient violence]]),
      _([[ for cowardly behaviour]]),
      _([[ for slapping a commanding officer with a sausage]]),
      _([[ for insubordination]]),
      _([[ for unauthorized use of a gravity anchor]]),
      _([[ for stealing a superior's lunch]]),
      _([[ for conducting a duel in a restricted area]]),
      _([[ for failure to provide adequate sacrifices to the Battle Gods]]),
      _([[ for misusing a teleportation device]]),
      _([[ for impersonating a high-ranking officer]]),
      _([[ for smuggling a pet into a war zone]]),
      _([[ for refusing to participate in a battle chant]]),
      _([[ for dishonouring the Dvaered Catalogue of Military Honours]]),
      _([[ for failing to uphold the values of strength, honour, valour, perseverance, and obedience]]),
      _([[ for treason against the Dvaered High Command]]),
      _([[ for conspiring against a Warlord and losing]]),
      _([[ for hoarding resources meant for the war effort]]),
      _([[ for deserting a key battle station]]),
      _([[ for spreading dissent among the ranks]]),
      _([[ for de-escalating a situation that required escalation]]),
      _([[ for impeccable hygiene]]),
      _([[ for clogging the commander's toilet]]),
   },
   ["Soromid"] = {
      _([[ for dishonouring their tribe]]),
      _([[ for insulting their ancestors]]),
      _([[ for deliberate misgendering]]),
      _([[ for abusing bioships]]),
      _([[ for dangerous and volatile biological modifications]]),
      _([[ for developing dangerous bioweapons]]),
      _([[ for disrespecting Soromid history]]),
      _([[ for insulting biological modifications]]),
      _([[ for denouncing the theory of evolution]]),
      _([[ for interfering with freedom of form]]),
      _([[ for practising conversion therapy]]),
      _([[ for disrespecting the sacrifices of the Soromid tribes]]),
      _([[ for spreading misinformation about the Soromid]]),
      _([[ for attempting to misuse genetic data]]),
      _([[ for conspiring against the tribe]]),
      _([[ for hoarding resources meant for the tribe]]),
      _([[ for deserting a key research station]]),
      _([[ for attempting to consolidate political power]]),
      _([[ for attempting to create artificial life]]),
      _([[ for biohacking without consent]]),
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
      _([[ for desecrating a sacred obelisk]]),
      _([[ for spreading heresy about the Sirichana's reincarnations]]),
      _([[ for impersonating a member of the theocratic government]]),
      _([[ for disrupting the harmony of Crater City]]),
      _([[ for attempting to steal a relic from the Tower of the Sirichana]]),
      _([[ for practising dark magic in a meditation chamber]]),
      _([[ for conspiring against an Arch-Canter]]),
      _([[ for vandalizing a sacred text]]),
      _([[ for attempting to flee the echelon system]]),
      _([[ for speaking ill of the 34th Emperor's decision to grant House Sirius autonomy]]),
   },
   ["Goddard"] = {
      _([[ for refusing to supply the Empire with Goddard battlecruisers during a time of war]]),
      _([[ for attempting to reverse-engineer Imperial technology]]),
      _([[ for disrespecting the memory of Eduard Goddard]]),
      _([[ for spreading misinformation about the capabilities of the Goddard battlecruiser]]),
      _([[ for attempting to steal Goddard family secrets]]),
      _([[ for conspiring against the current leader of House Goddard]]),
      _([[ for hoarding resources meant for the people of Zhiru]]),
      _([[ for spreading dissent among the Goddard family ranks]]),
      _([[ for refusing to pay taxes to the Empire]]),
      _([[ for attempting to create a rival to the Goddard battlecruiser]]),
      _([[ for disrespecting the Imperial navy]]),
      _([[ for spreading false rumours about the Goddard family]]),
      _([[ for attempting to sabotage Goddard battlecruiser production]]),
      _([[ for conspiring with rival Houses against the Empire]]),
      _([[ for deserting a key diplomatic mission]]),
      _([[ for spreading dissent among the people of Zhiru]]),
      _([[ for refusing to provide aid to the Empire during a natural disaster]]),
      _([[ for deserting a key Goddard production line]]),
   },
}

local function fleet_points( fleet )
   local points = 0
   for k,s in ipairs(fleet) do
      points = points + s:points()
   end
   return points
end

-- Tries to find a set of ships given a number of points
local function choose_ships_from_points( shiplist, points )
   -- Candidate ships
   local maybeship = {}
   local maybecap = {}
   for k,v in ipairs(shiplist) do
      local p = v:points()
      if p < points then
         table.insert( maybeship, v )
         if p > points*0.5 then
            table.insert( maybecap, v )
         end
      end
   end
   if #maybeship <= 0 then
      table.sort( shiplist, function( a, b ) return a:points() < b:points() end )
      return {shiplist[1]}
   end
   table.sort( maybeship, function( a, b ) return a:points() > b:points() end )

   -- Force top three ships to be considered capitals
   if #maybecap <= 0 then
      maybecap = { maybeship[1], maybeship[2], maybeship[3] }
   end

   -- Choose capship
   local cap = maybecap[ rnd.rnd(#maybecap) ]
   points = points - cap:points()
   local smallest = maybeship[ #maybeship ]:points()
   if points < smallest then return {cap} end

   -- Must be smaller than capship
   local cappoints = cap:points()
   local newships = {}
   for k,s in ipairs(maybeship) do
      if s:points() < cappoints then
         table.insert( newships, s )
      end
   end
   maybeship = newships

   -- Other ships have to be smaller
   local ships = {cap}
   while points >= smallest do
      local candidates = rnd.permutation( maybeship )
      local s
      local id = 1
      repeat
         s = candidates[id]
         if not s then return ships end
         id = id+1
      until s:points() < points
      table.insert( ships, s )
      points = points - s:points()
   end

   return ships
end

-- Set up the ship, credits, and reputation based on the level.
local function bounty_setup_pirate( payingfaction, points )
   local PIRATE_SHIPS = {
      ship.get("Pirate Hyena"),
      ship.get("Pirate Shark"),
      ship.get("Pirate Vendetta"),
      ship.get("Pirate Ancestor"),
      ship.get("Pirate Admonisher"),
      ship.get("Pirate Revenant"),
      ship.get("Pirate Phalanx"),
      ship.get("Pirate Starbridge"),
      ship.get("Pirate Rhino"),
      ship.get("Pirate Kestrel"),
      ship.get("Pirate Zebra"),
      ship.get("Dealbreaker"),
   }
   local fpir = faction.get("Pirate")

   local ships = choose_ships_from_points( PIRATE_SHIPS, points )
   points = fleet_points( ships ) -- Update points
   -- TODO swap variants in there

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 3,
      function(s)
         -- Must be claimable
         if not naev.claimTest( s, true ) then
            return false
         end
         -- More likely to only appear in empty systems with high points
         if rnd.rnd() < points/200 then
            for k,spb in ipairs(s:spobs()) do
               if spb:services().inhabited then
                  local f = spb:faction()
                  if not pir.factionIsPirate(f) and fpir:areEnemies(payingfaction) then
                     return false
                  end
               end
            end
         end
         return pir.systemPresence( s ) > points
      end )

   if #systems == 0 then
      -- No pirates nearby
      return
   end

   local missys = systems[ rnd.rnd( 1, #systems ) ]

   local level
   local num_pirates = pir.systemPresence( missys )
   if points <= 50 then
      level = 1
   elseif points <= 100 then
      level = 2
   elseif num_pirates <= 200 then
      level = 3
   elseif num_pirates <= 300 then
      level = 4
   else
      level = 5
   end

   local credits     = 1e6 * points / 200 * (0.9 + 0.2 * rnd.rnd())
   local reputation  = 30  * points / 200

   -- Reason for the bounty
   local reason = ""
   local reasons = reason_list[ payingfaction:nameRaw() ]
   if reasons then
      reason = fmt.f( reasons[ rnd.rnd(1,#reasons) ], {spb=spob.cur()} )
   end

   return {
      level       = level,
      system      = missys,
      name        = pilotname.pirate(),
      ships       = ships,
      reward      = credits,
      reputation  = reputation,
      faction     = faction.get("Pirate"),
      alive_only  = rnd.rnd() > 0.5,
      reason      = reason,
   }
end

local function bounty_setup( payingfaction, points )
   local target =  bounty_setup_pirate( payingfaction, points )
   if not target then
      return
   end

   -- Comput escorts
   local escorts = ""
   if #target.ships > 1 then
      local num = #target.ships-1
      escorts = fmt.f(n_(", with {num} escort", ", with {num} escorts", num), {
         num = num
      })
   end

   local title
   if target.alive_only then
      target.reward        = 1.4 * target.reward
      target.reputation    = 1.5 * target.reputation
      target.osd_objective = _("Capture {plt}")
      title = misn_title_alive
   else
      title = misn_title
   end
   target.title = fmt.f( title[target.level], {sys=target.system} )

   -- Faction prefix
   if not payingfaction:static() then
      local prefix = require("common.prefix").prefix(payingfaction)
      target.title = prefix..target.title
   end

   -- Some common stuff
   target.deadline = time.new( 0, 2 * system.cur():jumpDist(target.system, true), rnd.rnd( 100e3, 150e3 ) )
   target.desc = fmt.f( misn_desc, {
      pilotname= target.name,
      sys      = target.system,
      fct      = payingfaction,
      shipclass=_(ship.get(target.ships[1]):classDisplay()),
      escorts  = escorts,
      reason   = target.reason,
      deadoralive = (target.alive_only and _("alive")) or _("dead or alive"),
      wanted   = (target.alive_only and _("Alive")) or _("Dead or Alive"),
      pilotnoun= _("pirate"),
      deadline = target.deadline
   })
   if (target.reputation or 0) > 0 and not payingfaction:static() then
      target.desc = target.desc.."\n"..fmt.f(_([[#nReputation Gained:#0 {fct}]]),
         {fct=payingfaction})
   end

   -- Return value
   return target
end

function create ()
   local payingfaction = spob.cur():faction()

   -- Pirate details
   local target = bounty_setup( payingfaction, 30 + 300 * math.sqrt(rnd.rnd()) )
   if not target then
      -- Unable to find a target
      misn.finish(false)
   end
   if not misn.claim( target.system, true ) then return end

   mem.missys = target.system
   mem.deadline = time.cur() + target.deadline

   misn.setTitle( target.title )
   misn.setDesc( target.desc )
   misn.setReward( target.reward )
   misn.setDistance( lmisn.calculateDistance( system.cur(), spob.cur():pos(), mem.missys) )

   bounty.init( mem.missys, target.name, target.ships, target.reward, {
      payingfaction     = payingfaction,
      reputation        = target.reputation,
      targetfaction     = target.faction,
      dynamicfaction    = true,
      alive_only        = target.alive_only,
      osd_objective     = target.osd_objective,
      deadline          = mem.deadline,
   } )

end

function accept ()
   bounty.accept()
end
