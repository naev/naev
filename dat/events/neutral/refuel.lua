--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Refuel">
 <location>none</location>
 <chance>0</chance>
</event>
--]]
local pilotai = require "pilotai"

local plt, reward, amount
local shield

local SPAM_TIME   = 5 -- Time between spam in seconds

function create ()
   local nc = naev.cache()
   local r = nc.__refuel
   if not r then
      error("refuel event not properly initialized!")
   end
   plt = r.p
   amount = r.amount
   reward = r.reward

   plt:control(true)
   plt:brake()
   plt:setActiveBoard(true)
   shield = plt:shield()

   hook.pilot( plt, "board", "board" )
   hook.pilot( plt, "attacked", "attacked" )
   hook.pilot( plt, "death", "finish" )
   hook.enter("finish")
end

local last_spammed = naev.ticksGame()
function attacked( p, attacker )
   local s = p:shield()
   local isp = attacker:withPlayer()
   if not isp or s > math.min( shield-10, 50 ) then
      local t = naev.ticksGame()
      if last_spammed - t > SPAM_TIME then
         p:comm( player.pilot(), _("Come fast, I'm under attack!") )
      end
      return
   end

   -- Took too much damage
   if not isp then
      p:comm( player.pilot(), _("Under attack! I'm outta here.") )
   end

   plt:setActiveBoard(false)
   plt:control(false)
end

function board ()
   local pp = player.pilot()

   -- Nice work
   plt:comm( pp, _("Thanks a bunch!") )

   -- Reward time
   pp:setFuel( pp:fuel()-amount )
   player.pay( reward )
   plt:credits( plt:credits()-reward )
   plt:setFuel( plt:fuel()+amount )

   -- Done, time to go away
   plt:setActiveBoard(false)
   plt:control(false)
   pilotai.hyperspace( plt )
   player.unboard()

   -- We don't reset the vulnerability so it's more likely they get away and we
   -- have a happy ending
end

function finish ()
   evt.finish()
end
