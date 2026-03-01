--[[
<?xml version='1.0' encoding='utf8'?>
<event name="P-1587 Dragon Epilogue">
 <location>land</location>
 <chance>100</chance>
 <spob>P-1587</spob>
 <cond>player.evtDone("P-1587 Dragon Awake")</cond>
 <unique />
</event>
--]]
local vn = require "vn"
local fmt = require "format"

local GOODS    = commodity.get("Luxury Goods")
local REWARD   = outfit.get("Dragon Scales")

function create ()
   vn.clear()
   vn.scene()
   vn.transition()

   vn.na(fmt.f(_([[You return to where the pile of {goods} lays scattered and damaged over the planetary surface. Looks like you won't be able to recover any.]]),
      {goods=GOODS}))
   vn.na(_([[You give the area one last scan before giving up, which, lo and behold,  seems to bring up something other than damaged goods. It has an odd shape, almost like an egg! Could this be?!]]))
   vn.na(_([[You race to get a closer look at the object, however, it seems to be exfoliated scales from the feral bioship that inhabited the mountain of luxury goods. Taking a closer look at them, they seem like they could be useful as makeshift armour on your ships.]]))

   vn.sfxVictory()
   vn.func( function ()
      player.outfitAdd( REWARD )
   end )
   vn.na( fmt.reward( REWARD ) )

   vn.run()
   evt.finish(true)
end
