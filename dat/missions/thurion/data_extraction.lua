--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Thurion Data Extraction">
 <priority>3</priority>
 <location>Computer</location>
 <chance>270</chance>
 <cond>
   if system.cur():reputation("Thurion") &lt; 0 then
      return false
   end

   local sc = spob.cur()
   if sc~=spob.get("Tenal-P") and sc~=spob.get("Cerberus Outpost") then
      return false
   end

   return true
 </cond>
 <done>Android Abroad</done>
</mission>
--]]
--[[
   Just used to create an OSD. Doesn't really affect anything.
--]]
local fmt = require "format"
local vntk = require "vntk"
local lmisn = require "lmisn"
local thurion = require "common.thurion"

mem.state = 0

function create ()
   local validfcts = {
      faction.get("Independent"),
      faction.get("Empire"),
      faction.get("Dvaered"),
      faction.get("Traders Society"),
   }
   local candidates = lmisn.getSpobAtDistance( nil, 5, 10, "Independent", false, function (s)
      local f = s:faction()
      for k,v in ipairs(validfcts) do
         if f==v then
            return true
         end
      end
      return false
   end )
   if #candidates <= 0 then
      misn.finish(false)
   end

   mem.targetspb = candidates[ rnd.rnd(1,#candidates) ]
   mem.targetsys = mem.targetspb:system()
   local dist = mem.targetsys:jumpDist()
   mem.returnspb, mem.returnsys = spob.cur()
   mem.amount = rnd.rnd(8,24)
   mem.reward = 100e3 + 50e3 * dist + 3e3 * mem.amount
   mem.reward = mem.reward * (1 + 0.1*rnd.sigma())
   mem.reputation = math.floor( 2 + dist/5 + mem.amount/24 + 0.5 )

   -- TODO more custom ridiculous message about types of documents and such
   local cargos = {
      { N_("Confidential Documents"),
         N_("A serious of notes and documents collected by Thurion agents.") },
      { N_("Situational Reports"),
         N_("Detailed reports covering different aspects of systems outside of the Nebula, mixed in with stolen documents and illegal recordings.") }
   }
   local fspb = mem.targetspb:faction()
   if fspb==faction.get("Empire") then
      tmergei( cargos, {
         { N_("Imperial Census Data"),
            N_("Private data regarding habits and behaviours of the Citizens of the Empire.") },
         { N_("Imperial Gift Log"),
            N_("Data regarding gifts given and received by the different members of the Imperial Aristocracy, paid by the Imperial Coffers.") },
         { N_("Aristocratic Secrets"),
            N_("Detailed information about the likes, dislikes, and personalities of various important figures in the Empire.") },
         { N_("Great House Liaison Logs"),
            N_("Logs regarding the behaviours and details of interactions between the Great House Liaisons and the Empire.") },
      } )
   end
   if fspb==faction.get("Dvaered") then
      tmergei( cargos, {
         { N_("Dvaered Ship Census"),
            N_("Information collected about ship movements and activities around Dvaered space.") },
         { N_("Dvaered Economic Data"),
            N_("Data regarding the poor economic outlook of the Dvared territories, where large lists of military expenses outnumber investment in infrastructure and social services.") },
         { N_("Dvaered Security Reports"),
            N_("Detailed reports regarding engagements between different Warlords. You can tell whoever wrote this is a fan of military tactics.") },
      } )
   end
   if fspb==faction.get("Traders Society") then
      tmergei( cargos, {
         { N_("Trade Secrets"),
            N_("Detailed documents regarding patented trade secrets.") },
         { N_("Space Trader Membership Logs"),
            N_("Information about the members of the different guilds of the Space Traders Society.") },
      } )
   end
   mem.cargoname, mem.cargodesc = table.unpack( cargos[rnd.rnd(1,#cargos)] )

   misn.setTitle( thurion.prefix..fmt.f(_("Data Extraction from {spb} in {sys}"),
      {spb=mem.targetspb, sys=mem.targetsys}))
   misn.setDesc(fmt.f(_([[A Thurion agent has prepared some {cargoname} that have to be extracted. #rThe cargo is illegal to possess and must be delivered without raising suspicions.#0

#nAmount:#0 {amount}
#nPickup:#0 {spb} ({sys} system, )
#nDelivery:#0 {returnspb} ({returnsys} system)
#nJumps (One-way):#0 {dist}
#nReputation Gained:#0 {rep}]]),
      {spb=mem.targetspb, sys=mem.targetsys, returnspb=mem.returnspb, returnsys=mem.returnsys, dist=dist, amount=fmt.tonnes(mem.amount), rep=mem.reputation, cargoname=_(mem.cargoname)}))
   misn.setReward( mem.reward )
   misn.markerAdd( mem.targetspb, "computer" )
end

function accept ()
   misn.accept()
   misn.osdCreate( _("Data Extraction"), {
      fmt.f(_([[Pick up the documents at {spb} ({sys} system)]]), {spb=mem.targetspb, sys=mem.targetsys}),
      fmt.f(_([[Return to {spb} ({sys} system)]]), {spb=mem.returnspb, sys=mem.returnsys}),
   } )
   hook.land("land")
end

function land ()
   local sc = spob.cur()
   if sc==mem.targetspb and mem.state==0 then
      local freecargo = player.fleetCargoMissionFree()
      if freecargo < mem.amount then
         vntk.msg( _("No room in ship"), fmt.f(
            _("You don't have enough cargo space to collect the extracted {cargoname}. You need {tonnes_free} of free space ({tonnes_short} more than you have)."),
            { tonnes_free=fmt.tonnes(mem.amount), tonnes_short=fmt.tonnes( mem.amount - freecargo ), cargoname=_(mem.cargoname) } ) )
         return
      end

      vntk.msg(_("Ready to Go!"),fmt.f(_("After you land, some shady contract workers quickly load your ship with crates out of the site of the spaceport surveillance systems. With the documents the Thurion want in your hold, it is time to head back to {spb} in the {sys} system."),
         {spb=mem.returnspb, sys=mem.returnsys}))

      misn.markerRm()
      misn.markerAdd( mem.returnspb, "computer" )
      local c = commodity.new( mem.cargoname, mem.cargodesc )
      c:illegalto{ "Empire", "Dvaered", "Za'lek", "Sirius", "Soromid", "Frontier" }
      mem.carg_id = misn.cargoAdd(c, mem.amount)

      misn.osdActive(2)
      mem.state = 1
   elseif sc==mem.returnspb and mem.state==1 then

      lmisn.sfxMoney()
      vntk.msg(_("Delivery Success"),_("You touch down on the spaceport and get the documents taken care of."))

      player.pay( mem.reward )
      faction.get("Thurion"):hit( mem.reputation )
      misn.finish(true)
   end
end
