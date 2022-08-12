require 'ai.core.core'
require 'ai.core.idle.advertiser'
--require 'ai.core.misc.distress'
local fmt = require "format"

mem.lanes_useneutral = true
mem.aggressive = true
-- Spam less often
mem.ad = nil -- Has to be set for them to spam
mem.adspamdelayalpha = 45
mem.adspamdelaybeta = 90

function create ()
   create_pre()

   -- Credits.
   local price = ai.pilot():ship():price()
   ai.setcredits( rnd.rnd(price/180, price/40) )

   -- Refuel
   mem.refuel = rnd.rnd( 1000, 3000 )
   mem.refuel_msg = fmt.f(_([["I'll supply your ship with fuel for {credits}."]]),
         {credits=fmt.credits(mem.refuel)})

   -- Custom greeting
   mem.comm_greet = mem.ad

   mem.loiter = rnd.rnd(5,20) -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end

function taunt( target, _offense )
   if mem.taunt then
      ai.pilot():comm( target, mem.taunt )
   end
end
