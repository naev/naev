--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Refuel">
 <location>none</location>
 <chance>0</chance>
</event>
--]]
local pilotai = require "pilotai"

local p, reward, amount

function create ()
   local nc = naev.cache()
   local r = nc.__refuel
   if not r then
      error("refuel event not properly initialized!")
   end
   p = r.p
   amount = r.amount
   reward = r.reward

   p:control(true)
   p:brake()
   p:setActiveBoard(true)

   hook.pilot( p, "board", "board" )

   hook.enter("enter")
end

function board ()
   local pp = player.pilot()

   -- Nice work
   p:comm( pp, _("Thanks a bunch!") )

   -- Reward time
   pp:setFuel( pp:fuel()-amount )
   player.pay( reward )
   p:credits( p:credits()-reward )
   p:setFuel( p:fuel()+amount )

   -- Done, time to go away
   p:setActiveBoard(false)
   p:control(false)
   pilotai.hyperspace( p )
   player.unboard()

   -- We don't reset the vulnerability so it's more likely they get away and we
   -- have a happy ending
end

function enter ()
   evt.finish()
end
