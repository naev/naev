--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Mining Vrata Moss">
 <unique/>
 <priority>4</priority>
 <chance>5</chance>
 <location>Bar</location>
 <cond>
   local misn_test = require("misn_test")
   local scur = spob.cur()
   return player.evtDone("Mining Vrata Delivered Space Moss") and scur ~= spob.get("Mining Vrata Guildhouse") and scur:faction():tags().generic and misn_test.cargo(true) and misn_test.reweight_active()
 </cond>
</mission>
--]]
--[[
   After the player brings Space Moss to Steve, he hands it to some scientists for
   experimentation. They manage to mutate an ever-growing variety of it and want it
   delivered to a Soromid lab for further research.

   Player has to jettison some of the moss away every once in a while, or they explode.

   Meant to be a one-off gag mission, but of course there's nothing stopping the creation
   of an extensive "Mining Vrata Moss" campaign if someone wants to run with it.

   - Zivi (hsza)
--]]
local vn = require "vn"
local neu = require 'common.neutral'
local fmt = require "format"
local vni = require "vnimage"
local vntk = require "vntk"
local lmisn = require "lmisn"
local fleet = require "fleet"
local ferals = require "common.ferals"
local pilotai = require "pilotai"

local title    = _("Mossy Mess")
local npcname  = _("Researcher")
local pspb = spob.get("Mining Vrata Guildhouse")
local psys  = pspb:system()
local dspb = spob.get("Wigheta")
local dsys  = dspb:system()
local faction = faction.get("Traders Society")
local reward   = 600e3
local cargo_amount = 1
local npcvn, npcpor = vni.generic()

mem.c = commodity.new( _("Mutated Moss"), _("A clump of genetically engineered moss that keeps growing out of control.") )

function create ()
   misn.setNPC( npcname, npcpor, _("A Mining Vrata researcher is pacing around anxiously.") )
   misn.setTitle( title )
   misn.setReward( reward )
   misn.setDesc(fmt.f(_([[Deliver an ever-growing clump of mutated space moss to {dspb} ({dsys} system).]]), {
      dspb  = dspb,
      dsys  = dsys,
   }))
end

