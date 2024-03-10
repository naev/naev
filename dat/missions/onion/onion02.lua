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
--local dstspb3, dstsys3 = spob.getS("DVNN Central")
--local money_reward = onion.rewards.misn02

local title01 = _("Onion Delivery")
--local title02 = _("Onion's Revenge")

--[[
   Mission States
   0: mission accepted
   1: item picked up
   2: landed on destspb2
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
   hook.enter("enter")
end

function land ()
   if mem.state==0 and spob.cur()==dstspb1 then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(fmt.f(_([[You land on the {spb1} spacedock and find a puzzled dockworker holding a small package outside your ship. It seems like something is confusing them, but they quickly give you the package and disappear before you can answer anything. Looks like they just wanted to get it over with.]]),
         {spb1=dstspb1}))
      vn.na(fmt.f(_([[The box looks similar to the one you delivered to {spb}. It seems to be sealed tight and emits some sort of faint beep at somewhat random intervals.]]),
         {spb=spob.get("Gordon's Exchange")}))
      vn.menu{
         {_([[Get on your ship]]), "01_ship"},
         {_([[Try to open the box]]), "01_box"},
      }

      vn.label("01_ship")
      vn.na(fmt.f(_([[You decide it's best to leave sleeping dogs lie, and take the box with you to the ship. Time to head to {spb2} to finish the job.]]),
         {spb2=dstspb2}))
      vn.done()

      vn.label("01_box")
      vn.na(fmt.f(_([[You try to open the box by hand, but find out it's too hard to open. Looks like whoever packed it really did not want it to be opened. You try to use several utensils to open it, but figure there's no easy way to open it without damaging it. It may be best to just get this over with. Time to head to {spb2} to finish the job.]]),
         {spb2=dstspb2}))

      vn.run()

      local c = commodity.new( N_("Another Small Box"), N_("Another suspicuous box sealed tight. You think you can hear a faint beeping sound occasionally.") )
      mem.carg_id = misn.cargoAdd( c, 0 )
      misn.osdActive(2)
      mem.state = 1
   --elseif mem.state>=1 and spob.cur()==dstspb2 then
   end
end

function enter ()
   if mem.state==1 then
      hook.timer(3, "strange_things")
   end
end
