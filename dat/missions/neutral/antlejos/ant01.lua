--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Terraforming Antlejos 1">
 <unique />
 <priority>4</priority>
 <chance>5</chance>
 <location>Bar</location>
 <cond>
   -- Must be "nearby" Antlejos
   local d = system.jumpDist("Antlejos")
   if d &lt; 4 or d &gt; 12 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <notes>
  <tier>1</tier>
  <campaign>Terraforming Antlejos</campaign>
 </notes>
</mission>
--]]
--[[
   Campaign to Terraform Antlejos V

   Just take Verner to see Antlejos V and set up camp.
--]]
local vn = require "vn"
local fmt = require "format"
local ant = require "common.antlejos"

local destpnt, destsys = spob.getS("Antlejos V")

local cargo_amount = 20 -- Amount in mass
local reward = ant.rewards.ant01


function create ()
   misn.setNPC( _("Verner"), ant.verner.portrait, _("A bored individual that seems to be looking for someone to do a task for him.") )
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   v(fmt.f(_([["You look like a pilot in need of a job. I'm looking for someone to take me and {amount} of my equipment to {pnt} in the {sys} system. Do you think you could transport me there? I'll make it worth your time with {creds}. What do you say?"]]),
      {pnt=destpnt, sys=destsys, amount=fmt.tonnes(cargo_amount), creds=fmt.credits(reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   v(_([["That's a shame. I'll be here if you change your mind."]]))
   vn.done()

   vn.label("nospace")
   v(fmt.f(_([["You only have {freespace} of free space. You need to be able to carry at least {neededspace}!"]]),
         {freespace=fmt.tonnes(player.pilot():cargoFree()), neededspace=fmt.tonnes(cargo_amount) }))
   vn.done()

   vn.label("accept")
   vn.func( function ()
      if player.pilot():cargoFree() < cargo_amount then
         vn.jump("nospace")
         return
      end
   end )
   v(_([["Great! My name is Verner. Let me load my equipment on your ship and let's be off!"]]))
   vn.func( function () accepted = true end )

   vn.run()

   -- Not accepted
   if not accepted then
      return
   end

   misn.accept()

   local c = commodity.new( N_("Equipment"), N_("Some fancy equipment. You are not sure what it is for.") )
   misn.cargoAdd(c, cargo_amount)

   misn.setTitle( _("Verner's Request") )
   misn.setDesc(fmt.f(_("Verner asked you to take him to {pnt} in the {sys} system."), {pnt=destpnt, sys=destsys}))
   misn.setReward(reward)
   misn.osdCreate(_("Verner's Request"), {
      fmt.f(_("Take Verner to {pnt} ({sys} system)"), {pnt=destpnt, sys=destsys}),
   })
   misn.markerAdd( destpnt )

   hook.land( "land" )
end

-- Land hook.
function land ()
   if spob.cur() ~= destpnt then
      return
   end

   vn.clear()
   vn.scene()
   local v = vn.newCharacter( ant.vn_verner() )
   vn.transition()
   vn.na(_([[You land on the barren moon, not really understanding what Verner is going to do here, since there really isn't anything but dust and rocks. You and Verner don space suits and unload the cargo.]]))
   v(_([[After getting off the ship he reaches down and swipes his finger across the lunar surface before bringing it up to inspect it.
"More silica content than expected. This is perfect!", he mumbles to himself.]]))
   v(_([[He turns to you and speaks.
"Thank you for bringing me here. This is much better than expected. You are probably wondering what I've come to do to such an inhospitable place, but it should be obvious. I'm going to terraform this wonderful place into a paradise!"]]))
   v(_([["I'm going to be setting up camp, but come back in a bit and I'll have more work for you. This is going to be very exciting indeed!"]]))
   vn.sfxVictory()
   vn.na( fmt.reward(reward) )
   vn.run()

   -- Apply first diff
   ant.unidiff( ant.unidiff_list[1] )

   player.pilot():setFuel(true) -- Refuel

   player.pay( reward )
   ant.log(fmt.f(_("You escorted Verner to {pnt}. He may have more work for you for his terraforming project."),{pnt=destpnt}))
   ant.dateupdate()
   misn.finish(true)
end
