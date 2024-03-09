--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 02">
 <unique />
 <priority>0</priority>
 <chance>0</chance>
 <location>None</location>
 <notes>
  <done_evt>Onion Society 02 Trigger</done_evt>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Onion02
--]]
local fmt = require "format"
local vn = require "vn"
--local onion = require "common.onion"

local dstspb1, dstsys1 = spob.getS("Ulios")
local dstspb2, dstsys2 = spob.getS("The Frontier Council")
--local money_reward = onion.rewards.misn02

local title01 = _("Onion Delivery")

--[[
   0: mission accepted
   1: item picked up
--]]
mem.state = 0

-- Create the mission
function create()
   -- Automatically accepted
   misn.accept()

   misn.setTitle(title01)
   misn.setDesc(fmt.f(_([[You have been tasked with picking up something from {spb1} ({sys1} system) and delivering it to {spb2} ({sys2} system).]]),
      {spb1=dstspb1, sys1=dstsys1, spb2=dstspb2, sys2=dstsys2}))
   misn.setReward(_("Unknown"))

   misn.osdCreate( title01, {
      fmt.f(_([[Pick up the cargo at {spb1} ({sys1})]]), {
         spb1=dstspb1, sys1=dstsys1 }),
      fmt.f(_([[Deliver the cargo to {spb2} ({sys2})]]), {
         spb2=dstspb2, sys2=dstsys2 }),
   } )

   hook.land("land")
end

function land ()
   if mem.state==0 and spob.cur()==dstspb1 then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(fmt.f(_([[You land on the {spb1} spacedock and find a puzzled dockworker holding a small package outside your ship. It seems like something is confusing them, but they quickly give you the package and disappear before you can answer anything. Looks like they just wanted to get it over with. Time to head to {spb2} and finish the job.]]),
         {spb1=dstspb1, spb2=dstspb2}))
      vn.run()

      local c = commodity.new( N_("Another Small Box"), N_("Another suspicuous box sealed tight. You think you can hear a faint beeping sound occasionally.") )
      mem.carg_id = misn.cargoAdd( c, 0 )
      misn.osdActive(2)
      mem.state = 1
   --elseif mem.state>=1 and spob.cur()==dstspb2 then
   end
end
