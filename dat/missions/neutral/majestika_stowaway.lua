--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Majestika Stowaways">
 <unique />
 <chance>0</chance>
 <location>None</location>
 <notes>
  <done_evt name="Majestika Stowaways Aboard">Triggers</done_evt>
 </notes>
</mission>
--]]
--[[
   Majestika Stowaways

   Player has to find a good home for the pair of orphans that snuck into their ship.
--]]
local neu = require "common.neutral"
local fmt = require "format"
local vn = require "vn"

local money_reward = 200e3

local title = _([[Majestika Stowaways]])
local stowaway_spob, stowaway_sys = spob.get("Majestika II")

function create ()
   misn.accept()
   misn.setDesc(fmt.f(_([[You have to find a good home for a pair of orphans that snuck unto your ship when you landed on {spob} in the {sys} system.]]),
      {spob=stowaway_spob, sys=stowaway_sys}))
   misn.setTitle( title )
   misn.setReward(_([[A good life for the pair of orphans.]]))
   misn.osdCreate( title,{
      _([[Find a suitable home for the little stowaways]]),
   } )
   hook.land("land")
end

function land()
   vn.clear()
   vn.scene()
   vn.na(_([[]]))
   vn.func( function ()
      --player.pay(money_reward)
   end )
   vn.sfxVictory()
   vn.na( fmt.reward(money_reward) )
   vn.run()

   neu.addMiscLog(_([[]]))
   misn.finish(true)
end
