require 'ai.core.core'
require 'ai.core.idle.miner'
require 'ai.core.misc.distress'
require "numstring"

mem.lanes_useneutral = true

function create ()
   local price = ai.pilot():ship():price()
   ai.setcredits( rnd.rnd(price/300, price/70) )

   create_post()
end

function hail ()
   if mem.setuphail then return end

   -- Refuel
   mem.refuel = rnd.rnd( 1000, 3000 )
   mem.refuel_msg = string.format(_([["I'll supply your ship with fuel for %s."]]),
         creditstring(mem.refuel))

   -- Communication stuff
   mem.bribe_no = _([["I don't want any problems."]])

   mem.setuphail = true
end