function accept ()
   local accepted

   vn.clear()
   vn.scene()
   local r = vn.newCharacter( "Researcher", { image = npcvn } )
   vn.transition()

   r(_([["Hello there! Are you the one that handed in the sample of space moss from Fertile Crescent? We at the Mining Vrata have been analysing it and performing some experiments, and in a stroke of genius, we've been able to create a mutation that lets it grow very rapidly at practically no expense, so we don't have to ask anyone to go all the way to bring extra samples!"]]))
   r(_([["Overlooking the fact it grew enough to nearly destroy one of our labs after being left to its own devices for a few periods, this is quite fascinating, and we're certain it could be used as a cheap source of feed for some livestock as well as Soromid bioships."]]))
   r(fmt.f(_([["Now we're ready to hand the research over to a dedicated lab over on {dspb}, and I'm on the search for a suitable courier. Before you respond, a warning: there's no way to stop the moss from growing outside of controlled environments, so you might need to dump some of it every now and again as you fly to {dsys} just in case. If you decide to help, {rwd} will be your reward. What do you say?"]]),
      {dspb=dspb, dsys=dsys, rwd=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   vn.na(_([[You decide not to get further involved in this for now.]]))
   vn.done()

   vn.label("accept")
   r(fmt.f(_([["Oh, thanks! The moss will be waiting for you at {pspb} in {psys}."]]),
      {pspb=pspb, psys=psys}))
   vn.func( function () accepted = true end )

   vn.run()

   if not accepted then
      return
   end

   misn.accept()

   misn.osdCreate( title, {
      fmt.f(_("Pick up the moss at {pspb} ({psys} system)"), {pspb=pspb, psys=psys}),
      fmt.f(_("Carefully deliver the moss to {dspb} ({dsys} system)"), {dspb=dspb, dsys=dsys}),
   })
   misn.markerAdd( pspb )
   hook.land( "land" )
end

local function pickup ()
   vn.clear()
   vn.scene()
   vn.transition()

   vn.func( function ()
      if player.fleetCargoMissionFree() < cargo_amount then
         vn.jump("nospace")
         return
      elseif player.fleetCargoMissionFree() < 200 then
         vn.jump("littlespace")
         return
      else
         vn.jump("pickup")
      end
   end )

   vn.label("nospace")
   vn.na(_([[You don't have enough cargo space for the moss.]]))
   vn.done()

   vn.label("littlespace")
   vn.na(_([[You have dangerously little cargo space available. Your ship will explode if the moss overflows. Are you sure you're ready to start the delivery?]]))
   vn.menu{
      {_("Yes"), "pickup"},
      {_("No"), "no"},
   }

   vn.label("no")
   vn.done()

   vn.label("pickup")
   vn.na(_([[A bundle of the mutated moss is quickly loaded onto your ship. You can see it expanding already.]]))
   vn.func( function () mem.gotmoss = true end )

   vn.run()

   if not mem.gotmoss then
      return
   end

   player.pilot():cargoAdd(mem.c, cargo_amount) -- We do not use misn.cargoAdd as the gimmick here is the player has to jettison it partially a few times
   misn.osdActive(2)
   misn.markerRm()
   misn.markerAdd( dspb )
   hook.takeoff( "takeoff" )
   hook.comm_jettison( "jettison" )
end

local function dropoff ()
   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(fmt.f(_([[As you approach {dspb}, you're directed to an incineration pit and told to dump any excess moss inside. Upon landing, a team of Soromid researchers takes what remains to their facility, while your cargo holds are meticulously scrubbed clean of any remains of the bothersome green gunk. Finally, you're rid of the moss, and hopefully for good.]]),
      { dspb=dspb }))
   vn.func(function()
      lmisn.sfxVictory()
      player.pay(reward)
      faction:hit(25)
      player.pilot():cargoRm(mem.c, player.pilot():cargoHas(mem.c)) -- Why does it not default to removing all?
   end)
   vn.na(fmt.reward(reward))
   vn.run()

   neu.addMiscLog(fmt.f(_("You helped deliver a sample of mutated space moss that kept growing inside your cargo hold to {dspb} for further research."), {dspb=dspb}))
   misn.finish(true)
end

function land ()
   local cspb = spob.cur()
   if cspb == dspb and mem.gotmoss then
      dropoff()
   elseif cspb == pspb and not mem.gotmoss then
      pickup()
   end
end

function takeoff ()
   if mem.gotmoss then
      hook.timerClear()
      hook.timer(8, "grow")
   end
end

local overgrowth = 0
function grow ()
   if not mem.gotmoss then return end

   local pp = player.pilot()
   local hasc = pp:cargoHas(mem.c)

   if pp:cargoFree() <= 0 then
      local a = pp:armour(true) - ((hasc + 10) ^ (1.015 + 0.02 * overgrowth) - hasc)
      if a <= 0 then
         vntk.msg(_("Uh oh"), _([[Suddenly, a loud creaking noise comes from your cargo holds. That creak was your ship's hull giving way to the growing moss. As critical systems fail and life support goes out, you feel a hint of amusement at just how dumb of an end this is.]]))
         player.damageSPFX(1)
         pp:setEnergy(0) -- Otherwise ZD-15 Guardian Unit breaks this
         pp:setHealth(-1, -1)
      else
         player.damageSPFX(0.4)
         player.autonavReset(4)
         pp:setHealthAbs( a )
         overgrowth = overgrowth + 1
         if overgrowth == 1 then player.msg(_([[Your cargo holds are overfilled with moss.]]), true)
         elseif overgrowth == 2 then player.msg(_([[The growing moss is making your hull creak dangerously.]]), true)
         elseif overgrowth == 3 then player.msg(_([[The moss is putting uncomfortable pressure on your ship's internals.]]), true)
         elseif overgrowth >= 4 then player.msg(_([[Your ship's hull is starting to give way to the moss.]]), true)
         end
      end
   else
   overgrowth = 0
   end

   pp:cargoAdd(mem.c, ((hasc + 1) ^ 1.0035) - hasc)

   hook.timer(rnd.rnd(3, 4), "grow")
end

local ambushees = {}
local function ambush ( q )
   local j = system.cur():jumps()
   local ambushes = {
         {"Nohinohi", "Nohinohi"},
         {"Nohinohi", "Nohinohi", "Nohinohi", "Nohinohi"},
         {"Taitamariki"},
         {"Taitamariki", "Taitamariki"},
         {"Taitamariki", "Nohinohi", "Nohinohi"},
         {"Kauweke"},
      }
   while rnd.rnd() < math.min(1, q / 1000) do
      local ambush_fleet = fleet.spawn( ambushes[rnd.rnd( #ambushes )], ferals.faction(), j[rnd.rnd( #j )] )
      for _,p in ipairs(ambush_fleet) do
         p:tryStealth()
         p:setHostile(true)
         p:intrinsicSet("accel", 150) -- high on moss
         p:intrinsicSet("ew_detect", 25)
         p:intrinsicSet("cooldown_mod", -40) -- tasty moss!!! more biting!!!
         table.insert(ambushees, p)
      end
      q = q - 800
   end
end

function jettison ( c, q )
   local ppp = player.pilot():pos()
   if c == mem.c and system.cur():faction() == faction.get('Soromid') then
      ambush( q )
      if ambushees then
         local ambushees_n = {}
         for _,p in ipairs(ambushees) do
            if p:exists() then
               pilotai.guard(p, ppp)
               table.insert(ambushees_n, p)
            end
         end
         ambushees = ambushees_n
      end
   end
   if player.pilot():cargoHas(mem.c) == 0 then
      vntk.msg(_("Mission failure"), _([[You have jettisoned all of the moss out of your ship. You have nothing to deliver anymore.]]))
      misn.finish(false)
   end
end

function abort ()
   local pp = player.pilot()
   local pphc = pp:cargoHas(mem.c)
   if mem.gotmoss then
      if player.isLanded() then
         pp:cargoRm(mem.c, pphc)
      else
         pp:cargoJet(mem.c, pphc)
         jettison(mem.c, pphc)
      end
   end
end
