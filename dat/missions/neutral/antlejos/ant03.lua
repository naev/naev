--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 3">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Land</location>
  <planet>Antlejos V</planet>
  <done>Terraforming Antlejos 2</done>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Verner needs you to bring in an atmospheric generator from the Za'lek.
--]]
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local ant = require "common.antlejos"
local lmisn = require "lmisn"

local cargo_name = _("atmosphere generator")
local cargo_amount = 70 -- Amount in mass
local reward = ant.rewards.ant03

local returnpnt, returnsys = planet.getS("Antlejos V")

function create ()
   if ant.datecheck() then misn.finish() end

   mem.destpnt, mem.destsys = lmisn.getRandomPlanetAtDistance( system.cur(), 5, 30, "Za'lek", true, function( _p )
      -- TODO only look for industrial Za'lek planets
      --return p.tags().industrial
      return true
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
   vn.na(_("As soon as you land, Verner comes bouncing up to you."))
   v(fmt.f(_([["Everything is going great thanks to you! However, the debris we are creating is getting very abrasive and we need to start setting up an atmosphere. I've found a Za'lek factory that seems to provide exactly what we need. Could you go to {pnt} in the {sys} system to pick up an atmosphere generator? It should be {amount}. If you do me this favour I'll pay you {creds}. What do you say?"]]),
      {pnt=mem.destpnt, sys=mem.destsys, amount=fmt.tonnes(cargo_amount), creds=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(_([["Perfect! We're starting to make true progress!."]]))
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
   misn.setReward( fmt.credits(reward) )
   misn.osdCreate(_("Terraforming Antlejos V"), {
      fmt.f(_("Pick up the {cargo} at {pnt} ({sys} system)"), {cargo=cargo_name, pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Deliver the cargo to {pnt} ({sys} system)"), {pnt=returnpnt, sys=returnsys})
   })
   mem.mrk = misn.markerAdd( mem.destpnt )
   mem.state = 1

   hook.land( "land" )
end

-- Land hook.
function land ()
   if mem.state==1 and  planet.cur() == mem.destpnt then

      local fs = player.pilot():cargoFree()
      if fs < cargo_amount then
         vntk.msg(_("Insufficient Space"), fmt.f(_("You have insufficient free cargo space for the {cargo}. You only have {freespace} of free space, but you need at least {neededspace}."),
            {cargo=cargo_name, freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(cargo_amount)}))
         return
      end
      vntk.msg(_("Cargo Loaded"), fmt.f(_("The automated cargo drones load the {amount} of {cargo} onto your ship."),{cargo=cargo_name, amount=fmt.tonnes(cargo_amount)}))

      local c = misn.cargoNew( N_("Atmosphere Generator"), N_("A large industrial device used for generating atmospheric conditions on barren planets of Za'lek origin.") )
      misn.cargoAdd( c, cargo_amount )
      misn.osdActive(2)
      mem.state = 2

      misn.markerMove( mem.mrk, returnpnt )

   elseif mem.state==2 and planet.cur() == returnpnt then
      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      vn.na(fmt.f(_([[As soon as your ship sets foot, Verner and his gang scramble to carefully unload the {cargo} to start setting it up.]]),
         {cargo=cargo_name}))
      v(_([["Wow, great work! You sure are proving to be invaluable to the project. Once we start getting an atmosphere we should almost be there. It seems like my calculations were all spot on!"
You can see glee in his eyes.
"If you're still interested, I have another important task for you."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      -- Apply first diff
      ant.unidiff( ant.unidiff_list[3] )

      player.pay( reward )
      ant.log(fmt.f(_("You brought a Za'lek atmosphere generator to {pnt} to help Verner further terraform it."),{pnt=returnpnt}))
      ant.dateupdate()
      misn.finish(true)
   end
end

