--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Levo Pirates Epilogue">
 <unique/>
 <location>land</location>
 <chance>100</chance>
 <spob>Levo</spob>
 <cond>player.evtDone("Levo Pirates")</cond>
</event>
--]]
--[[
   Epilogue for saving Levo.
--]]
local vn = require 'vn'
local vni = require 'vnimage'
local fmt = require 'format'

local piratename = _("Red Goatee")
local reward = 200e3

local npcname = _("Levo Citizen")
local npcimage = vni.generic()

function create ()
   vn.clear()
   vn.scene()
   local npc = vn.newCharacter( npcname, { image=npcimage } )
   vn.transition()

   vn.na(_([[You land and quickly find yourself surrounded by ecstatic locals.]]))
   npc(_([[A more distinguished individual comes out to shake your hand.
"Thanks you for saving us!"]]))
   npc(fmt.f(_([["We usually don't see any weird stuff here in {spb}, so we were just minding our business when some pirate called {pirate} hailed us and told us they were going to dominate us!"]]),
      {spb=spob.cur(), pirate=piratename}))
   npc(_([["I have no idea how they managed to get here without being eliminated by Imperial or Soromid forces. Bunch of useless lots they are, the Great Houses."]]))
   npc(_([["We've been stuck like this for almost 2 cycles until you came along. I can't believe how lawless the Galaxy has become. Back in the day we didn't have so many ruffians with delusions of grandeur flying around."]]))

   npc(fmt.f(_([["At {spb}, we let no good dead go unrewarded. Here, take some credits for your troubles"]]),
      {spb=spob.cur()}))
   vn.sfxVictory()
   vn.func( function ()
      player.pay( reward )
   end )
   vn.na( fmt.reward( reward ) )

   npc(fmt.f(_([["Say, it's not much, but as the saviour of {spb}, we would love to host a small banquet in your honour."]]),
      {spb=spob.cur()}))
   vn.menu{
      {_([[Accept the invitation.]]),"accept"},
      {_([[Decline to join in.]]),"leave"},
   }

   vn.label("accept")
   vn.na(_([[You join in a short celebration which features a great assortment of local fish cuisine and lagers. It is quite enjoyable, and once it's done you get back on your way.]]))
   vn.done()

   vn.label("leave")
   vn.na(_([[You politely decline and go on your way.]]))
   vn.run()

   evt.finish(true)
end
