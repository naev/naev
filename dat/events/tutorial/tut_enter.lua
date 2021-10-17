--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Enter Tutorial Event">
 <trigger>enter</trigger>
 <chance>100</chance>
</event>
--]]
--[[

   Enter Tutorial Event

--]]
local fmt = require "format"
local tut = require "common.tutorial"
local vn  = require "vn"

function create ()
   if tut.isDisabled() then evt.finish() end

   local enter_delay = 5

   local sys = system.cur()
   local _nebu_dens, nebu_volat = sys:nebula()
   if not var.peek( "tut_nebvol" ) and nebu_volat > 0 then
      hook.timer( enter_delay, "tut_volatility" )
   end
end


function tut_volatility ()
   local sys = system.cur()
   local _nebden, nebvol = sys:nebula()

   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.na(fmt.f(_([[As you jump the system you notice a small alarm lights up in the control panel:
#rWARNING - Volatile nebula detected in {sysname}! Taking {nebvol:.1f} MW damage!#0]]),{sysname=sys:name(), nebvol=nebvol}))
   sai(fmt.f(_([[{ainame} materializes in front of you.
"It looks like we entered part of the volatile nebula. The instability here causes heavy damage to any ships that enter. If our shield regeneration surpasses the volatility damage, we should be fine. However, if the volatility gets any stronger, it could be fatal to the {shipname}. Going deeper into the nebula could prove to be a very risky endeavour."]]), {shipname=player.ship(), ainame=tut.ainame()} ) )
   vn.done( tut.shipai.transition )
   vn.run()

   var.push( "tut_nebvol", true )
   evt.finish()
end
