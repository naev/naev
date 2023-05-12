--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 4">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Land</location>
 <spob>Antlejos V</spob>
 <done>Terraforming Antlejos 3</done>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Verner needs you to bring in morphogenic archaea
   Introduces the Pilots United Against Atmosphere Anthropocentrism (PUAAA)
--]]
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local ant = require "common.antlejos"
local lmisn = require "lmisn"
local fleet = require "fleet"

local cargo_name = _("morphogenic archaea")
local cargo_amount = 100 -- Amount in mass
local reward = ant.rewards.ant04

local returnpnt, returnsys = spob.getS("Antlejos V")


function create ()
   if ant.datecheck() then misn.finish() end

   -- We claim Antlejos V
   if not misn.claim(returnsys) then misn.finish() end

   mem.destpnt, mem.destsys = lmisn.getRandomSpobAtDistance( system.cur(), 5, 30, "Soromid", true, function( p )
      return p:tags().agriculture
   end )
   if not mem.destpnt then
      misn.finish()
      return
   end

   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_("Your ship touches ground and Verner comes to greet you."))
   v(fmt.f(_([[He beams a smile at you.
"The atmosphere is starting to take shape. We're almost ready for the next step. We can now try to introduce some sort of stable form of life and bootstrap the final terraforming stages. I've found a supplier of Morphogenic Archaea that should be able to do the job. Would you be able to go to {pnt} in the {sys} system to bring some {amount} of archaea over? You'll get {creds} for your troubles. Are you interested?"]]),
      {pnt=mem.destpnt, sys=mem.destsys, amount=fmt.tonnes(cargo_amount), creds=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(_([["Awesome! We're so close!"]]))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   misn.accept()
   misn.setTitle( _("Terraforming Antlejos") )
   misn.setDesc(fmt.f(_("Pick up the {cargo} at {pnt} in the {sys} system and deliver it to {retpnt}."),
      {cargo=cargo_name, pnt=mem.destpnt, sys=mem.destsys, retpnt=returnpnt}))
   misn.setReward(reward)
   misn.osdCreate(_("Terraforming Antlejos V"), {
      fmt.f(_("Pick up the {cargo} at {pnt} ({sys} system)"), {cargo=cargo_name, pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Deliver the cargo to {pnt} ({sys} system)"), {pnt=returnpnt, sys=returnsys})
   })
   mem.mrk = misn.markerAdd( mem.destpnt )
   mem.state = 1

   hook.land( "land" )
   hook.enter( "enter" )
end

-- Land hook.
function land ()
   if mem.state==1 and  spob.cur() == mem.destpnt then

      local fs = player.pilot():cargoFree()
      if fs < cargo_amount then
         vntk.msg(_("Insufficient Space"), fmt.f(_("You have insufficient free cargo space for the {cargo}. You only have {freespace} of free space, but you need at least {neededspace}."),
            {cargo=cargo_name, freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(cargo_amount)}))
         return
      end
      vntk.msg(_("Cargo Loaded"), fmt.f(_("The dock workers load the {amount} of {cargo} onto your ship."),{cargo=cargo_name, amount=fmt.tonnes(cargo_amount)}))

      local c = commodity.new( N_("Morphogenic Archaea"), N_("Containers filled to the brim with organisms that are suitable for creating life from scratch.") )
      misn.cargoAdd( c, cargo_amount )
      misn.osdActive(2)
      mem.state = 2

      misn.markerMove( mem.mrk, returnpnt )

   elseif mem.state==2 and spob.cur() == returnpnt then
      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      vn.na(fmt.f(_([[You land your planet after all the commotion going on outside, and find Verner waiting for you. His gang unloads the {cargo} while he talks to you.]]),
         {cargo=cargo_name}))
      v(_([["I see you met them. They are part of the #oPilots United Against Atmosphere Anthropocentrism#0 or #oPUAAA#0 for short. What they are is a bunch of assholes that reject progress and terraforming barren moons like this one into places suitable for human living."]]))
      v(_([["They must have noticed when the paperwork I filed at the Empire for terraforming permission was made public. I thought we would have a lot more time before they started messing things up. Looks like we'll have to be careful from now on as they'll only get more aggressive as they see our wonderful progress."]]))
      v(_([["The archaea look like they're in tip-top shape. We'll get to spreading them around right away. Things should start looking much better now that life will start to take a hold on this moon."]]))
      v(_([["We still have a ton of things for you to do. If you are interested, please meet me up here again after I organize things a bit."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      -- Apply first diff
      ant.unidiff( ant.unidiff_list[4] )

      player.pay( reward )
      ant.log(fmt.f(_("You delivered some Soromid morphogenic archaea to {pnt} to help Verner further terraform it."),{pnt=returnpnt}))
      ant.dateupdate()
      misn.finish(true)
   end
end

local plts
function enter ()
   if mem.state~=2 or system.cur() ~= returnsys then
      return
   end

   pilot.clear()
   pilot.toggleSpawn(false)

   local puaaa = ant.puaaa()
   plts = fleet.add( 2, "Hyena", puaaa, returnpnt:pos(), _("Protestor"), {ai="guard"} )
   for _k,p in ipairs(plts) do
      p:setVisplayer()
   end

   hook.timer( 10, "protest" )
end

local protest_lines = ant.protest_lines
local protest_id, attacked
function protest ()
   if protest_id == nil then
      protest_id = rnd.rnd(1,#protest_lines)
   end

   -- See surviving pilots
   local nplts = {}
   for _k,p in ipairs(plts) do
      if p:exists() then
         table.insert( nplts, p )
      end
   end
   plts = nplts
   if #plts <= 0 then
      return
   end

   local p = plts[ rnd.rnd(1,#plts) ]
   local pp = player.pilot()
   if not attacked and p:inrange(pp) and p:pos():dist(pp:pos()) < p:memory().guarddodist then
      attacked = true
      for _k, pk in ipairs(plts) do
         pk:setHostile()
      end
      p:broadcast( fmt.f(_("Hey! That ship is helping to terraform {pnt}! Get them!"),{pnt=returnpnt}) )
      player.autonavReset(5)
   else
      -- Say some protest slogan
      p:broadcast( protest_lines[ protest_id ] )
      protest_id = math.fmod(protest_id, #protest_lines)+1
   end

   -- Protest again
   hook.timer( 15, "protest" )
end
