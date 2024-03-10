--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 2">
 <unique />
 <priority>4</priority>
 <chance>100</chance>
 <location>Land</location>
 <spob>Antlejos V</spob>
 <done>Terraforming Antlejos 1</done>
 <notes>
  <campaign>Terraforming Antlejos</campaign>
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

local cargo_name = _("heavy machinery")
local cargo_amount = 50 -- Amount in mass
local reward = ant.rewards.ant02

local returnpnt, returnsys = spob.getS("Antlejos V")


function create ()
   if ant.datecheck() then misn.finish() end

   mem.destpnt, mem.destsys = lmisn.getRandomSpobAtDistance( system.cur(), 5, 30, "Dvaered", true, function( p )
      return p:tags().industrial
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
   v(fmt.f(_([["The camp has been set up and terraforming is underway! However, it seems like we'll need much heavier machinery to be able to penetrate the thick surface and accelerate the terraforming. I need you to go to {pnt} in the {sys} system to pick up {amount} of {cargo}. I'll pay you {creds} for your troubles. What do you say?"]]),
      {pnt=mem.destpnt, sys=mem.destsys, cargo=cargo_name, amount=fmt.tonnes(cargo_amount), creds=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["OK… I'll keep on terraforming here. Come back if you change your mind about helping."]]))
   vn.done()

   vn.label("accept")
   v(fmt.f(_([["Awesome. The {cargo} should be of great help."]]),{cargo=cargo_name}))
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
      vntk.msg(_("Cargo Loaded"), fmt.f(_("The dockworkers load the {amount} of {cargo} onto your ship."),{amount=fmt.tonnes(cargo_amount), cargo=cargo_name}))

      local c = commodity.new( N_("Heavy Machinery"), N_("Heavy machinery of Dvaered manufacture. Seems like it could be fairly useful for terraforming.") )
      misn.cargoAdd( c, cargo_amount )
      misn.osdActive(2)
      mem.state = 2

      misn.markerMove( mem.mrk, returnpnt )

   elseif mem.state==2 and spob.cur() == returnpnt then
      vn.clear()
      vn.scene()
      local v = vn.newCharacter( ant.vn_verner() )
      vn.transition()
      vn.na(fmt.f(_([[You land and Verner, who now seems to be accompanied by a small team, quickly unloads the {cargo} and starts putting it to use.]]),{cargo=cargo_name}))
      v(fmt.f(_([["That was faster than expected."
He slaps the hull of a heavy machine.
"With these beauties we'll be able to make progress ten times faster now. That said, it does seem like this might not be enough. Come back to me once I get the {cargo} set up, and I should have another task for you if you are interested."]]),{cargo=cargo_name}))
      vn.sfxVictory()
      vn.na( fmt.reward(reward) )
      vn.run()

      -- Apply first diff
      ant.unidiff( ant.unidiff_list[2] )

      player.pay( reward )
      ant.log(fmt.f(_("You brought heavy Dvaered machinery to {pnt} to help Verner terraform it."),{pnt=returnpnt}))
      ant.dateupdate()
      misn.finish(true)
   end
end
