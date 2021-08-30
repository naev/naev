require 'ai.core.core'
require 'ai.core.idle.civilian'
require 'ai.core.misc.distress'
require "numstring"

mem.careful   = false
mem.lanes_useneutral = true

local bribe_msg_list = {
   _([["Just leave me alone!"]]),
   _([["What do you want from me!?"]]),
   _([["Get away from me!"]])
}

function create ()
   -- Credits.
   local price = ai.pilot():ship():price()
   ai.setcredits( rnd.rnd(price/150, price/50) ) -- Target for crime

   mem.loiter = 3 -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end

function hail ()
   if mem.setuphail then return end

   mem.refuel = rnd.rnd( 1000, 3000 )
   mem.refuel_msg = string.format(_([["I'll supply your ship with fuel for %s."]]),
         creditstring(mem.refuel))

   -- No bribe
   mem.bribe_no = bribe_msg_list[ rnd.rnd(1,#bribe_msg_list) ]

   mem.setuphail = true
end
