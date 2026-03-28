--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Mining Vrata Moss">
 <unique/>
 <priority>4</priority>
 <chance>5</chance>
 <location>Bar</location>
 <cond>
   local misn_test = require("misn_test")
   return player.evtDone("Mining Vrata Delivered Space Moss") and spob.cur ~= spob.get("Mining Vrata Guildhouse") and misn_test.cargo(true) and misn_test.reweight_active()
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

local title    = _("Mossy Mess")
local npcname  = _("Mining Vrata Researcher")
local pspb = spob.get("Mining Vrata Guildhouse")
local psys  = pspb:system()
local dspb = spob.get("Wigheta")
local dsys  = dspb:system()
local reward   = 400e3
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

function land ()
   cspb = spob.cur()
   if cspb == dspb and mem.gotmoss then
     dropoff()
   elseif cspb == pspb and not mem.gotmoss then
     pickup()
   end
end

function pickup ()
   vn.clear()
   vn.scene()
   vn.transition()
   
   vn.func( function ()
      if player.fleetCargoMissionFree() < cargo_amount then
         vn.jump("nospace")
         return
      elseif player.fleetCargoMissionFree() < 300 then
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

function dropoff ()
   vn.clear()
   vn.scene()
   vn.transition()
   
   vn.na(fmt.f(_([[As you approach {dspb}, you're directed to an incineration pit and told to dump any excess moss inside. Upon landing, a team of Soromid researchers takes what remains to their facility, while your cargo holds are meticulously scrubbed clean of any remains of the bothersome green gunk. Finally, you're rid of the moss, and hopefully for good.]]),
      { dspb=dspb }))
   vn.func(function()
      lmisn.sfxMoney()
      player.pay(reward)
      player.pilot():cargoRm(mem.c, player.pilot():cargoHas(mem.c)) -- Why does it not default to removing all?
   end)
   vn.na(fmt.reward(reward))
   vn.run()
   
   neu.addMiscLog(fmt.f(_("You helped deliver a sample of mutated space moss that kept growing inside your cargo hold to {dspb} for further research."), {dspb=dspb}))
   misn.finish(true)
end

function takeoff ()
   if mem.gotmoss then
      hook.timerClear()
      hook.timer(8, "grow")
   end
end

function grow ()
   if not mem.gotmoss then return
   elseif player.pilot():cargoFree() <= 0 and rnd.rnd() > 0.85 then uhoh() end
   
   local hasc = player.pilot():cargoHas(mem.c)
   player.pilot():cargoAdd(mem.c, ((hasc + 1) ^ 1.003) - hasc)
   
   hook.timer(rnd.rnd(3, 4), "grow")
end

function jettison ()
   if player.pilot():cargoHas(mem.c) == 0 then
      vntk.msg(_("Mission failure"), _([[You have jettisoned all of the moss out of your ship. You have nothing to deliver anymore.]]))
      misn.finish(false)
   end
end

function uhoh ()
   vntk.msg(_("Uh oh"), _([[Suddenly, a loud creaking noise comes from your cargo holds. That creak was your ship's hull giving way to the growing moss. As critical systems fail and life support goes out, you feel a hint of amusement at just how dumb of an end this is.]]))
   player.pilot():setHealth( -1, -1 )
end

function abort ()
   if mem.gotmoss then player.pilot():cargoRm(mem.c, player.pilot():cargoHas(mem.c)) end
end
