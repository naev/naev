--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 2">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>4</priority>
  <chance>100</chance>
  <location>Land</location>
  <planet>Antlejos V</planet>
  <done>Terraforming Antlejos 1</done>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Verner needs you to bring in heavy machinery from the Dvaered.
--]]
local vn = require "vn"
local vntk = require "vntk"
local fmt = require "format"
local ant = require "common.antlejos"
local lmisn = require "lmisn"

local cargo_amount = 50 -- Amount in mass
local reward = ant.rewards.ant02

local returnpnt, returnsys = planet.getS("Antlejos V")

function create ()
   mem.destpnt, mem.destsys = lmisn.getPlanetAtDistance( system.cur(), 5, 30, "Dvaered", true, function( p )
      -- TODO only look for industrial Dvaered planets
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
   vn.na(_("You land and are immediately greeted by Verner."))
   v(fmt.f(_([["The camp has been set up and terraforming is underway! However, it seems like we'll need much heavier machinery to be able to penetrate the thick surface and accelerate the terraforming. I would need you to go to {pnt} in the {sys} system to pick up {amount} of heavy machinery. I'll pay you {creds} for your troubles. What do you say?"]]),
      {pnt=mem.destpnt, sys=mem.destsys, amount=fmt.tonnes(cargo_amount), creds=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   vn.func( function ()
      if player.pilot():cargoFree() < cargo_amount then
         vn.jump("nospace")
         return
      end
   end )
   v(_([["Awesome. The heavy machinery should be of great help."]]))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   misn.accept()
   misn.setTitle( _("Verner's Request") )
   misn.setDesc(fmt.f(_("Pick up heavy machinery at {pnt} in the {sys} system and deliver it to {retpnt}."), {pnt=mem.destpnt, sys=mem.destsys, retpnt=returnpnt}))
   misn.setReward( fmt.credits(reward) )
   misn.osdCreate(_("Terraforming Antlejos V"), {
      fmt.f(_("Pick up heavy machinery to {pnt} ({sys} system)"), {pnt=mem.destpnt, sys=mem.destsys}),
      fmt.f(_("Deliver the cargo to {pnt} ({sys} system)"), {pnt=returnpnt, sys=returnsys})
   })
   mem.mrk = misn.markerAdd( mem.destpnt )

   hook.land( "land" )
end

-- Land hook.
function land ()
   if planet.cur() == mem.destpnt then

      local fs = player.pilot():cargoFree()
      if fs < cargo_amount then
         vntk.msg(_("Insufficient Space"), fmt.f(_("You have insufficient free cargo space for the heavy machinery. You only have {freespace} of free space, but you need at least {neededspace}."),{freespace=fmt.tonnes(fs), neededspace=fmt.tonnes(cargo_amount)}))
         return
      end
      vntk.msg(_("Cargo Loaded"), fmt.f(_("The dock workers load the {amount} of heavy machinery onto your ship."),{amount=fmt.tonnes(cargo_amount)}))

      local c = misn.cargoNew( N_("Heavy Machinery"), N_("Heavy machinery of Dvaered manufacture. Seems like it could be fairly useful for terraforming.") )
      misn.cargoAdd( c, cargo_amount )

      misn.markerMove( mem.mrk, returnpnt )

   elseif planet.cur() == returnpnt then

      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      vn.na(_([[You land and quickly Verner, who now seems to be accompanied by a small team, quickly unload the heavy machinery and start putting it to use.]]))
      v(_([["That was faster than expected."
He slaps the hull of a heavy machine.
"With these beauties we'll be able to make progress ten times faster now. That said, it does seem like this might not be enough. Come back to me once I get the heavy machinery set up and I should have another task for you if you are interested."]]))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      -- Apply first diff
      ant.unidiff( ant.unidiff_list[2] )

      player.pay( reward )
      ant.log(fmt.f(_("You brought heavy Dvaered machinery to {returnpnt} to help Verner terraform it."),{pnt=returnpnt}))
      misn.finish(true)
   end
end

