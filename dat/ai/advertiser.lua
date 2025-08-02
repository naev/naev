require 'ai.core.core'
require 'ai.core.idle.advertiser'
require 'ai.core.misc.distress'
local fmt = require "format"
local ads = require "scripts.common.ads"

mem.lanes_useneutral = true
mem.atk_skill = 0

function create ()
   create_pre()

   -- Credits.
   local price = ai.pilot():ship():price()
   ai.setcredits( rnd.rnd(price/180, price/40) )

   -- No bribe
   local bribe_msg = {
      _([["Just leave me alone!"]]),
      _([["What do you want from me!?"]]),
      _([["Get away from me!"]])
   }
   mem.bribe_no = bribe_msg[ rnd.rnd(1,#bribe_msg) ]

   -- Refuel
   mem.refuel = rnd.rnd( 1000, 3000 )
   mem.refuel_msg = _([["I'll supply your ship with fuel for {credits}."]])

   -- Generate a random ad
   mem.ad = ads.generate_ad()

   -- Custom greeting
   mem.comm_greet = fmt.f(_([["{msg}"]]), {msg=mem.ad})

   mem.loiter = rnd.rnd(5,20) -- This is the amount of waypoints the pilot will pass through before leaving the system
   create_post()
end
