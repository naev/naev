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
local neu = require "common.neutral"

local mainspb, mainsys = spob.cur()
local piratename = _("Red Goatee")
local reward = 200e3

local npcname = _("Levo Citizen")
local npcimage = vni.generic()

local articles = {
   {
      faction = "Generic",
      head = _("Brave Pilot Breaks Levo Blockade"),
      body = _("A brave pilot known as the 'Saviour of Levo' has broken a failed pirate blockade in the system of Levo. Local authorities praise the hero who stepped in when nobody else would."),
   },
   {
      faction = "Pirate",
      head = _("Marauders Out of Control?"),
      body = fmt.f(_([[Marauder '{pirate}' was defeated after attempting to dominate the {sys} system. Local pirate warlords express anger at such rashness which can lead to more patrols in nearby systems. "Dominating in this day and age? Gotta get with the times!" says Captain of the Pink Demon.]]),
         {pirate=piratename, sys=mainsys}),
   }
}

function create ()
   vn.clear()
   vn.scene()
   local npc = vn.newCharacter( npcname, { image=npcimage } )
   vn.transition()

   vn.na(_([[You land and quickly find yourself surrounded by ecstatic locals.]]))
   npc(_([[A more distinguished individual comes out to shake your hand.
"Thanks you for saving us!"]]))
   npc(fmt.f(_([["We usually don't see any weird stuff here in {spb}, so we were just minding our business when some pirate called {pirate} hailed us and told us they were going to dominate us!"]]),
      {spb=mainspb, pirate=piratename}))
   npc(_([["I have no idea how they managed to get here without being eliminated by Imperial or Soromid forces. Bunch of useless lots they are, the Great Houses."]]))
   npc(_([["We've been stuck like this for almost 2 cycles until you came along. I can't believe how lawless the Galaxy has become. Back in the day we didn't have so many ruffians with delusions of grandeur flying around."]]))

   npc(fmt.f(_([["At {spb}, we let no good dead go unrewarded. Here, take some credits for your troubles"]]),
      {spb=mainspb}))
   vn.sfxVictory()
   vn.func( function ()
      player.pay( reward )
   end )
   vn.na( fmt.reward( reward ) )

   npc(fmt.f(_([["Say, it's not much, but as the saviour of {spb}, we would love to host a small banquet in your honour."]]),
      {spb=mainspb}))
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

   -- Add some news
   articles[1].date_to_rm = time.get()+time.new(0,20,0)
   news.add( articles )

   neu.addMiscLog(fmt.f(_([[You helped clear a pirate blockade at the {sys} system. The locals were very grateful for your help.]]),
      {sys=mainsys}))

   evt.finish(true)
end
